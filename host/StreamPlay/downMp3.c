#include "comshead.h"
#include "base/pool.h"
#include "curldown.h"
#include "host/voices/wm8960i2s.h"
#include "config.h"

#define EXIT_TULING_PLAY 		0	//退出图灵播放
#define TULING_PLAY_ING 		1	//正在播放下载内容
#define WAIT_TULING_PLAY_EXIT 	2	//等待退出图灵

#define DEBUG_DOWN_MP3
#ifdef DEBUG_DOWN_MP3  
#define DEBUG_DOWN(fmt, args...)     printf("%s: "fmt,__func__, ## args)
#else   
#define DEBUG_DOWN(fmt, args...) { }  
#endif  


static unsigned char playTuling_lock=0;

void SetplayNetwork_Lock(void){
	playTuling_lock=PLAY_NETWORK_VOICES_LOCK;
}
//设置在线播放上锁
void SetplayNetwork_unLock(void){
	playTuling_lock=PLAY_NETWORK_VOICES_UNLOCK;
}
unsigned char GetplayNetwork_LockState(void){
	return playTuling_lock;
}

void ExitPlayNetworkPlay(void){
	__ExitQueueQttsPlay();
	while(1){
		unsigned char playTuling_lock = GetplayNetwork_LockState();
		if(playTuling_lock==PLAY_NETWORK_VOICES_UNLOCK){
			break;
		}
		usleep(1000);
		//DEBUG_DOWN("waiting exit network play \n");
		if(getDownState()==DOWN_ING){		//退出下载
			quitDownFile();
		}
	}
	DEBUG_DOWN("exit ok \n");
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
	setDowning();
	StartPthreadPlay();
	RequestTulingLog("downTulingMp3 start",1);
	DEBUG_DOWN("start down tuling file \n");
	demoDownFile(url,15,tulingStartDown,tulingGetStreamData,tulingEndDown);
	RequestTulingLog("downTulingMp3 wait",1);
	DEBUG_DOWN("exit down tuling file \n");
	WaitPthreadExit();
	RequestTulingLog("downTulingMp3 end",1);
	DEBUG_DOWN("--------------downTulingMp3 exit mp3 -------------\n");
}
