#include "comshead.h"
#include "base/pool.h"
#include "curldown.h"
#include "host/studyvoices/std_worklist.h"
#include "host/voices/callvoices.h"
#include "log.h"
#include "../studyvoices/qtts_qisc.h"
static unsigned int playTulingEventNums=0;
static int playRet=-1;
#define PLAY_FINNISH	0
#define INTERRUPT_PLAY	-1

//开始下载, 接口兼容，需要去掉streamLen
static void tulingStartDown(const char *filename,int streamLen){
	initputPcmdata();
	printf("filename =%s streamLen=%d\n",filename,streamLen);
	setPlayAudioSize(streamLen);
}

//获取到流数据
static void  tulingGetStreamData(const char *data,int size){
	if(playTulingEventNums!=GetCurrentEventNums()){
		quitDownFile();
		playRet=INTERRUPT_PLAY;
		return;
	}
	putPcmStreamToQueue((const void *)data,size);
}

//结束下载
static void  tulingEndDown(int downLen){
	printf("tulingEndDown mp3 \n");
}

int downTulingMp3_forPlay(HandlerText_t *handtext){
	SetWm8960Rate(RECODE_RATE); 
	setDowning();
	playRet=PLAY_FINNISH;
	RequestTulingLog("downTulingMp3_forPlay start");
	playTulingEventNums = handtext->EventNums;
	enabledownNetworkVoiceState();
	demoDownFile(handtext->data,10,tulingStartDown,tulingGetStreamData,tulingEndDown);
	if(handtext->playLocalVoicesIndex==TULING_TEXT_MUSIC){	//表示播放图灵请求的故事和歌曲
		RequestTulingLog((const char *)"downTulingMp3_forPlay wait play");
		while(getPlayVoicesQueueNums()>0&&playTulingEventNums==GetCurrentEventNums()){	
			usleep(1000);//等待图灵前缀声音播放完
			printf("wait play tuling exit : state =%d getPlayVoicesQueueNums=%d\n",GetRecordeVoices_PthreadState(),getPlayVoicesQueueNums());
		}		
	}
	disabledownNetworkVoiceState();
	RequestTulingLog("downTulingMp3_forPlay end");
	free((void *)handtext->data);
	free((void *)handtext);
	return playRet;
}
