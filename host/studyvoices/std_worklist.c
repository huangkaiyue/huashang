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
@函数功能:	文本处理函数
@参数:	text 文本文字
******************************************/
static void handle_text(char *text)
{
	tolkLog("tolk_start\n");
	int ret=0;
	//检查关键词，做出相应的回答
	if(check_text_cmd(text)){
		//pause_record_audio();------请在check_text_cmd函数中添加
		return ;
	}
	tolkLog("tolk handle qtts start\n");
	ret = PlayQttsText(text,QTTS_GBK);
	if(ret == 10202){
		//重连，语音播报
		playsysvoices(REQUEST_FAILED);
		startServiceWifi();
	}
	tolkLog("tolk handle qtts end\n");
}
/*******************************************
@函数功能:	json解析服务器数据
@参数:	pMsg	服务器数据
@		handle_jsion	解析后处理函数
@		textString	解析后的数据
@返回值:	0	成功	其他整数都是错误码
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
@函数功能:	上传数据到服务器并获取到回复
@参数:	voicesdata 上传数据
@		len	上传数据大小
@		voices_type	数据类型
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
	start_event_play_wav();//暂停录音
	audio = base64_encode(voicesdata, len);
	if(!audio){
		create_event_system_voices(5);
		goto exit1;
	}
	DEBUG_STD_MSG("up voices data ...(len=%d)\n",len);
	
	tulingLog("tuling_start",err);
	int r = sigsetjmp(env,1);  //保存一下上下文
	tulingLog("tuling env start...",err);
	if(r== 0){
		signal(SIGSEGV, recvSignal);  //出现段错误回调函数
		DEBUG_STD_MSG("set signal ok \n");
		///图灵服务器上，内部可能会发生错误的代码 
		if((err=tl_req_voice(audio, len, RECODE_RATE, voices_type,\
								key, &result,&resSize,&text,&textSize)))
		{
			tulingLog("tl_req_voice error",err);
			if(err==5||err==6){
				endtime=time(&t);
				if((endtime-starttime)<15)//重连，语音播报
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
	start_event_play_wav();//暂停录音
	DEBUG_STD_MSG("up voices data ...(len=%d)\n",len);
	err=reqTlVoices(10,key,(const void *)voicesdata,len,RECODE_RATE,voices_type,&text,&textSize);
	if(err==-1){
		create_event_system_voices(5);
		startServiceWifi();
		goto exit1;
	}else if(err==1){
		QttsPlayEvent("我就不回答你。",QTTS_GBK);
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
@函数功能:	学习音事件处理函数
@参数:	data 数据 len 数据大小
*******************************************************/
static void runJsonEvent(const char *data)
{
	parseJson_string(data,handle_text);
	free(data);
	//playsysvoices(TULING_DIDI);
}
int event_lock=0;
/*******************************************************
@函数功能:	添加事件到链表
@参数:	databuf 数据	len 数据长度 type事件类型
@返回值:	-1 添加失败 其它添加成功
********************************************************/
int add_event_msg(const char *databuf,int  len,int  type)
{
	int msgSize=0;
	if(event_lock){
		DEBUG_STD_MSG("add_event_msg event_lock =%d\n",event_lock); // 写入 type event_lock a+
		eventlockLog("event_lock add error\n",event_lock);
		return ;
	}
	if(type!=LOCAL_MP3_EVENT)	//不为本地播放清理播放上下曲
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
@函数功能:	事件处理函数
@参数:	data 数据	msgSize事件类型以及数据大小结构体
********************************************************/
static void handle_event_msg(const char *data,int msgSize)
{
	struct eventMsg *cur =(struct eventMsg *)(&msgSize); 
	DEBUG_STD_MSG("=====================================================================================\n");
	DEBUG_STD_MSG("handle_event_msg cur->type = %d\n",cur->type);
	DEBUG_STD_MSG("=====================================================================================\n");
	handleeventLog("handleevent_start\n",cur->type);
	switch(cur->type){
		case STUDY_WAV_EVENT:		//会话事件
#ifdef QITUTU_SHI
			Led_System_vigue_close();
#endif
			runJsonEvent(data);
			//pause_record_audio();
			break;
			
		case SYS_VOICES_EVENT:		//系统音事件
			start_event_play_wav();
			handle_event_system_voices(cur->len);
			if(cur->len!=2)
				pause_record_audio();
			break;
			
		case SET_RATE_EVENT:		//URL清理事件
			event_lock=1;	//受保护状态事件
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
			
		case URL_VOICES_EVENT:		//URL网络播放事件
			playurlLog("url play\n");
			cleanplayEvent(0);
			NetStreamExitFile();
			start_event_play_url();
			playurlLog("NetStreamExitFile\n");
			AddDownEvent(data,URL_VOICES_EVENT);
			sleep(3);
			break;
			
#ifdef 	LOCAL_MP3
		case LOCAL_MP3_EVENT:		//本地音乐播放事件
			cleanplayEvent(0);
			NetStreamExitFile();
			start_event_play_url();
			AddDownEvent(data,LOCAL_MP3_EVENT);
			break;
#endif
			
		case QTTS_PLAY_EVENT:		//QTTS事件
			start_event_play_wav();
			stait_qtts_cache();
			PlayQttsText(data,cur->len);
			//pause_record_audio();
			free((void *)data);
			break;
			
#ifdef SPEEK_VOICES	
		case SPEEK_VOICES_EVENT:	//接收到语音消息	
			playspeekVoices(data);	
			pause_record_audio();
			remove(data);
			usleep(1000);
			free((void *)data);
			break;
			
		case TALK_EVENT_EVENT:		//对讲事件
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
