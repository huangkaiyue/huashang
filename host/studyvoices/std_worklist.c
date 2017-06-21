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
#include "uart/uart.h"
#include "log.h"

static const char *key = "b1833040534a6bfd761215154069ea58";
static WorkQueue *EventQue;
static WorkQueue *PlayList=NULL;
static unsigned int newEventNums=0;
static unsigned char downState=0;
static unsigned int cacheNetWorkPlaySize=0;
static unsigned char playlistVoicesSate=0;
static int event_lock=0;

#define TLJSONERNUM 9.0
//����ͼ������������ݣ����ű����Ѿ�¼�ƺõ���Ƶ
static int playTulingRequestErrorVoices(unsigned int playEventNums){
	char buf[32]={0};
	int i=(1+(int) (TLJSONERNUM*rand()/(RAND_MAX+1.0)));
	snprintf(buf,32,"qtts/TulingJsonEr%d_8k.amr",i);
	return PlaySystemAmrVoices(buf,playEventNums);
}
/*
@ ��¼�Ƶ������ϴ���ͼ�������
@ voicesdata �������� len�������� voices_type ��Ƶ��ʽ(amr/pcm) 
  asr �����ʽ(0:pcm 3:amr) rate:����������
@ ��
*/
void ReqTulingServer(HandlerText_t *handText,const char *voices_type,const char* asr,int rate){
	int textSize = 0, err = 0;
	char *text = NULL;
	err = reqTlVoices(8,key,(const void *)handText->data, handText->dataSize, rate, voices_type,asr,&text,&textSize);//��ʱ����
	if(GetCurrentEventNums()!=handText->EventNums){	//�����ǰ���Ǵ��ڲ��ŵȴ�ͼ��״̬������û�������ⲿ�¼���ϣ���ӽ�ȥ��������ͼ�������ʧ��
		return;
	}
	if (err == -1){	//���������ʧ��
		Create_PlaySystemEventVoices(REQUEST_FAILED_PLAY);//�����������������ʧ��
		goto exit1;
	}
	else if (err == 1){
		Create_PlaySystemEventVoices(TIMEOUT_PLAY_LOCALFILE);//���ű����Ѿ�¼�ƺõ��ļ�		
		goto exit1;
	}
	if (text){
		free(handText->data);
		handText->data = text;
		handText->dataSize = textSize;
		handText->event=STUDY_WAV_EVENT;
		AddworkEvent(handText,sizeof(HandlerText_t));
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
/*
����ͼ���ƶ˷��ص�tts�ı�
playUrl:�ƶ��������������ӵ�ַ
playText: �ƶ������������
*/
static int playTulingQtts(const char *playUrl,const char *playText,unsigned int playEventNums,unsigned short playLocalVoicesIndex){
	int ret =-1;
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext==NULL){
		return -1;		
	}
	handtext->data= (char *)calloc(1,strlen(playUrl)+1);
	if( handtext->data==NULL){
		perror("calloc error !!!");
		free(handtext);
		return -1;
	}
	sprintf(handtext->data,"%s",playUrl);
	DEBUG_STD_MSG("handtext->data:%s\n", handtext->data);
	handtext->EventNums = playEventNums;
	handtext->playLocalVoicesIndex=playLocalVoicesIndex;
	SetMainQueueLock(MAIN_QUEUE_UNLOCK);
	Show_musicPicture();
#if defined(HUASHANG_JIAOYU)	
	char playVoicesName[12]={0};
	int playSpeed=0;
	GetPlayVoicesName(playVoicesName,&playSpeed);
	if(!strcmp(playVoicesName,"tuling")){	//��ǰ�����˲���ͼ���
		ret =AddDownEvent((const char *)handtext,TULING_URL_MAIN);
	}else{
		enabledownNetworkVoiceState();
		PlayQttsText(playText,QTTS_UTF8,playVoicesName,playEventNums,playSpeed);	
		if(playLocalVoicesIndex==TULING_TEXT_MUSIC){
			//���ò����̲߳�Ҫ�л�¼���߳�״̬����Ȼ�ᵼ�£���Ӹ�����ʱ�򣬺���ĸ������ܲ���
			while(1){
				if(getPlaylistVoicesSate()==END_PLAY_VOICES_LIST){
					ret=0;
					printf("%s : exit play tuing qtts voices ,not playNums \n",__func__);
					usleep(100000);
					break;
				}
				if(GetCurrentEventNums()!=playEventNums){
					printf("%s : exit play tuing qtts voices ,interrupts \n",__func__);
					ret=-1;
					break;
				}
				printf("%s : wait play tuing qtts voices nums[%d] \n",__func__,getWorkMsgNum(PlayList));
				usleep(100000);
			}
		}
		disabledownNetworkVoiceState();
	}
#else
	ret = AddDownEvent((const char *)handtext,TULING_URL_MAIN);
#endif
	return ret;
}
/*******************************************
@��������:	json��������������
@����:	pMsg	����������
@		textString	�����������
@����ֵ:	0	�ɹ�	�����������Ǵ�����
***********************************************/
static int parseJson_string(HandlerText_t *handText){
	int err=-1;
	if(NULL == handText->data){
		return -1;
    }
    cJSON * pJson = cJSON_Parse(handText->data);
	if(NULL == pJson){
    		goto exit1;
    }
	cJSON *pSub = cJSON_GetObjectItem(pJson, "token");//��ȡtoken��ֵ��������һ������ʱ�ϴ���У��ֵ
	if(pSub!=NULL){
		//��ʱ���壬������ʱ���У��ֵ��ÿ����һ�η�����������token
		updateTokenValue((const char *) pSub->valuestring);
	}
    pSub = cJSON_GetObjectItem(pJson, "code");
   	if(NULL == pSub){
		DEBUG_STD_MSG("get code failed\n");
		goto exit1;
	}
	DEBUG_STD_MSG("code : %d\n", pSub->valueint);
	switch(pSub->valueint){
		case 40001:	//����Ϊ��
		case 40002:	//��ƵΪ��
		case 40003:	//����ȱʧ
		case 40004: 	//�������Ǳ�׼�� json �ı�
		case 40005:	//�޷���������Ƶ�ļ�	
		case 40006:	//��������
		case 40007: 	//��Ч�� Key
		case 40008:
		case 40009:	//��������
		case 40010:	//Token ���� ,����ͼ���������
			goto exit2;
		case 40011: 	//�Ƿ�����
		case 40012:	//��������
		case 40013: 	//ȱ�� Token
			if(playTulingRequestErrorVoices(handText->EventNums)){	//�ⲿ�¼���ϣ������л�¼���߳�״̬
				goto exit0;
			}else{
				goto exit1;
			}
	}
	pSub = cJSON_GetObjectItem(pJson, "info");		//���ؽ��
    if(NULL == pSub){
		DEBUG_STD_MSG("get info failed\n");
		goto exit1;
    }
	char *infoText = pSub->valuestring;
	DEBUG_STD_MSG("info: %s\n",pSub->valuestring);			//����ʶ������ĺ���	
	Write_tulingTextLog(pSub->valuestring);
	char getPlayMusicName[128]={0};
	pSub = cJSON_GetObjectItem(pJson, "text");				//�������������
	if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit1;
	}	
	int cmd = CheckinfoText_forContorl(infoText,(const char *)pSub->valuestring,getPlayMusicName);
	if(cmd<0){
	}else{//��ȷ����������������ݣ�����ϵͳ��
		if(HandlerPlay_checkTextResult(cmd,(const char *)getPlayMusicName,handText->EventNums)){
			goto exit0;
		}else{
			goto exit1;
		}
	}
exit2:	
	DEBUG_STD_MSG("text: %s\n",pSub->valuestring);
	pSub = cJSON_GetObjectItem(pJson, "ttsUrl");		//������˵�������ݵ����ӵ�ַ����Ҫ���ز���
    if(NULL == pSub||(!strcmp(pSub->valuestring,""))){	//������ֿյ����ӵ�ַ��ֱ������
		DEBUG_STD_MSG("get fileUrl failed\n");
		goto exit1;
    }
	DEBUG_STD_MSG("ttsUrl: %s \n",pSub->valuestring);
	char *ttsURL= pSub->valuestring;
	
	pSub = cJSON_GetObjectItem(pJson, "fileUrl"); 	//����Ƿ���mp3�������أ������
	if(NULL == pSub){	//ֱ�Ӳ�������֮��Ľ��
		playTulingQtts((const char *)ttsURL,(const char *)cJSON_GetObjectItem(pJson, "text")->valuestring,handText->EventNums,TULING_TEXT);
	}else{				//ʶ�������������mp3���ӵ�ַ������Ȳ���ǰ����������ݣ��ٲ���mp3���ӵ�ַ����
		if(!strcmp(pSub->valuestring,"")){//������ֿյ����ӵ�ַ��ֱ������
			goto exit1;
		}
		Player_t *player=(Player_t *)calloc(1,sizeof(Player_t));
		if(player==NULL){
			perror("calloc error !!!");
			goto exit1;
		}
		snprintf(player->playfilename,128,"%s",pSub->valuestring);
		//snprintf(player->playfilename,128,"%s","");
		snprintf(player->musicname,64,"%s","speek");
		player->musicTime = 0;
		Write_tulinglog((const char *)"play url:");
		Write_tulinglog((const char *)pSub->valuestring);
		if(playTulingQtts((const char *)ttsURL,(const char *)cJSON_GetObjectItem(pJson, "text")->valuestring,handText->EventNums,TULING_TEXT_MUSIC)){
			free((void *)player);
			goto exit0;
		}
		HandlerText_t *handMainMusic = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
		if(handMainMusic==NULL){
			free((void *)player);
			goto exit0;	
		}
		//��Ӹ���ҲҪ��ǵ�ǰ�¼����
		handMainMusic->EventNums =handText->EventNums;
		handMainMusic->data=(char *)player;
		AddDownEvent((const char *)handMainMusic,TULING_URL_VOICES);
	}
	cJSON_Delete(pJson);
	return 0;
exit1:
	pause_record_audio();
exit0:
	cJSON_Delete(pJson);
	return err;
}
/******************************************************
@��������:	ѧϰ���¼�������
@����:	data ���� len ���ݴ�С
*******************************************************/
static void runJsonEvent(HandlerText_t *handText){
	start_event_play_wav();
#if defined(TANGTANG_LUO)||defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)
	Led_System_vigue_close();
#elif defined(DATOU_JIANG)
	led_lr_oc(closeled);
#endif
	parseJson_string(handText);
	free((void *)handText->data);
	free((void *)handText);
}

/*******************************************************
@��������:	����¼�������
@����:	eventMsg ����	
@����ֵ:	-1 ���ʧ�� ������ӳɹ�
********************************************************/
int AddworkEvent(HandlerText_t *eventMsg,int msgSize){
	if(event_lock){
		WriteEventlockLog("event_lock add error\n",event_lock);
		return -1;
	}
	if(eventMsg->event!=LOCAL_MP3_EVENT)	//��Ϊ���ز���������������
		sysMes.localplayname=0;
	WriteEventlockLog("event_lock add ok\n",event_lock);
	return putMsgQueue(EventQue,(const char *)eventMsg,msgSize);
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
	HandlerText_t *handText = (HandlerText_t *)data;
	switch(handText->event){
		case STUDY_WAV_EVENT:		//�Ự�¼�
			newEventNums=handText->EventNums;
			runJsonEvent((HandlerText_t *)data);
			break;
			
		case SYS_VOICES_EVENT:		//ϵͳ���¼�
			start_event_play_wav();	//�л���wavģʽ������ϵͳ�������˳������Զ��л�������״̬�����������´��״̬
			Handle_PlaySystemEventVoices(handText->playLocalVoicesIndex,handText->EventNums);
			break;
			
		case SET_RATE_EVENT:		//URL�����¼�
			event_lock=1;			//�ܱ���״̬�¼�
			WriteEventlockLog("eventlock_start\n",event_lock);
			SetMainQueueLock(MAIN_QUEUE_LOCK);
			NetStreamExitFile();
			SetWm8960Rate(RECODE_RATE,(const char *)"HandleEventMessage SET_RATE_EVENT");
			event_lock=0;
			pause_record_audio();
			WriteEventlockLog("eventlock end\n",event_lock);
			break;
			
		case URL_VOICES_EVENT:		//URL���粥���¼�
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		
			NetStreamExitFile();
			start_event_play_Mp3music();
			WritePlayUrl_Log("HandleEventMessage","start add url to mainQueue for play");
			AddDownEvent((const char *)data,URL_VOICES_EVENT);
			sleep(3);
			break;
			
#ifdef 	LOCAL_MP3
		case LOCAL_MP3_EVENT:		//�������ֲ����¼�
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		//ȥ��������
			NetStreamExitFile();
			start_event_play_Mp3music();
			AddDownEvent((const char *)data,LOCAL_MP3_EVENT);
			DEBUG_STD_MSG("handle_event_msg LOCAL_MP3_EVENT add end\n");
			break;
#endif		
		case QTTS_PLAY_EVENT:		//QTTS�¼�
			newEventNums=handText->EventNums;
			Handler_PlayQttsEvent(handText);
			break;

		case SPEEK_VOICES_EVENT:	//���յ�������Ϣ	
			//showFacePicture(WEIXIN_PICTURE);
			PlayWeixin_SpeekAmrFileVoices(handText->data,handText->EventNums,handText->mixMode);
			usleep(1000);
			free((void *)handText->data);
			free((void *)handText);
			break;
			
		case TALK_EVENT_EVENT:		//�Խ��¼�
			Handle_WeixinSpeekEvent(handText->playLocalVoicesIndex,handText->EventNums);
			break;		
		default:
			DEBUG_STD_MSG("not event msg !!!\n");
			break;
	}
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

void putPcmDataToPlay(const void * data,int size){
	cacheNetWorkPlaySize+=size;
	putMsgQueue(PlayList,data,size); //��ӵ����Ŷ���	
}
int getPlayVoicesQueueNums(void){
	return getWorkMsgNum(PlayList);
}
#if 1
//������֮�󱣳ֵ�ǰ¼��״̬,��Ҫ���ڵ����������㲥ͼ�����ʱ�򣬲�����ǰ׺����
//��������ͼ�����,�л������ʣ�����Ҫ���Ű�����
void enabledownNetworkVoiceState(void){
	downState=1;
}
void disabledownNetworkVoiceState(void){
	downState=0;
}
#endif
unsigned char getPlaylistVoicesSate(void){
	return playlistVoicesSate;
}
static void *PlayVoicesPthread(void *arg){
	unsigned short playNetwork_pos=0;
	playlistVoicesSate=START_PLAY_VOICES_LIST;
	int i=0,pcmSize=0,CleanendVoicesNums=0;
	int CacheNums=0;//��������β���֮�󻺴����nums ��
	PlayList = initQueue();
	char *data=NULL; 
	while(1){
		switch(playlistVoicesSate){
			case START_PLAY_VOICES_LIST:
				//printf("getPlayVoicesQueueNums=%d\n",getWorkMsgNum(PlayList));
				if(getWorkMsgNum(PlayList)==0){	//��ǰ����Ϊ�գ����𲥷�	
					if(playNetwork_pos!=0){		//����β��
						memset(play_buf+playNetwork_pos,0,I2S_PAGE_SIZE-playNetwork_pos);
						write_pcm(play_buf);
					}
					lock_pause_record_audio();
#if defined(HUASHANG_JIAOYU)					
					led_lr_oc(openled);
#endif
					cacheNetWorkPlaySize=0;
					playlistVoicesSate =END_PLAY_VOICES_LIST;
					Close_tlak_Light();
				}
				getMsgQueue(PlayList,&data,&pcmSize);
				playlistVoicesSate=START_PLAY_VOICES_LIST;
				//printf("WaitingDown=%d\n",WaitingDown);
				while(1){
					if(newEventNums!=GetCurrentEventNums()){
						break;
					}
					if(cacheNetWorkPlaySize>12*KB){
#if defined(HUASHANG_JIAOYU)						
						Show_tlak_Light();
						led_lr_oc(closeled);
#endif						
						usleep(10000);
						break;
					}
					if(downState==0){
						Show_tlak_Light();
						break;
					}
					usleep(10000);
				}
				if(data){
					for(i=0;i<pcmSize;i+=2){
						memcpy(play_buf+playNetwork_pos,data+i,2);
						playNetwork_pos += 2;
						memcpy(play_buf+playNetwork_pos,data+i,2);
						playNetwork_pos += 2;		
						if(playNetwork_pos==I2S_PAGE_SIZE){
							write_pcm(play_buf);
							playNetwork_pos=0;
						}
						if(newEventNums!=GetCurrentEventNums()){	//��ǰ�����¼���ź������¼����
							Mute_voices(MUTE);
							CleanendVoicesNums=4;	//���β������Ƭ
							playlistVoicesSate=INTERRUPT_PLAY_VOICES_LIST;
							CacheNums =getWorkMsgNum(PlayList);
							break;
						}
					}
					free(data);
				}
				break;
			case INTERRUPT_PLAY_VOICES_LIST:
#if 0		
				CleanI2S_PlayCachedata();//����
				StopplayI2s();			 //���һƬ���ݶ���
#else		
				if(--CleanendVoicesNums<=0){
					playlistVoicesSate=CLEAN_PLAY_VOICES_LIST;
				}
				memset(play_buf,0,I2S_PAGE_SIZE);
				write_pcm(play_buf);
				usleep(1000);
#endif	

				break;
			case CLEAN_PLAY_VOICES_LIST:
				if(getWorkMsgNum(PlayList)>0&&--CacheNums>0){
					getMsgQueue(PlayList,&data,&pcmSize);
					free(data);
				}else{
					playlistVoicesSate=START_PLAY_VOICES_LIST;
					cacheNetWorkPlaySize=0;
				}
				break;
		}
	}
	destoryQueue(PlayList);
	return NULL;
}

/*
@  ��ʼ���¼�������Ϣ�߳� ,����һ������ ,������ΪEventQue
*/
void InitEventMsgPthread(void){
	EventQue = InitCondWorkPthread(HandleEventMessage);
	Init_Iat_MSPLogin();
	if(pthread_create_attr(PlayVoicesPthread,NULL)){
		printf("create play voices pthread failed \n");	
	}
}
/*
@  ����¼�������Ϣ�߳�
*/
void CleanEventMsgPthread(void){
	CleanCondWorkPthread(EventQue,CleanEventMessage);
	Iat_MSPLogout();
}
