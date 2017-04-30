#include "comshead.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/cJSON.h"
#include "base/pool.h"
#include "base/queWorkCond.h"
#include "host/voices/callvoices.h"
#include "host/voices/wm8960i2s.h"
#include "qtts_qisc.h"
#include "config.h"
#include "StreamFile.h"
#include "../sdcard/musicList.h"
#include "../voices/gpio_7620.h"
#if defined(HUASHANG_JIAOYU)
#include "../voices/huashangMusic.h"
#endif
static const char *key = "b1833040534a6bfd761215154069ea58";
static WorkQueue *EventQue;

#define TLJSONERNUM 9.0
//����ͼ������������ݣ����ű����Ѿ�¼�ƺõ���Ƶ
static void playTulingRequestErrorVoices(void){
	char buf[32]={0};
	int i=(1+(int) (TLJSONERNUM*rand()/(RAND_MAX+1.0)));
	snprintf(buf,32,"qtts/TulingJsonEr%d_8k.amr",i);
	PlaySystemAmrVoices(buf);
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
	DEBUG_STD_MSG("up voices data ...(len=%d)\n", len);
	err = reqTlVoices(8,key,(const void *)voicesdata, len, rate, voices_type,\
			asr,&text,&textSize);
#if defined(HUASHANG_JIAOYU)
		//ͼ������ʶ�����֮�󣬼��Ѷ��ʶ����
#ifdef XUN_FEI_OK		
		if(check_tuingAifiPermison()==DISABLE_TULING_PLAY){
			return ;
		}
#endif		
#endif	
	if (err == -1){	//���������ʧ��
		Create_PlaySystemEventVoices(REQUEST_FAILED_PLAY);//�����������������ʧ��
		goto exit1;
	}
	else if (err == 1){
#ifdef TEST_ERROR_TULING	
		test_tulingApi_andDownerrorFile();
#else
		Create_PlaySystemEventVoices(TIMEOUT_PLAY_LOCALFILE);//���ű����Ѿ�¼�ƺõ��ļ�
#endif		
		goto exit1;
	}
	if (text){
		AddworkEvent(text,0,STUDY_WAV_EVENT);
	}
	return ;
exit1:
#if defined(TANGTANG_LUO)||defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)
	Led_System_vigue_close();
#elif defined(DATOU_JIANG)
	led_lr_oc(closeled);
#endif
	pause_record_audio();
	return;
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
		updateTokenValue((const char *) pSub->valuestring);
	}
    pSub = cJSON_GetObjectItem(pJson, "code");
    if(NULL == pSub){
		DEBUG_STD_MSG("get code failed\n");
		goto exit0;
	}
	DEBUG_STD_MSG("code : %d\n", pSub->valueint);
#if 1	
	switch(pSub->valueint){
		case 40001:	//����Ϊ��
		case 40002:	//��ƵΪ��
		case 40003:	//����ȱʧ
		case 40004: //�������Ǳ�׼�� json �ı�
		case 40005:	//�޷���������Ƶ�ļ�	
		case 40006:	//��������
		case 40007: //��Ч�� Key
		case 40008:
		case 40009:	//��������
		case 40010:	//Token ���� ,����ͼ���������
			goto exit1;
		case 40011: //�Ƿ�����
		case 40012:	//��������
		case 40013: //ȱ�� Token
			playTulingRequestErrorVoices();
			err=0;
			goto exit0;
	}
#endif	
	pSub = cJSON_GetObjectItem(pJson, "info");		//���ؽ��
    	if(NULL == pSub){
		DEBUG_STD_MSG("get info failed\n");
		goto exit0;
    	}
	DEBUG_STD_MSG("info: %s\n",pSub->valuestring);			//����ʶ������ĺ���	
	Write_tulingTextLog(pSub->valuestring);
	if(!CheckinfoText_forContorl((const char *)pSub->valuestring)){
		err=-1;	//��ȷ����������������ݣ�����ϵͳ��
		goto exit0;
	}
exit1:	
	pSub = cJSON_GetObjectItem(pJson, "text");		//������˵��������
	if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit0;
	}
	DEBUG_STD_MSG("text: %s\n",pSub->valuestring);
	pSub = cJSON_GetObjectItem(pJson, "ttsUrl");		//������˵�������ݵ����ӵ�ַ����Ҫ���ز���
    if(NULL == pSub||(!strcmp(pSub->valuestring,""))){	//������ֿյ����ӵ�ַ��ֱ������
		DEBUG_STD_MSG("get fileUrl failed\n");
		goto exit0;
    }
	DEBUG_STD_MSG("ttsUrl: %s \n",pSub->valuestring);
	char *ttsURL= (char *)calloc(1,strlen(pSub->valuestring)+1);
	if(ttsURL==NULL){
		perror("calloc error !!!");
		goto exit0;
	}
	sprintf(ttsURL,"%s",pSub->valuestring);
	DEBUG_STD_MSG("ttsURL:%s\n",ttsURL);
	
	pSub = cJSON_GetObjectItem(pJson, "fileUrl"); 	//����Ƿ���mp3�������أ������
	if(NULL == pSub){	//ֱ�Ӳ�������֮��Ľ��
		SetMainQueueLock(MAIN_QUEUE_UNLOCK);
		AddDownEvent((const char *)ttsURL,TULING_URL_MAIN);
		err=0;
		goto exit0;
	}else{				//ʶ�������������mp3���ӵ�ַ������Ȳ���ǰ����������ݣ��ٲ���mp3���ӵ�ַ����
		if(!strcmp(pSub->valuestring,"")){//������ֿյ����ӵ�ַ��ֱ������
			free(ttsURL);
			goto exit0;
		}
		Player_t *player=(Player_t *)calloc(1,sizeof(Player_t));
		if(player==NULL){
			perror("calloc error !!!");
			free(ttsURL);
			goto exit0;
		}
		snprintf(player->playfilename,128,"%s",pSub->valuestring);
		snprintf(player->musicname,64,"%s","speek");
		player->musicTime = 0;
		SetMainQueueLock(MAIN_QUEUE_UNLOCK);
		Write_tulinglog((const char *)"play url:");
		Write_tulinglog((const char *)pSub->valuestring);
		AddDownEvent((const char *)ttsURL,TULING_URL_MAIN);
		AddDownEvent((const char *)player,TULING_URL_VOICES);
		err=0;
	}
exit0:
	cJSON_Delete(pJson);
	return err;
}
/******************************************************
@��������:	ѧϰ���¼�������
@����:	data ���� len ���ݴ�С
*******************************************************/
static void runJsonEvent(const char *data){
	SetplayNetwork_Lock();	
	start_event_play_wav();
#if defined(TANGTANG_LUO)||defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)
	Led_System_vigue_close();
#elif defined(DATOU_JIANG)
	led_lr_oc(closeled);
#endif
	if(parseJson_string(data)){
		printf("parse json data or get resource failed \n");
		pause_record_audio();
		
	}
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
		WriteEventlockLog("event_lock add error\n",event_lock);
		return -1;
	}
	if(type!=LOCAL_MP3_EVENT)	//��Ϊ���ز���������������
		sysMes.localplayname=0;
	WriteEventlockLog("event_lock add ok\n",event_lock);
	EventMsg_t *msg =(EventMsg_t *)(&msgSize);
	msg->len = len;
	msg->type = type;
	return putMsgQueue(EventQue,eventMsg,msgSize);
}
/*
@ 
@ ��ȡ���е����¼���
*/
int getEventNum(void){
	return getWorkMsgNum(EventQue);
}
/*
@ 
@ ����������������¼�
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
			break;
			
		case SYS_VOICES_EVENT:		//ϵͳ���¼�
			start_event_play_wav();
			Handle_PlaySystemEventVoices(cur->len);
			break;
			
		case SET_RATE_EVENT:		//URL�����¼�
			event_lock=1;			//�ܱ���״̬�¼�
			WriteEventlockLog("eventlock_start\n",event_lock);
			SetMainQueueLock(MAIN_QUEUE_LOCK);
			NetStreamExitFile();
			SetWm8960Rate(RECODE_RATE);
			event_lock=0;
			sysMes.localplayname=0;
			pause_record_audio();
			WriteEventlockLog("eventlock end\n",event_lock);
			break;
			
		case URL_VOICES_EVENT:		//URL���粥���¼�
			WritePlayUrl_Log("handler url voices event \n");
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		
			NetStreamExitFile();
			start_event_play_url();
			WritePlayUrl_Log("start add url to mainQueue for play\n");
			AddDownEvent((const char *)data,URL_VOICES_EVENT);
			sleep(3);
			break;
			
#ifdef 	LOCAL_MP3
		case LOCAL_MP3_EVENT:		//�������ֲ����¼�
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		//ȥ��������
			NetStreamExitFile();
			start_event_play_url();
			AddDownEvent((const char *)data,LOCAL_MP3_EVENT);
			DEBUG_STD_MSG("handle_event_msg LOCAL_MP3_EVENT add end\n");
			break;
#endif
#ifdef TEST_PLAY_EQ_MUSIC
		case TEST_PLAY_EQ_WAV:
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		//ȥ��������
			SetplayWavEixt();
			start_event_play_url();
			AddDownEvent((const char *)data,TEST_PLAY_EQ_WAV);		
			break;
#endif			
		case QTTS_PLAY_EVENT:		//QTTS�¼�
			SetplayNetwork_Lock();	
			PlayQttsText(data,cur->len);
			free((void *)data);
			break;
			
#ifdef SPEEK_VOICES	
		case SPEEK_VOICES_EVENT:	//���յ�������Ϣ	
			start_event_play_wav();
			playspeekVoices(data);
			pause_record_audio();
			usleep(1000);
			free((void *)data);
			break;
			
		case TALK_EVENT_EVENT:		//�Խ��¼�
			Handle_WeixinSpeekEvent(cur->len);
			break;
#endif
#if defined(HUASHANG_JIAOYU)
		case XUNFEI_AIFI_EVENT:
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		//ȥ��������
			NetStreamExitFile();
			start_event_play_url();
			//��ӵ����̵߳��в��Ż��Ͻ�������
			AddDownEvent((const char *)data,LOCAL_MP3_EVENT);
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
	init_iat_MSPLogin(WriteStreamPcmData);
}
/*
@  ����¼�������Ϣ�߳�
*/
void CleanEventMsgPthread(void){
	CleanCondWorkPthread(EventQue,CleanEventMessage);
	iat_MSPLogout();
}
