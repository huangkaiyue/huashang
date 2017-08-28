#include "comshead.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/cJSON.h"
#include "base/pool.h"
#include "base/queWorkCond.h"
#include "host/voices/callvoices.h"
#include "host/voices/wm8960i2s.h"
#include "host/studyvoices/qtts_qisc.h"
#include "config.h"
#include "StreamFile.h"
#include "../voices/gpio_7620.h"
#include "../voices/huashangMusic.h"

#include "uart/uart.h"
#include "log.h"

//static const char *key = "b1833040534a6bfd761215154069ea58";
static const char *key = "a2f6808bf85a693e1bde2069c8b7fd79";
static WorkQueue *EventQue;
static WorkQueue *PlayList=NULL;
static unsigned int newEventNums=0;
static unsigned char downState=0;
static unsigned int cacheNetWorkPlaySize=0;
static unsigned char playlistVoicesSate=0;
static int event_lock=0;

void Lock_EventQueue(void){
	event_lock=1;
}
void Unlock_EventQueue(void){
	event_lock=0;
}
int getLock_EventQueue(void){
	return event_lock;
}
/*
@ 
@ 获取队列当中事件数
*/
int getEventNum(void){
	return getWorkMsgNum(EventQue);
}

/*******************************************************
@函数功能:	添加事件到链表
@参数:	eventMsg 数据	
@返回值:	-1 添加失败 其它添加成功
********************************************************/
int AddworkEvent(HandlerText_t *eventMsg,int msgSize){
	if(event_lock){
		printf("event_lock add error  \n");
		WriteEventlockLog("event_lock add error",event_lock);
		return -1;
	}
	WriteEventlockLog("event_lock add ok",event_lock);
	printf("AddworkEvent ok getEventNum =%d  \n",getEventNum());
	return putMsgQueue(EventQue,(const char *)eventMsg,msgSize);
}
/*
@ 将录制的语音上传到图灵服务器
@ voicesdata 声音数据 len声音长度 voices_type 音频格式(amr/pcm) 
  asr 请求格式(0:pcm 3:amr) rate:声音采样率
@ 无
*/
void ReqTulingServer(HandlerText_t *handText,const char *voices_type,const char* asr,int rate){
	int textSize = 0, err = 0;
	char *text = NULL;
	err = reqTlVoices(8,key,(const void *)handText->data, handText->dataSize, rate, voices_type,asr,&text,&textSize);//耗时操作
	SpeekEvent_process_log("request tuling","end",0);
	if(GetCurrentEventNums()!=handText->EventNums){	//如果当前还是处于播放等待图灵状态，表明没有其他外部事件打断，添加进去播放请求图灵服务器失败
		return;
	}
	if (err){	//请求服务器失败
		Create_PlaySystemEventVoices(CMD_35_39_REQUEST_FAILED);//播放请求服务器数据失败
		goto exit1;
	}
	if (text){
		free(handText->data);
		handText->data = text;
		handText->dataSize = textSize;
		handText->event=STUDY_WAV_EVENT;
		SpeekEvent_process_log("request tuling","add result for handler",0);
		AddworkEvent(handText,sizeof(HandlerText_t));
	}
	return ;
exit1:
	pause_record_audio();
	return;
}
/*
播放图灵云端返回的tts文本
playUrl:云端语义结果播放链接地址
playText: 云端语义解析内容
*/
static int playTulingQtts(const char *playText,unsigned int playEventNums,unsigned short playLocalVoicesIndex){
	int ret =-1;
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext==NULL){
		return -1;		
	}
	handtext->EventNums = playEventNums;
	handtext->playLocalVoicesIndex=playLocalVoicesIndex;
	SetMainQueueLock(MAIN_QUEUE_UNLOCK);
	SpeekEvent_process_log((const char *)"playTulingQtts:","play text start",0);
	char playVoicesName[12]={0};
	int playSpeed=0;
	Huashang_GetPlayVoicesName(playVoicesName,&playSpeed);
	char *huashangPlayText=(char *)calloc(1,strlen(playText)+8);
	if(huashangPlayText==NULL){
		return ret;
	}
	sprintf(huashangPlayText,"%s%s",playText," ,");
	enabledownNetworkVoiceState();
	SpeekEvent_process_log("xunfei text to text","start",0);
	PlayQttsText(playText,QTTS_UTF8,playVoicesName,playEventNums,playSpeed);	
	free(huashangPlayText);
	if(playLocalVoicesIndex==TULING_TEXT_MUSIC){
	//设置播放线程不要切换录音线程状态，不然会导致，添加歌曲的时候，后面的歌曲不能播放
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
	SpeekEvent_process_log("xunfei text to text","end",0);
	disabledownNetworkVoiceState();
	return ret;
}
/*******************************************
@函数功能:	json解析服务器数据
@参数:	pMsg	服务器数据
@		textString	解析后的数据
@返回值:	0	成功	其他整数都是错误码
***********************************************/
static int parseJson_string(HandlerText_t *handText){
	int err=-1;
	if(NULL == handText->data){
		return -1;
    }
	SpeekEvent_process_log("parseJson_string tuling result ",handText->data,0);
    cJSON * pJson = cJSON_Parse(handText->data);
	if(NULL == pJson){
    		goto exit1;
    }
	cJSON *pSub = cJSON_GetObjectItem(pJson, "token");//获取token的值，用于下一次请求时上传的校验值
	if(pSub!=NULL){
		updateTokenValue((const char *) pSub->valuestring);//暂时定义，用于临时存放校验值，每请求一次服务器都返回token
	}
#ifndef HUASHANG_JIAOYU
    pSub = cJSON_GetObjectItem(pJson, "code");
   	if(NULL == pSub){
		DEBUG_STD_MSG("get code failed\n");
		goto exit1;
	}
	DEBUG_STD_MSG("code : %d\n", pSub->valueint);
	switch(pSub->valueint){
		case 40001:		//参数为空
		case 40002:		//音频为空
		case 40003:		//参数缺失
		case 40004:	 	//参数不是标准的 json 文本
		case 40005:		//无法解析该音频文件	
		case 40006:		//参数错误
		case 40007: 	//无效的 Key
		case 40008:
		case 40009:		//访问受限
		case 40010:		//Token 错误 ,播放图灵错误内容
			goto exit2;
		case 40011: 	//非法请求
		case 40012:		//服务受限
		case 40013: 	//缺少 Token
	}
#endif	
	pSub = cJSON_GetObjectItem(pJson, "asr");		//返回识别的汉字结果 
    	if(NULL == pSub){
		DEBUG_STD_MSG("get info failed\n");
		goto exit1;
    }
	if(pSub->valuestring==NULL){	//当出现 "info":{}  --->这种字段，会遇到空指针
		pSub = cJSON_GetObjectItem(pJson, "tts");
		if(pSub==NULL||pSub->valuestring==NULL){
			goto exit1;
		}else{
			goto exit2;
		}
	}
	char *infoText = pSub->valuestring;
	
	DEBUG_STD_MSG("info: %s\n",pSub->valuestring);			//语音识别出来的汉字	
	SpeekEvent_process_log("get tuling text",pSub->valuestring,0);
	char getPlayMusicName[128]={0};
	pSub = cJSON_GetObjectItem(pJson, "tts");				//解析到语音结果
	if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit1;
	}	
	int cmd = CheckinfoText_forContorl(infoText,(const char *)pSub->valuestring,getPlayMusicName);
	SpeekEvent_process_log("CheckinfoText_forContorl ","check cmd",cmd);
	if(cmd<0){
	}else{//正确加载里面的文字内容，播放系统音
		if(HandlerPlay_checkTextResult(cmd,(const char *)getPlayMusicName,handText->EventNums)){
			SpeekEvent_process_log("HandlerPlay_checkTextResult","exit0",cmd);
			goto exit0;
		}else{
			SpeekEvent_process_log("HandlerPlay_checkTextResult ","exit1",cmd);
			goto exit1;
		}
	}
exit2:	
	DEBUG_STD_MSG("text: %s\n",pSub->valuestring);
	cJSON *funcpSub = cJSON_GetObjectItem(pJson, "func");
	if(funcpSub==NULL){//直接播放语义之后的结果
		SpeekEvent_process_log("playTulingQtts ","start play text",0);
		playTulingQtts((const char *)cJSON_GetObjectItem(pJson, "tts")->valuestring,handText->EventNums,TULING_TEXT);
	}else{
			pSub = cJSON_GetObjectItem(funcpSub, "url"); 	//检查是否有mp3歌曲返回，如果有				
			if(NULL == pSub||pSub->valuestring==NULL||!strcmp(pSub->valuestring,"")){//如果出现空的链接地址，直接跳出
				goto exit1;
			}
			Player_t *player=(Player_t *)calloc(1,sizeof(Player_t));
			if(player==NULL){
				perror("calloc error !!!");
				goto exit1;
			}
			snprintf(player->playfilename,128,"%s",pSub->valuestring);
			snprintf(player->musicname,64,"%s","speek");
			player->musicTime = 0;
			SpeekEvent_process_log((const char *)"play tuling music url:",pSub->valuestring,0);
			//识别出有语义结果和mp3链接地址结果，先播放前面的语义内容，再播放mp3链接地址内容
			if(playTulingQtts((const char *)cJSON_GetObjectItem(pJson, "tts")->valuestring,handText->EventNums,TULING_TEXT_MUSIC)){
				free((void *)player);
				goto exit0;
			}
			HandlerText_t *handMainMusic = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
			if(handMainMusic==NULL){
				free((void *)player);
				goto exit0;	
			}
			handMainMusic->EventNums =handText->EventNums;//添加歌曲也要标记当前事件编号
			handMainMusic->data=(char *)player;
			start_event_play_Mp3music();
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
@函数功能:	学习音事件处理函数
@参数:	data 数据 len 数据大小
*******************************************************/
static void runJsonEvent(HandlerText_t *handText){
	start_event_play_wav();
	parseJson_string(handText);
	free((void *)handText->data);
	free((void *)handText);
}

/*
@ 
@ 清除队列里面多余的事件
*/
void cleanQuequeEvent(void){
	char *msg;
	int msgSize;
	printf("\n cleanQuequeEvent \n");
	event_lock=1;	//受保护状态事件
	while(getWorkMsgNum(EventQue)){
		getMsgQueue(EventQue,&msg,&msgSize);
		if(msg!=NULL){
			free(msg);
			usleep(100);
		}
	}
	event_lock=0;
}
static void playSelectDirMenu(HandlerText_t *handText){
	int timeOut=0;
	SetMainQueueLock(MAIN_QUEUE_UNLOCK);		//去除清理锁
	NetStreamExitFile();
	if(GetRecordeVoices_PthreadState()==PLAY_MP3_MUSIC){
		usleep(200000);	
		while(GetLockRate()){	// main pthread is change rate ��delay  time  for play dirent name voices
			usleep(100000);
			if(++timeOut>5){
				break;
			}
		}
	}
	if(GetCurrentEventNums()!=handText->EventNums){
		goto exit0;
	}
	Handle_PlaySystemEventVoices(handText->playLocalVoicesIndex,handText->EventNums);
	usleep(200000);	
	if(GetCurrentEventNums()!=handText->EventNums){
		goto exit0;
	}
	SetStreamPlayState(MUSIC_PLAY_LIST);
	start_event_play_Mp3music();
	handText->event =LOCAL_MP3_EVENT;
	AddDownEvent((const char *)handText,LOCAL_MP3_EVENT);
	return ;
exit0:
	free((void *)handText->data);
	free((void *)handText);
}
/*******************************************************
@函数功能:	事件处理函数
@参数:	data 数据	msgSize事件类型以及数据大小结构体
********************************************************/
static void HandleEventMessage(const char *data,int msgSize){
	HandlerText_t *handText = (HandlerText_t *)data;
	switch(handText->event){
		case STUDY_WAV_EVENT:		//会话事件
			newEventNums=handText->EventNums;
			runJsonEvent((HandlerText_t *)data);
			break;
			
		case SYS_VOICES_EVENT:		//系统音事件
			start_event_play_wav();	//切换成wav模式，播放系统音正常退出，则自动切换到挂起状态，否则保留最新打断状态
			Handle_PlaySystemEventVoices(handText->playLocalVoicesIndex,handText->EventNums);
			free((void *)handText);
			break;
			
		case SET_RATE_EVENT:		//URL清理事件
			Lock_EventQueue();			//受保护状态事件
			WriteEventlockLog("start",getLock_EventQueue());
			SetMainQueueLock(MAIN_QUEUE_LOCK);
			NetStreamExitFile();
			SetWm8960Rate(RECODE_RATE,(const char *)"HandleEventMessage SET_RATE_EVENT");
			Unlock_EventQueue();
			pause_record_audio();
			WriteEventlockLog("SET_RATE_EVENT end",getLock_EventQueue());
			break;
			
		case URL_VOICES_EVENT:		//URL网络播放事件
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		
			NetStreamExitFile();
			start_event_play_Mp3music();
			WritePlayUrl_Log("HandleEventMessage","start add url to mainQueue for play");
			AddDownEvent((const char *)data,URL_VOICES_EVENT);
			sleep(3);
			break;
			
		case LOCAL_MP3_EVENT:		//本地音乐播放事件
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		//去除清理锁
			NetStreamExitFile();
			start_event_play_Mp3music();
			AddDownEvent((const char *)data,LOCAL_MP3_EVENT);
			DEBUG_STD_MSG("handle_event_msg LOCAL_MP3_EVENT add end\n");
			break;		
			
		case QTTS_PLAY_EVENT:		//QTTS事件
			newEventNums=handText->EventNums;
			Handler_PlayQttsEvent(handText);
			break;

		case SPEEK_VOICES_EVENT:	//接收到语音消息	
			//showFacePicture(WEIXIN_PICTURE);
			PlayWeixin_SpeekAmrFileVoices(handText->data,handText->EventNums,handText->mixMode);
			usleep(1000);
			free((void *)handText->data);
			free((void *)handText);
			break;
			
		case TALK_EVENT_EVENT:		//对讲事件
			Handle_WeixinSpeekEvent(handText->playLocalVoicesIndex,handText->EventNums);
			break;	
			
		case DIR_MENU_PLAY_EVENT:
			playSelectDirMenu(handText);
			break;
			
		default:
			DEBUG_STD_MSG("not event msg !!!\n");
			break;
	}
}
/*
@  清除队列消息
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
	putMsgQueue(PlayList,data,size); //添加到播放队列	
}
int getPlayVoicesQueueNums(void){
	return getWorkMsgNum(PlayList);
}
#if 1
//播放完之后保持当前录音状态,主要用于当遇到语音点播图灵歌曲时候，播放完前缀音，
//继续播放图灵歌曲,切换采样率，不需要播放按键音
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
	int CacheNums=0;//保存打断这次播放之后缓存队列nums 数
	PlayList = initQueue();
	char *data=NULL; 
	while(1){
		switch(playlistVoicesSate){
			case START_PLAY_VOICES_LIST:
				//printf("getPlayVoicesQueueNums=%d\n",getWorkMsgNum(PlayList));
				if(getWorkMsgNum(PlayList)==0){	//当前队列为空，挂起播放	
					if(playNetwork_pos!=0){		//播放尾音
						memset(play_buf+playNetwork_pos,0,I2S_PAGE_SIZE-playNetwork_pos);
						write_pcm(play_buf);
					}
#if defined(HUASHANG_JIAOYU)					
					Close_tlak_Light();
					led_lr_oc(openled);
					usleep(100000);
#endif
					lock_pause_record_audio();
					cacheNetWorkPlaySize=0;
					playlistVoicesSate =END_PLAY_VOICES_LIST;
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
						usleep(100000);
						showFacePicture(WAIT_CTRL_NUM3);
						//printf("----------set face 1\n");
						led_lr_oc(closeled);
#endif						
						break;
					}
					if(downState==0){
						//usleep(100000);
						//Show_tlak_Light();
						//printf("------------set face 2\n");
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
						if(newEventNums!=GetCurrentEventNums()){	//当前播放事件编号和最新事件编号
							Mute_voices(MUTE);
							CleanendVoicesNums=4;	//清除尾部数据片
							playlistVoicesSate=INTERRUPT_PLAY_VOICES_LIST;
							CacheNums =getWorkMsgNum(PlayList);
							break;
						}
					}
					free(data);
				}
				break;
			case INTERRUPT_PLAY_VOICES_LIST:	
				if(--CleanendVoicesNums<=0){
					playlistVoicesSate=CLEAN_PLAY_VOICES_LIST;
				}
#ifdef HUASHANG_JIAOYU
				for(i=0;i<2;i++){	//写入空的音频数据，冲掉之前存留的杂音
					memset(play_buf,0,I2S_PAGE_SIZE);
					write_pcm(play_buf);
				}
#endif				
				usleep(1000);

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
			case EXIT_PLAY_VOICES_LIST:
				goto exit0;
				break;
		}
	}
exit0:
	destoryQueue(PlayList);
	return NULL;
}

/*
@  初始化事件处理消息线程 ,创建一件队列 ,队列名为EventQue
*/
void InitEventMsgPthread(void){
	EventQue = InitCondWorkPthread(HandleEventMessage);
	Init_Iat_MSPLogin();
	if(pthread_create_attr(PlayVoicesPthread,NULL)){
		printf("create play voices pthread failed \n");	
	}
}
/*
@  清除事件处理消息线程
*/
void CleanEventMsgPthread(void){
	playlistVoicesSate=EXIT_PLAY_VOICES_LIST;

	char *data = (char *)calloc(1,4);
	if(data){
		putPcmDataToPlay((const void * )data,4);
	}
	CleanCondWorkPthread(EventQue,CleanEventMessage);
	Iat_MSPLogout();
}
