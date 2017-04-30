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
//解析图灵请求错误内容，播放本地已经录制好的音频
static void playTulingRequestErrorVoices(void){
	char buf[32]={0};
	int i=(1+(int) (TLJSONERNUM*rand()/(RAND_MAX+1.0)));
	snprintf(buf,32,"qtts/TulingJsonEr%d_8k.amr",i);
	PlaySystemAmrVoices(buf);
}
/*
@ 将录制的语音上传到图灵服务器
@ voicesdata 声音数据 len声音长度 voices_type 音频格式(amr/pcm) 
  asr 请求格式(0:pcm 3:amr) rate:声音采样率
@ 无
*/
void ReqTulingServer(const char *voicesdata,int len,const char *voices_type,const char* asr,int rate){
	int textSize = 0, err = 0;
	char *text = NULL;
	DEBUG_STD_MSG("up voices data ...(len=%d)\n", len);
	err = reqTlVoices(8,key,(const void *)voicesdata, len, rate, voices_type,\
			asr,&text,&textSize);
#if defined(HUASHANG_JIAOYU)
		//图灵语音识别结束之后，检查讯飞识别结果
#ifdef XUN_FEI_OK		
		if(check_tuingAifiPermison()==DISABLE_TULING_PLAY){
			return ;
		}
#endif		
#endif	
	if (err == -1){	//请求服务器失败
		Create_PlaySystemEventVoices(REQUEST_FAILED_PLAY);//播放请求服务器数据失败
		goto exit1;
	}
	else if (err == 1){
#ifdef TEST_ERROR_TULING	
		test_tulingApi_andDownerrorFile();
#else
		Create_PlaySystemEventVoices(TIMEOUT_PLAY_LOCALFILE);//播放本地已经录制好的文件
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
@函数功能:	json解析服务器数据
@参数:	pMsg	服务器数据
@		textString	解析后的数据
@返回值:	0	成功	其他整数都是错误码
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
	cJSON *pSub = cJSON_GetObjectItem(pJson, "token");//获取token的值，用于下一次请求时上传的校验值
	if(pSub!=NULL){
		//暂时定义，用于临时存放校验值，每请求一次服务器都返回token
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
		case 40001:	//参数为空
		case 40002:	//音频为空
		case 40003:	//参数缺失
		case 40004: //参数不是标准的 json 文本
		case 40005:	//无法解析该音频文件	
		case 40006:	//参数错误
		case 40007: //无效的 Key
		case 40008:
		case 40009:	//访问受限
		case 40010:	//Token 错误 ,播放图灵错误内容
			goto exit1;
		case 40011: //非法请求
		case 40012:	//服务受限
		case 40013: //缺少 Token
			playTulingRequestErrorVoices();
			err=0;
			goto exit0;
	}
#endif	
	pSub = cJSON_GetObjectItem(pJson, "info");		//返回结果
    	if(NULL == pSub){
		DEBUG_STD_MSG("get info failed\n");
		goto exit0;
    	}
	DEBUG_STD_MSG("info: %s\n",pSub->valuestring);			//语音识别出来的汉字	
	Write_tulingTextLog(pSub->valuestring);
	if(!CheckinfoText_forContorl((const char *)pSub->valuestring)){
		err=-1;	//正确加载里面的文字内容，播放系统音
		goto exit0;
	}
exit1:	
	pSub = cJSON_GetObjectItem(pJson, "text");		//解析到说话的内容
	if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit0;
	}
	DEBUG_STD_MSG("text: %s\n",pSub->valuestring);
	pSub = cJSON_GetObjectItem(pJson, "ttsUrl");		//解析到说话的内容的链接地址，需要下载播放
    if(NULL == pSub||(!strcmp(pSub->valuestring,""))){	//如果出现空的链接地址，直接跳出
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
	
	pSub = cJSON_GetObjectItem(pJson, "fileUrl"); 	//检查是否有mp3歌曲返回，如果有
	if(NULL == pSub){	//直接播放语义之后的结果
		SetMainQueueLock(MAIN_QUEUE_UNLOCK);
		AddDownEvent((const char *)ttsURL,TULING_URL_MAIN);
		err=0;
		goto exit0;
	}else{				//识别出有语义结果和mp3链接地址结果，先播放前面的语义内容，再播放mp3链接地址内容
		if(!strcmp(pSub->valuestring,"")){//如果出现空的链接地址，直接跳出
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
@函数功能:	学习音事件处理函数
@参数:	data 数据 len 数据大小
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
@函数功能:	添加事件到链表
@参数:	eventMsg 数据	len 数据长度 type事件类型
@返回值:	-1 添加失败 其它添加成功
********************************************************/
int AddworkEvent(const char *eventMsg,int  len,int  type){
	int msgSize=0;
	if(event_lock){
		WriteEventlockLog("event_lock add error\n",event_lock);
		return -1;
	}
	if(type!=LOCAL_MP3_EVENT)	//不为本地播放清理播放上下曲
		sysMes.localplayname=0;
	WriteEventlockLog("event_lock add ok\n",event_lock);
	EventMsg_t *msg =(EventMsg_t *)(&msgSize);
	msg->len = len;
	msg->type = type;
	return putMsgQueue(EventQue,eventMsg,msgSize);
}
/*
@ 
@ 获取队列当中事件数
*/
int getEventNum(void){
	return getWorkMsgNum(EventQue);
}
/*
@ 
@ 清除队列里面多余的事件
*/
void cleanQuequeEvent(void){
	char *msg;
	int msgSize;
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
/*******************************************************
@函数功能:	事件处理函数
@参数:	data 数据	msgSize事件类型以及数据大小结构体
********************************************************/
static void HandleEventMessage(const char *data,int msgSize){
	EventMsg_t *cur =(EventMsg_t *)(&msgSize); 
	handleeventLog("handleevent_start\n",cur->type);
	switch(cur->type){
		case STUDY_WAV_EVENT:		//会话事件
			runJsonEvent(data);
			break;
			
		case SYS_VOICES_EVENT:		//系统音事件
			start_event_play_wav();
			Handle_PlaySystemEventVoices(cur->len);
			break;
			
		case SET_RATE_EVENT:		//URL清理事件
			event_lock=1;			//受保护状态事件
			WriteEventlockLog("eventlock_start\n",event_lock);
			SetMainQueueLock(MAIN_QUEUE_LOCK);
			NetStreamExitFile();
			SetWm8960Rate(RECODE_RATE);
			event_lock=0;
			sysMes.localplayname=0;
			pause_record_audio();
			WriteEventlockLog("eventlock end\n",event_lock);
			break;
			
		case URL_VOICES_EVENT:		//URL网络播放事件
			WritePlayUrl_Log("handler url voices event \n");
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		
			NetStreamExitFile();
			start_event_play_url();
			WritePlayUrl_Log("start add url to mainQueue for play\n");
			AddDownEvent((const char *)data,URL_VOICES_EVENT);
			sleep(3);
			break;
			
#ifdef 	LOCAL_MP3
		case LOCAL_MP3_EVENT:		//本地音乐播放事件
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		//去除清理锁
			NetStreamExitFile();
			start_event_play_url();
			AddDownEvent((const char *)data,LOCAL_MP3_EVENT);
			DEBUG_STD_MSG("handle_event_msg LOCAL_MP3_EVENT add end\n");
			break;
#endif
#ifdef TEST_PLAY_EQ_MUSIC
		case TEST_PLAY_EQ_WAV:
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		//去除清理锁
			SetplayWavEixt();
			start_event_play_url();
			AddDownEvent((const char *)data,TEST_PLAY_EQ_WAV);		
			break;
#endif			
		case QTTS_PLAY_EVENT:		//QTTS事件
			SetplayNetwork_Lock();	
			PlayQttsText(data,cur->len);
			free((void *)data);
			break;
			
#ifdef SPEEK_VOICES	
		case SPEEK_VOICES_EVENT:	//接收到语音消息	
			start_event_play_wav();
			playspeekVoices(data);
			pause_record_audio();
			usleep(1000);
			free((void *)data);
			break;
			
		case TALK_EVENT_EVENT:		//对讲事件
			Handle_WeixinSpeekEvent(cur->len);
			break;
#endif
#if defined(HUASHANG_JIAOYU)
		case XUNFEI_AIFI_EVENT:
			SetMainQueueLock(MAIN_QUEUE_UNLOCK);		//去除清理锁
			NetStreamExitFile();
			start_event_play_url();
			//添加到主线程当中播放华上教育内容
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

/*
@  初始化事件处理消息线程 ,创建一件队列 ,队列名为EventQue
*/
void InitEventMsgPthread(void){
	EventQue = InitCondWorkPthread(HandleEventMessage);
	init_iat_MSPLogin(WriteStreamPcmData);
}
/*
@  清除事件处理消息线程
*/
void CleanEventMsgPthread(void){
	CleanCondWorkPthread(EventQue,CleanEventMessage);
	iat_MSPLogout();
}
