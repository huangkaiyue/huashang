#include "comshead.h"
#include "base/pool.h"
#include "curldown.h"
#include "host/studyvoices/std_worklist.h"
#include "host/voices/callvoices.h"
#include "log.h"
#include "../studyvoices/qtts_qisc.h"
static unsigned int playTulingEventNums=0;
static int playRet=-1;
//��ʼ����, �ӿڼ��ݣ���Ҫȥ��streamLen
static void tulingStartDown(const char *filename,int streamLen){
	initputPcmdata();
	printf("filename =%s streamLen=%d\n",filename,streamLen);
	setPlayAudioSize(streamLen);
}

//��ȡ��������
static void  tulingGetStreamData(const char *data,int size){
	if(playTulingEventNums!=GetCurrentEventNums()){
		quitDownFile();
		playRet=-1;
		return;
	}
	putPcmStreamToQueue((const void *)data,size);
}

//��������
static void  tulingEndDown(int downLen){
	printf("tulingEndDown mp3 \n");
}

int downTulingMp3_forPlay(HandlerText_t *handtext){
	SetWm8960Rate(RECODE_RATE); 
	setDowning();
	playRet=0;
	RequestTulingLog("downTulingMp3_forPlay start",1);
	playTulingEventNums = handtext->EventNums;
	demoDownFile(handtext->data,10,tulingStartDown,tulingGetStreamData,tulingEndDown);
	if(handtext->playLocalVoicesIndex==TULING_TEXT_MUSIC){	//��ʾ����ͼ������Ĺ��º͸���
		while(getPlayVoicesQueueNums()>0&&playRet==0){	
			usleep(1000);
		}
	}
	pause_record_audio();
	RequestTulingLog("downTulingMp3_forPlay end",1);
	free((void *)handtext->data);
	free((void *)handtext);
	return playRet;
}
