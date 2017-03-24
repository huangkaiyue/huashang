#include "comshead.h"
#include "base/pool.h"
#include "config.h"
#include "host/voices/wm8960i2s.h"


//��ʼ����, �ӿڼ��ݣ���Ҫȥ��streamLen
static void tulingStartDown(const char *filename,int streamLen){
	initputPcmdata();
	printf("filename =%s streamLen=%d\n",filename,streamLen);
	setPlayAudioSize(streamLen);
}
//��ȡ��������
static void  tulingGetStreamData(const char *data,int size){
	putPcmdata((const void *)data,size);
}
//��������
static void  tulingEndDown(int downLen){
	printf("tulingEndDown mp3 \n");
}

void downTulingMp3(const char *url){
#ifdef TEST_DOWNFILE
	test_playTuingPcmFile();
#else
	setDowning();
	RequestTulingLog("downTulingMp3 start",1);
	demoDownFile(url,15,tulingStartDown,tulingGetStreamData,tulingEndDown);
	SetDownExit();
	RequestTulingLog("downTulingMp3 wait",1);
	WaitPthreadExit();
	RequestTulingLog("downTulingMp3 end",1);
	printf("downTulingMp3 exit mp3 \n");
#endif
}
