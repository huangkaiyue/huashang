#include "comshead.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/cJSON.h"
#include "base/pool.h"
#include "base/queWorkCond.h"
#include "host/voices/callvoices.h"
#include "host/voices/message_wav.h"
#include "host/voices/wm8960i2s.h"
#include "qtts_qisc.h"
#include "config.h"
#include "StreamFile.h"
#include "../sdcard/musicList.h"
#include "../voices/gpio_7620.h"

//static char *key = "a2f6808bf85a693e1bde2069c8b7fd79";
//static char *key = "21868a0cd8806ee2ba5eab6181f0add7";//tang : change 2016.4.26 for from chang key 
static const char *key = "b1833040534a6bfd761215154069ea58";

typedef struct {
	int  len:24,type:8;
}EventMsg_t;
static WorkQueue *EventQue;

//#define TEST_SAVE_MP3
#ifdef TEST_SAVE_MP3
static int test_mp3file=0;
void test_save_mp3file(char *mp3_data,int size){
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

#define TLJSONERNUM 9.0
static void TaiBenToTulingJsonEr(void){
	char buf[32]={0};
	int i=(1+(int) (TLJSONERNUM*rand()/(RAND_MAX+1.0)));
	snprintf(buf,32,"qtts/TulingJsonEr%d_8k.amr",i);
	playsysvoices(buf);
}
/*******************************************
@��������:	json��������������
@����:	pMsg	����������
@		textString	�����������
@����ֵ:	0	�ɹ�	�����������Ǵ�����
***********************************************/
static int parseJson_string(const char * pMsg){
	int err=-1;
	if(NULL == pMsg){
		return -1;
    }
    cJSON * pJson = cJSON_Parse(pMsg);
	if(NULL == pJson){
       	return -1;
    }
	cJSON *pSub = cJSON_GetObjectItem(pJson, "token");//��ȡtoken��ֵ��������һ������ʱ�ϴ���У��ֵ
	if(pSub!=NULL){
		//��ʱ���壬������ʱ���У��ֵ��ÿ����һ�η�����������token
		writeLog((const char * )"/home/tuling_log.txt",(const char * )pSub->valuestring);
		updateTokenValue((const char *) pSub->valuestring);
	}
    pSub = cJSON_GetObjectItem(pJson, "code");
    if(NULL == pSub){
		DEBUG_STD_MSG("get code failed\n");
		goto exit;
	}
	ack_TlingCtr(pMsg);
	DEBUG_STD_MSG("code : %d\n", pSub->valueint);
	switch(pSub->valueint){
		case 40007:
			PlayTuLingTaibenQtts("tokenֵ���� ,���ڸ������ݡ�",QTTS_GBK);
			goto exit;
		case 40001:
		case 40003:
		case 40004:
		case 40005:		
		case 40006:	
		case 305000:
		case 302000:
		case 200000:
		case 40002:
#if 1
			TaiBenToTulingJsonEr();
#else
			playsysvoices(ERROR_40002);
#endif
			goto exit;
	}
	pSub = cJSON_GetObjectItem(pJson, "info");		//���ؽ��
    if(NULL == pSub){
		DEBUG_STD_MSG("get info failed\n");
		goto exit;
    }
	printf("info: %s\n",pSub->valuestring);			//����ʶ������ĺ���
	if(CheckinfoText_forContorl((const char *)pSub->valuestring)){
		goto exit;
	}
	pSub = cJSON_GetObjectItem(pJson, "text");		//������˵��������
	if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
	}
	printf("text: %s\n",pSub->valuestring);
	pSub = cJSON_GetObjectItem(pJson, "ttsUrl");		//������˵�������ݵ����ӵ�ַ����Ҫ���ز���
    if(NULL == pSub){
		DEBUG_STD_MSG("get fileUrl failed\n");
		goto exit;
    }
	printf("ttsUrl: %s \n",pSub->valuestring);
	char *ttsURL= (char *)calloc(1,strlen(pSub->valuestring)+1);
	if(ttsURL==NULL){
		perror("calloc error !!!");
		goto exit;
	}
	sprintf(ttsURL,"%s",pSub->valuestring);
	printf("ttsURL:%s\n",ttsURL);
	
	pSub = cJSON_GetObjectItem(pJson, "fileUrl"); 	//���ؽ��
	if(NULL == pSub){
		tulingLog(ttsURL,1);
		cleanplayEvent(0);
		AddDownEvent((const char *)ttsURL,TULING_URL_MAIN);
		err=0;
		goto exit0;
	}else{
		Player_t *App=(Player_t *)calloc(1,sizeof(Player_t));
		if(App==NULL){
			perror("calloc error !!!");
			free(ttsURL);
			goto exit;
		}
		snprintf(App->playfilename,128,"%s",pSub->valuestring);
		snprintf(App->musicname,64,"%s","speek");
		App->musicTime = 0;
		cleanplayEvent(0);
		AddDownEvent((const char *)ttsURL,TULING_URL_MAIN);
		AddDownEvent((const char *)App,TULING_URL_VOICES);
		err=0;
		goto exit0;
	}
exit:
	pause_record_audio(28);
exit0:
	cJSON_Delete(pJson);
	return err;
}

/*******************************************************
@��������:	�ϴ����ݵ�����������ȡ���ظ�
@����:	voicesdata �ϴ�����
@		len	�ϴ����ݴ�С
@		voices_type	��������
********************************************************/
static void TaiBenToTulingQuestEr(void){
	int i=(1+(int) (5.0*rand()/(RAND_MAX+1.0)));
	switch(i){
		case 1:
			Create_PlaySystemEventVoices(REQUEST_FAILED_PLAY);	//�����������������ʧ��
			break;
#ifdef QITUTU_SHI
		case 2:
			createPlayEvent((const void *)"xiai",PLAY_NEXT);
			break;
#else
		case 3:
			createPlayEvent((const void *)"mp3",PLAY_NEXT);
			break;
		case 4:
			createPlayEvent((const void *)"story",PLAY_NEXT);
			break;
		case 5:
			createPlayEvent((const void *)"guoxue",PLAY_NEXT);
			break;
		case 6:
			createPlayEvent((const void *)"english",PLAY_NEXT);
			break;
#endif
	}
}
/*
*��������ͼ��Ľӿ�
*/
static void test_tulingApi_andDownerrorFile(void){
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root,"code",10);
	cJSON_AddStringToObject(root,"info","testetest");
	cJSON_AddStringToObject(root,"text","testetest");
	cJSON_AddStringToObject(root,"ttsUrl","http://opentest.tuling123.com/file/8caf0b37-3359-4b85-b06b-aec080ab1d69.pcm1");
	char *text=cJSON_Print(root);
	cJSON_Delete(root);
	//free(text);
	AddworkEvent(text,0,STUDY_WAV_EVENT);
}

/*
@ ��¼�Ƶ������ϴ���ͼ�������
@ voicesdata �������� len�������� voices_type ��Ƶ��ʽ(amr/pcm) 
  asr �����ʽ(0:pcm 3:amr) rate:����������
@ ��
*/
void ReqTulingServer(const char *voicesdata,int len,const char *voices_type,const char* asr,int rate){
	int textSize = 0, err = 0;
	char *text = NULL;
	printf("up voices data ...(len=%d)\n", len);
	err = reqTlVoices(8,key,(const void *)voicesdata, len, rate, voices_type,\
			asr,&text,&textSize);
	if (err == -1)
	{	//���������ʧ��
		Create_PlaySystemEventVoices(REQUEST_FAILED_PLAY);//�����������������ʧ��
		goto exit1;
	}
	else if (err == 1)
	{
		Create_PlaySystemEventVoices(TIMEOUT_PLAY_LOCALFILE);//���ű����Ѿ�¼�ƺõ��ļ�
		//test_tulingApi_andDownerrorFile();
		goto exit1;
	}
	if (text)
	{
		AddworkEvent(text,0,STUDY_WAV_EVENT);
	}
	return ;
exit1:
#ifdef QITUTU_SHI
	Led_System_vigue_close();
#endif
#ifdef TANGTANG_LUO
	Led_System_vigue_close();
#endif
#ifdef DATOU_JIANG
	led_lr_oc(closeled);
#endif
	pause_record_audio(27);
	return;
}
/******************************************************
@��������:	ѧϰ���¼�������
@����:	data ���� len ���ݴ�С
*******************************************************/
static void runJsonEvent(const char *data){
#ifdef QITUTU_SHI
	Led_System_vigue_close();
#endif
#ifdef TANGTANG_LUO
	Led_System_vigue_close();
#endif
#ifdef DATOU_JIANG
	led_lr_oc(closeled);
#endif
	parseJson_string(data);
	free((void *)data);
}
int event_lock=0;
/*******************************************************
@��������:	����¼�������
@����:	eventMsg ����	len ���ݳ��� type�¼�����
@����ֵ:	-1 ���ʧ�� ������ӳɹ�
********************************************************/
int AddworkEvent(const char *eventMsg,int  len,int  type){
	int msgSize=0;
	if(event_lock){
		DEBUG_STD_MSG("AddworkEvent event_lock =%d\n",event_lock); // д�� type event_lock a+
		eventlockLog("event_lock add error\n",event_lock);
		return ;
	}
	if(type!=LOCAL_MP3_EVENT)	//��Ϊ���ز���������������
		sysMes.localplayname=0;
	eventlockLog("event_lock add ok\n",event_lock);
	EventMsg_t *msg =(EventMsg_t *)(&msgSize);
	msg->len = len;
	msg->type = type;
	return putMsgQueue(EventQue,eventMsg,msgSize);
}
int getEventNum(void){
	return getWorkMsgNum(EventQue);
}
/*
@ 
@ ���������������ʱ��
*/
void cleanQuequeEvent(void){
	char *msg;
	int msgSize;
	event_lock=1;	//�ܱ���״̬�¼�
	while(getWorkMsgNum(EventQue)){
		getMsgQueue(EventQue,&msg,&msgSize);
		if(msg!=NULL){
			free(msg);
			usleep(100);
		}
	}
	event_lock=0;
}
/*******************************************************
@��������:	�¼�������
@����:	data ����	msgSize�¼������Լ����ݴ�С�ṹ��
********************************************************/
static void HandleEventMessage(const char *data,int msgSize){
	EventMsg_t *cur =(EventMsg_t *)(&msgSize); 
	handleeventLog("handleevent_start\n",cur->type);
	switch(cur->type){
		case STUDY_WAV_EVENT:		//�Ự�¼�
			runJsonEvent(data);
			//pause_record_audio(29);
			break;
			
		case SYS_VOICES_EVENT:		//ϵͳ���¼�
			start_event_play_wav(7);
			Handle_PlaySystemEventVoices(cur->len);
			if(cur->len==TULING_WINT_PLAY)
				break;
			pause_record_audio(30);
			break;
			
		case SET_RATE_EVENT:		//URL�����¼�
			event_lock=1;			//�ܱ���״̬�¼�
			eventlockLog("eventlock_start\n",event_lock);
			cleanplayEvent(1);
			//cleanQuequeEvent();
			NetStreamExitFile();
			SetWm8960Rate(RECODE_RATE);
			event_lock=0;
			sysMes.localplayname=0;
			pause_record_audio(31);
			eventlockLog("eventlock end\n",event_lock);
			break;
			
		case URL_VOICES_EVENT:		//URL���粥���¼�
			playurlLog("url play\n");
			cleanplayEvent(0);		
			NetStreamExitFile();
			start_event_play_url();
			playurlLog("NetStreamExitFile\n");
			AddDownEvent((const char *)data,URL_VOICES_EVENT);
			sleep(3);
			break;
			
#ifdef 	LOCAL_MP3
		case LOCAL_MP3_EVENT:		//�������ֲ����¼�
			cleanplayEvent(0);		//ȥ��������
			NetStreamExitFile();
			start_event_play_url();
			AddDownEvent((const char *)data,LOCAL_MP3_EVENT);
			DEBUG_STD_MSG("handle_event_msg LOCAL_MP3_EVENT add end\n");
			break;
#endif
#ifdef TEST_PLAY_EQ_MUSIC
		case TEST_PLAY_EQ_WAV:
			cleanplayEvent(0);		//ȥ��������
			SetplayWavEixt();
			start_event_play_url();
			AddDownEvent((const char *)data,TEST_PLAY_EQ_WAV);		
			break;
#endif			
		case QTTS_PLAY_EVENT:		//QTTS�¼�
			PlayTuLingTaibenQtts(data,cur->len);
			free((void *)data);
			break;
			
#ifdef SPEEK_VOICES	
		case SPEEK_VOICES_EVENT:	//���յ�������Ϣ	
			playspeekVoices(data);
			pause_record_audio(31);
			remove(data);
			usleep(1000);
			free((void *)data);
			break;
			
		case TALK_EVENT_EVENT:		//�Խ��¼�
			Handle_WeixinSpeekEvent(cur->len);
			break;
#endif
		default:
			DEBUG_STD_MSG("not event msg !!!\n");
			break;
	}
	handleeventLog("handleevent end\n",cur->type);
}
/*
@  ���������Ϣ
*/
static void CleanEventMessage(const char *data,int msgSize){
#if 0
	EventMsg_t *cur =(EventMsg_t *)(&msgSize);
	if(cur->type==STUDY_WAV_EVENT){
		free(data);
		data=NULL;
	}else if(cur->type==SYS_VOICES_EVENT){

	}
	if(data)
		free(data);
#endif
}

/*
@  ��ʼ���¼�������Ϣ�߳� ,����һ������ ,������ΪEventQue
*/
void InitEventMsgPthread(void){
	EventQue = InitCondWorkPthread(HandleEventMessage);
	init_iat_MSPLogin(WriteqttsPcmData);
}
/*
@  ����¼�������Ϣ�߳�
*/
void CleanEventMsgPthread(void){
	CleanCondWorkPthread(EventQue,CleanEventMessage);
	iat_MSPLogout();
}
