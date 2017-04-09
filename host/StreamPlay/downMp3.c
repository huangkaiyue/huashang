#include "comshead.h"
#include "base/pool.h"
#include "config.h"
#include "host/voices/wm8960i2s.h"

#define EXIT_TULING_PLAY 		0	//退出图灵播放
#define TULING_PLAY_ING 		1	//正在播放下载内容
#define WAIT_TULING_PLAY_EXIT 	2	//等待退出图灵

static unsigned char exitTuling=EXIT_TULING_PLAY;
void exit_tulingplay(void){
	while(1){
		unsigned char playTuling_lock = getTuling_playunLock();
		if(playTuling_lock==TULING_PLAY_UNLOCK){
			break;
		}
		usleep(100);
		printf("%s: exit tulingplay ..............\n",__func__);
		exitTuling=WAIT_TULING_PLAY_EXIT;
		SetDownExit();
	}
	printf("%s exit ok\n",__func__);
}


//开始下载, 接口兼容，需要去掉streamLen
static void tulingStartDown(const char *filename,int streamLen){
	initputPcmdata();
	printf("filename =%s streamLen=%d\n",filename,streamLen);
	setPlayAudioSize(streamLen);
}

//获取到流数据
static void  tulingGetStreamData(const char *data,int size){
	putPcmStreamToQueue((const void *)data,size);
}

//结束下载
static void  tulingEndDown(int downLen){
	printf("tulingEndDown mp3 \n");
}

void downTulingMp3(const char *url){
	exitTuling = TULING_PLAY_ING;
	setDowning();
	StartPthreadPlay();
	RequestTulingLog("downTulingMp3 start",1);
	demoDownFile(url,15,tulingStartDown,tulingGetStreamData,tulingEndDown);
	SetDownExit();
	RequestTulingLog("downTulingMp3 wait",1);
	WaitPthreadExit();
	RequestTulingLog("downTulingMp3 end",1);
	exitTuling=EXIT_TULING_PLAY;
	printf("--------------downTulingMp3 exit mp3 -------------\n");
}
