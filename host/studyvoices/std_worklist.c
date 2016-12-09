#include "comshead.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/cJSON.h"
#include "base/pool.h"
#include "base/queWorkCond.h"
#include "host/voices/callvoices.h"
#include "host/voices/message_wav.h"
#include "host/voices/wm8960i2s.h"
#include "tlvoice.h"
#include "qtts_qisc.h"
#include "config.h"
#include "StreamFile.h"
#include "../sdcard/musicList.h"

static char *key = "a2f6808bf85a693e1bde2069c8b7fd79";
//static char *key = "21868a0cd8806ee2ba5eab6181f0add7";//tang : change 2016.4.26 for from chang key 

struct eventMsg{
	int  len:24,type:8;
};
static WorkQueue *evMsg;

#ifdef TEST_SAVE_MP3
static int test_mp3file=0;
static  void test_save_mp3file(char *mp3_data,int size)
{
	FILE * fp;	//tang :change 2016-2-22 add mp3 
	char buf[32]={0};
	sprintf(buf,"%s%d%s","/mnt/test/text",test_mp3file++,".amr");
	fp = fopen(buf,"w+");
	if(fp ==NULL){
		perror("test_save_mp3file: fopen failed ");
		return ;
	}
	fwrite(mp3_data,size,1,fp);
	fclose(fp);
}
#endif

/******************************************
@��������:	�ı�������
@����:	text �ı�����
******************************************/
static void handle_text(char *text)
{
	tolkLog("tolk_start\n");
	int ret=0;
	//���ؼ��ʣ�������Ӧ�Ļش�
	if(check_text_cmd(text)){
		//pause_record_audio();------����check_text_cmd���������
		return ;
	}
	tolkLog("tolk handle qtts start\n");
	ret = PlayQttsText(text,QTTS_GBK);
	if(ret == 10202){
		//��������������
		playsysvoices(REQUEST_FAILED);
		startServiceWifi();
	}
	tolkLog("tolk handle qtts end\n");
}
/*******************************************
@��������:	json��������������
@����:	pMsg	����������
@		handle_jsion	����������
@		textString	�����������
@����ֵ:	0	�ɹ�	�����������Ǵ�����
***********************************************/
static int parseJson_string(const char * pMsg,void handle_jsion(char *textString))
{
	int err=-1;
	if(NULL == pMsg){
		return -1;
    }
    cJSON * pJson = cJSON_Parse(pMsg);
	if(NULL == pJson){
       	return -1;
    }
    cJSON * pSub = cJSON_GetObjectItem(pJson, "code");
    if(NULL == pSub){
		DEBUG_STD_MSG("get code failed\n");
		goto exit;
	}
	DEBUG_STD_MSG("code : %d\n", pSub->valueint);
	switch(pSub->valueint)
	{
		case 40001:
		case 40003:
		case 40004:
		case 40005:		
		case 40006:	
		case 40007:
		case 305000:
		case 302000:
		case 200000:
		case 40002:
			//if(++sysMes.error_400002>2){
				sysMes.error_400002=0;
				playsysvoices(ERROR_40002);
			//}
			goto exit;
			break;
	}
    pSub = cJSON_GetObjectItem(pJson, "text");
    if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
    }
	printf("=get text=%s=\n",pSub->valuestring);
	handle_jsion(pSub->valuestring);
	
	pSub = cJSON_GetObjectItem(pJson, "info");
    if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
    }
	printf("=upload=%s=\n",pSub->valuestring);
	err=0;
exit:
	cJSON_Delete(pJson);
	return err;
}

/*******************************************************
@��������:	�ϴ����ݵ�����������ȡ���ظ�
@����:	voicesdata �ϴ�����
@		len	�ϴ����ݴ�С
@		voices_type	��������
********************************************************/
#if 0
jmp_buf env; 

void recvSignal(int sig) {
	tulingLog("recvSignal signal 11",0);
    printf("received signal %d !!!\n",sig);
	siglongjmp(env,1); 
} 

void send_voices_server(const char *voicesdata,int len,char *voices_type)
{
	time_t t;
	int endtime=0,starttime=0;
	int resSize = 0, textSize=0, err=0;
	char *result=NULL, *text=NULL, *audio=NULL;
	starttime=time(&t);
	start_event_play_wav();//��ͣ¼��
	audio = base64_encode(voicesdata, len);
	if(!audio){
		create_event_system_voices(5);
		goto exit1;
	}
	DEBUG_STD_MSG("up voices data ...(len=%d)\n",len);
	
	tulingLog("tuling_start",err);
	int r = sigsetjmp(env,1);  //����һ��������
	tulingLog("tuling env start...",err);
	if(r== 0){
		signal(SIGSEGV, recvSignal);  //���ֶδ���ص�����
		DEBUG_STD_MSG("set signal ok \n");
		///ͼ��������ϣ��ڲ����ܻᷢ������Ĵ��� 
		if((err=tl_req_voice(audio, len, RECODE_RATE, voices_type,\
								key, &result,&resSize,&text,&textSize)))
		{
			tulingLog("tl_req_voice error",err);
			if(err==5||err==6){
				endtime=time(&t);
				if((endtime-starttime)<15)//��������������
				{
					if((err=tl_req_voice(audio, len, RECODE_RATE, voices_type,\
								key, &result,&resSize,&text,&textSize))){
						DEBUG_STD_MSG("up voices data failed err =%d\n",err);
						if(err==5||err==6){
							create_event_system_voices(5);
							startServiceWifi();
							free(audio);
							goto exit1;
						}
					}
				}else{
					create_event_system_voices(5);
					startServiceWifi();
					free(audio);
					goto exit1;
				}
			}
		}
	}
	else{
		DEBUG_STD_MSG("jump this code bug!!\n");
		tulingLog("save up down",err);
		create_event_system_voices(5);
		free(audio);
		goto exit1;
	}
	tulingLog("tuling_end",err);
	free(audio);
	audio=NULL;
	if(result){
		free(result);
		result=NULL;
	}
	if(text){
		add_event_msg(text,0,STUDY_WAV_EVENT);
	}
	return ;
exit1:
#ifdef QITUTU_SHI
	Led_System_vigue_close();
#endif
	pause_record_audio();
	return;
}
#else
void send_voices_server(const char *voicesdata,int len,char *voices_type)
{
	int textSize=0, err=0;
	char *text=NULL;
	start_event_play_wav();//��ͣ¼��
	DEBUG_STD_MSG("up voices data ...(len=%d)\n",len);
	err=reqTlVoices(10,key,(const void *)voicesdata,len,RECODE_RATE,voices_type,&text,&textSize);
	if(err==-1){
		create_event_system_voices(5);
		startServiceWifi();
		goto exit1;
	}else if(err==1){
		QttsPlayEvent("�ҾͲ��ش��㡣",QTTS_GBK);
		goto exit1;
	}
	if(text){
		add_event_msg(text,0,STUDY_WAV_EVENT);
	}
	return ;
exit1:
#ifdef QITUTU_SHI
	Led_System_vigue_close();
#endif
	pause_record_audio();
	return;
}
#endif
/******************************************************
@��������:	ѧϰ���¼�������
@����:	data ���� len ���ݴ�С
*******************************************************/
static void runJsonEvent(const char *data)
{
	parseJson_string(data,handle_text);
	free(data);
	//playsysvoices(TULING_DIDI);
}
int event_lock=0;
/*******************************************************
@��������:	����¼�������
@����:	databuf ����	len ���ݳ��� type�¼�����
@����ֵ:	-1 ���ʧ�� ������ӳɹ�
********************************************************/
int add_event_msg(const char *databuf,int  len,int  type)
{
	int msgSize=0;
	if(event_lock){
		DEBUG_STD_MSG("add_event_msg event_lock =%d\n",event_lock); // д�� type event_lock a+
		eventlockLog("event_lock add error\n",event_lock);
		return ;
	}
	if(type!=LOCAL_MP3_EVENT)	//��Ϊ���ز���������������
		sysMes.localplayname=0;
	eventlockLog("event_lock add ok\n",event_lock);
	struct eventMsg *msg =(struct eventMsg *)(&msgSize);
	msg->len = len;
	msg->type = type;
	printf("add end ..\n");
	return putMsgQueue(evMsg,databuf,msgSize);
}
int getEventNum(void)
{
	return getWorkMsgNum(evMsg);
}
void cleanEvent(void){
char *msg;
int msgSize;
while(getWorkMsgNum(evMsg)){
	getMsgQueue(evMsg,&msg,&msgSize);
		free(msg);
		usleep(100);
	}
}
/*******************************************************
@��������:	�¼�������
@����:	data ����	msgSize�¼������Լ����ݴ�С�ṹ��
********************************************************/
static void handle_event_msg(const char *data,int msgSize)
{
	struct eventMsg *cur =(struct eventMsg *)(&msgSize); 
	DEBUG_STD_MSG("=====================================================================================\n");
	DEBUG_STD_MSG("handle_event_msg cur->type = %d\n",cur->type);
	DEBUG_STD_MSG("=====================================================================================\n");
	handleeventLog("handleevent_start\n",cur->type);
	switch(cur->type){
		case STUDY_WAV_EVENT:		//�Ự�¼�
#ifdef QITUTU_SHI
			Led_System_vigue_close();
#endif
			runJsonEvent(data);
			//pause_record_audio();
			break;
			
		case SYS_VOICES_EVENT:		//ϵͳ���¼�
			start_event_play_wav();
			handle_event_system_voices(cur->len);
			if(cur->len!=2)
				pause_record_audio();
			break;
			
		case SET_RATE_EVENT:		//URL�����¼�
			event_lock=1;	//�ܱ���״̬�¼�
			eventlockLog("eventlock_start\n",event_lock);
			cleanplayEvent(1);
			//cleanEvent();
			NetStreamExitFile();
			i2s_start_play(RECODE_RATE);
			event_lock=0;
			sysMes.localplayname=0;
			pause_record_audio();
			eventlockLog("eventlock end\n",event_lock);
			break;
			
		case URL_VOICES_EVENT:		//URL���粥���¼�
			playurlLog("url play\n");
			cleanplayEvent(0);
			NetStreamExitFile();
			start_event_play_url();
			playurlLog("NetStreamExitFile\n");
			AddDownEvent(data,URL_VOICES_EVENT);
			sleep(3);
			break;
			
#ifdef 	LOCAL_MP3
		case LOCAL_MP3_EVENT:		//�������ֲ����¼�
			cleanplayEvent(0);
			NetStreamExitFile();
			start_event_play_url();
			AddDownEvent(data,LOCAL_MP3_EVENT);
			break;
#endif
			
		case QTTS_PLAY_EVENT:		//QTTS�¼�
			start_event_play_wav();
			stait_qtts_cache();
			PlayQttsText(data,cur->len);
			//pause_record_audio();
			free((void *)data);
			break;
			
#ifdef SPEEK_VOICES	
		case SPEEK_VOICES_EVENT:	//���յ�������Ϣ	
			playspeekVoices(data);	
			pause_record_audio();
			remove(data);
			usleep(1000);
			free((void *)data);
			break;
			
		case TALK_EVENT_EVENT:		//�Խ��¼�
			handle_voices_key_event(cur->len);
			break;
#endif
		default:
			DEBUG_STD_MSG("not event msg !!!\n");
			break;
	}
	handleeventLog("handleevent end\n",cur->type);
}

static void clean_event_msg(const char *data,int msgSize)
{
	struct eventMsg *cur =(struct eventMsg *)(&msgSize); 
	if(cur->type==STUDY_WAV_EVENT){
		free(data);
	}else if(cur->type==SYS_VOICES_EVENT){

	}
}


void init_stdvoices_pthread(void)
{
	evMsg = InitCondWorkPthread(handle_event_msg);
	init_iat_MSPLogin(WriteqttsPcmData);
}
void clean_stdvoices_pthread(void)
{
	CleanCondWorkPthread(evMsg,clean_event_msg);
	iat_MSPLogout();
}
