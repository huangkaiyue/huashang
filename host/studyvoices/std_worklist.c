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

/********************************************************
@��������:	ƥ��������������
@����:	text ƥ����ı�
@����:	1��ʾƥ��ɹ�
@	  	0��ʾƥ��ʧ��
*********************************************************/
static int check_text_cmd(char *text)
{
	if(strstr(text,"����")){
		play_sys_tices_voices(NO_MUSIC);
		//PlayQttsText("С���ѣ��һ����ᳪ�裬����ҳ��ɡ�",0);
		return 1;
	}
	else if(strstr(text,"ͼ��")){
		play_sys_tices_voices(TULING_HAHAXIONG);
		//PlayQttsText("�ҽд�ͷ�������ֿɰ��Ĵ�ͷ��",0);
		//PlayQttsText("�Ҿ��Ƿ������Σ������ٷ磬�˼��˰�������������������̥�������ֿɰ��������ǣ���Ҳ���Խ������ǣ������������Ѱɡ�",0);
		return 1;
	}
	else if(strstr(text,"����")){
		if((strstr(text,"��")&&strstr(text,"��"))||(strstr(text,"��")&&strstr(text,"С")))
			return 0;
		else if(strstr(text,"��")||strstr(text,"��"))
			SetVol(VOL_ADD,0);
		else if(strstr(text,"��")||strstr(text,"С"))
			SetVol(VOL_SUB,0);	
		return 1;
	}
	return 0;
}
/******************************************
@��������:	�ı�������
@����:	text �ı�����
******************************************/
static void handle_text(char *text)
{
	int ret=0;
	//���ؼ��ʣ�������Ӧ�Ļش�
	if(check_text_cmd(text))
		return ;
	ret = PlayQttsText(text,0);
	if(ret == 10202){
		//��������������
		play_sys_tices_voices(REQUEST_FAILED);
		sleep(2);
		startServiceWifi();
		sleep(2);
	}
}
/*******************************************
@��������:	json��������������
@����:	pMsg	����������
@		handle_jsion	����������
@		textString	�����������
@����ֵ:	0	�ɹ�	�����������Ǵ�����
***********************************************/
static int parseJson_string(char * pMsg,void handle_jsion(char *textString))
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
				play_sys_tices_voices(ERROR_40002);
			//}
			goto exit;
			break;
	}
    pSub = cJSON_GetObjectItem(pJson, "text");
    if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
    }
	handle_jsion(pSub->valuestring);
	err=0;
exit:
	cJSON_Delete(pJson);
	return err;
 }
jmp_buf env; 

void recvSignal(int sig) {  
    printf("received signal %d !!!\n",sig);  
	siglongjmp(env,1); 
} 

/*******************************************************
@��������:	�ϴ����ݵ�����������ȡ���ظ�
@����:	voicesdata �ϴ�����
@		len	�ϴ����ݴ�С
@		voices_type	��������
********************************************************/
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
	
	int r = sigsetjmp(env,1);  //����һ��������  
	if(  r	== 0){
		signal(SIGSEGV, recvSignal);  //���ֶδ���ص�����
		DEBUG_STD_MSG("set signal ok \n");
		///ͼ��������ϣ��ڲ����ܻᷢ������Ĵ��� 
		if((err=tl_req_voice(audio, len, RECODE_RATE, voices_type,\
								key, &result,&resSize,&text,&textSize)))
		{
			if(err==5||err==6){
				endtime=time(&t);
				if((endtime-starttime)<15)//��������������
				{
					if((err=tl_req_voice(audio, len, RECODE_RATE, voices_type,\
								key, &result,&resSize,&text,&textSize))){
						DEBUG_STD_MSG("up voices data failed err =%d\n",err);
						if(err==5||err==6){
							create_event_system_voices(5);
							sleep(2);
							startServiceWifi();
							sleep(2);
							free(audio);
							goto exit1;
						}
					}
				}else{
					create_event_system_voices(5);
					//DEBUG_STD_MSG("startServiceWifi ...\n");
					sleep(2);
					startServiceWifi();
					sleep(2);
					free(audio);
					goto exit1;
				}
			}
		}
	}
	else{
		 DEBUG_STD_MSG("jump this code bug!!\n");
		 create_event_system_voices(5);
	}
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
	pause_record_audio();
	return;
}
/******************************************************
@��������:	ѧϰ���¼�������
@����:	data ���� len ���ݴ�С
*******************************************************/
static void runJsonEvent(const char *data)
{
	parseJson_string(data,handle_text);
	free(data);
	//play_sys_tices_voices(TULING_DIDI);
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
		DEBUG_STD_MSG("add_event_msg event_lock =%d\n",event_lock);
		return ;
	}
	struct eventMsg *msg =(struct eventMsg *)(&msgSize);
	msg->len = len;
	msg->type = type;
	return putMsgQueue(evMsg,databuf,msgSize);
}
int getEventNum(void)
{
	return getWorkMsgNum(evMsg);
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
	switch(cur->type){
		case STUDY_WAV_EVENT:		//�Ự�¼�
			runJsonEvent(data);
			//pause_record_audio();
			break;
			
		case SYS_VOICES_EVENT:		//ϵͳ���¼�
			start_event_play_wav();
			handle_event_system_voices(cur->len);
			pause_record_audio();
			break;
			
		case SET_RATE_EVENT:		//URL�����¼�
			event_lock=1;	//�ܱ���״̬�¼�
			NetStreamExitFile();
			i2s_start_play(RECODE_RATE);
			event_lock=0;
			pause_record_audio();
			break;
			
		case URL_VOICES_EVENT:		//URL���粥���¼�
#ifdef LOG_MP3PLAY
			playurlLog("url play\n");
#endif
			start_event_play_url();
			NetStreamExitFile();
#ifdef LOG_MP3PLAY
			playurlLog("NetStreamExitFile\n");
#endif
			AddDownEvent(data,URL_VOICES_EVENT);
			sleep(3);
			break;

		case LOCAL_MP3_EVENT:		//�������ֲ����¼�
			start_event_play_url();
			NetStreamExitFile();
			AddDownEvent(data,LOCAL_MP3_EVENT);
			sleep(1);
			break;
			
		case QTTS_PLAY_EVENT:		//QTTS�¼�
			start_event_play_wav();
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
