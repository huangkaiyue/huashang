#include "comshead.h"
#include "base/pool.h"
#include "config.h"
#include "host/voices/wm8960i2s.h"

#if 0
//开始下载, 接口兼容，需要去掉streamLen
static void tulingStartDown(const char *filename,int streamLen){
	initputPcmdata();
	printf("filename =%s streamLen=%d\n",filename,streamLen);
	setPlayAudioSize(streamLen);
}

//获取到流数据
static void  tulingGetStreamData(const char *data,int size){
	putPcmdata((const void *)data,size);
}

//结束下载
static void  tulingEndDown(int downLen){
	printf("tulingEndDown mp3 \n");
}
#else
#define CACHE_TULING_FILE_NAME	"./tuling.pcm"
static FILE *tulingFp=NULL; 
//开始下载, 接口兼容，需要去掉streamLen
static void tulingStartDown(const char *filename,int streamLen){
	tulingFp = fopen(CACHE_TULING_FILE_NAME,"w+");
}

//获取到流数据
static void  tulingGetStreamData(const char *data,int size){
	if(tulingFp)
		fwrite(data,size,1,tulingFp);
}


//结束下载
static void  tulingEndDown(int downLen){
	printf("%s:  tuling pcm \n",__func__);
	if(tulingFp){
		fclose(tulingFp);
		tulingFp=NULL;
	}
	printf("%s:  tuling pcm end\n",__func__);
}

#endif
#if 0
void downTulingMp3(const char *url){
#ifdef TEST_DOWNFILE
	test_playTuingPcmFile();
#else
	setDowning();
	StartPthreadPlay();
	RequestTulingLog("downTulingMp3 start",1);
	demoDownFile(url,15,tulingStartDown,tulingGetStreamData,tulingEndDown);
	SetDownExit();
	RequestTulingLog("downTulingMp3 wait",1);
	WaitPthreadExit();
	RequestTulingLog("downTulingMp3 end",1);
	printf("--------------downTulingMp3 exit mp3 -------------\n");
#endif
}
#else
#define EXIT_TULING_PLAY 		0
#define TULING_PLAY_ING 		1
#define WAIT_TULING_PLAY_EXIT 	2

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

void downTulingMp3(const char *url){
	setDowning();
	//StartPthreadPlay();
	RequestTulingLog("downTulingMp3 start",1);
	demoDownFile(url,15,tulingStartDown,tulingGetStreamData,tulingEndDown);
	SetDownExit();
	tulingFp = fopen(CACHE_TULING_FILE_NAME,"r");
	if(tulingFp==NULL){
		return ;
	}
	int readLen = I2S_PAGE_SIZE/2;
	char data[readLen];
	exitTuling = TULING_PLAY_ING;
	int ret=0;
	while(1){
		if(TULING_PLAY_ING!=exitTuling)
			goto exit1;
		ret = fread(data,1,readLen,tulingFp);
		if(ret==0){
			break;
		}
		printf("%s: %d\n",__func__,ret);
		WriteqttsPcmData(data,ret);
		printf("%s: write ok %d\n",__func__,ret);
	}
	clean_qtts_cache_2();
exit1:	
	exitTuling=EXIT_TULING_PLAY;
	printf("--------------downTulingMp3 exit mp3 -------------\n");
}

#endif
