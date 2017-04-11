#include "comshead.h"
#include "base/pool.h"
#include "config.h"
#include "host/voices/wm8960i2s.h"

#define EXIT_TULING_PLAY 		0	//�˳�ͼ�鲥��
#define TULING_PLAY_ING 		1	//���ڲ�����������
#define WAIT_TULING_PLAY_EXIT 	2	//�ȴ��˳�ͼ��

//#define DEBUG_DOWN_MP3
#ifdef DEBUG_DOWN_MP3  
#define DEBUG_DOWN(fmt, args...)     printf("%s: "fmt,__func__, ## args)
#else   
#define DEBUG_DOWN(fmt, args...) { }  
#endif  


static unsigned char playTuling_lock=0;

void SetTuling_playLock(void){
	playTuling_lock=TULING_PLAY_LOCK;
}
void SetTuling_playunLock(void){
	playTuling_lock=TULING_PLAY_UNLOCK;
}
unsigned char getTuling_playunLock(void){
	return playTuling_lock;
}

void exit_tulingplay(void){
	while(1){
		unsigned char playTuling_lock = getTuling_playunLock();
		if(playTuling_lock==TULING_PLAY_UNLOCK){
			break;
		}
		usleep(100);
		DEBUG_DOWN("waiting exit network play \n");
		SetDownExit();
		__ExitQueueQttsPlay();
	}
	DEBUG_DOWN("exit ok \n");
}


//��ʼ����, �ӿڼ��ݣ���Ҫȥ��streamLen
static void tulingStartDown(const char *filename,int streamLen){
	initputPcmdata();
	printf("filename =%s streamLen=%d\n",filename,streamLen);
	setPlayAudioSize(streamLen);
}

//��ȡ��������
static void  tulingGetStreamData(const char *data,int size){
	putPcmStreamToQueue((const void *)data,size);
}

//��������
static void  tulingEndDown(int downLen){
	printf("tulingEndDown mp3 \n");
}

void downTulingMp3(const char *url){
	setDowning();
	RequestTulingLog("downTulingMp3 start",1);
	DEBUG_DOWN("start down tuling file \n");
	demoDownFile(url,15,tulingStartDown,tulingGetStreamData,tulingEndDown);
	RequestTulingLog("downTulingMp3 wait",1);
	DEBUG_DOWN("exit down tuling file \n");
	WaitPthreadExit();
	RequestTulingLog("downTulingMp3 end",1);
	DEBUG_DOWN("--------------downTulingMp3 exit mp3 -------------\n");
}
