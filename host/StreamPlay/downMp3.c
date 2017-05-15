#include "comshead.h"
#include "base/pool.h"
#include "curldown.h"
#include "host/studyvoices/std_worklist.h"
#include "host/voices/callvoices.h"
#include "log.h"

static unsigned int playTulingEventNums=0;
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
		return;
	}
	putPcmStreamToQueue((const void *)data,size);
}

//结束下载
static void  tulingEndDown(int downLen){
	printf("tulingEndDown mp3 \n");
}

void downTulingMp3(HandlerText_t *handtext){
	setDowning();
	RequestTulingLog("downTulingMp3 start",1);
	playTulingEventNums = handtext->EventNums;
	demoDownFile(handtext->data,15,tulingStartDown,tulingGetStreamData,tulingEndDown);
	RequestTulingLog("downTulingMp3 end",1);
}
