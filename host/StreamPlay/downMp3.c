#include "comshead.h"
#include "base/pool.h"
#include "base/head_mp3.h"
#include "StreamFile.h"
#include "madplay.h"
#include "config.h"
#include "curldown.h"

//ʵ��д����Ƶ���Ľӿ�, ��Ҫ����������ڴ���λ�� inputMsg  inputSize �������������С
static void InputTulingStream(char * inputMsg,int inputSize){
	while(st->playSize+inputSize>st->cacheSize){
		if(getDownState()==DOWN_QUIT){	//�Ѿ��˳����أ�ֹͣ����	
			if( st->playSize+inputSize>=st->cacheSize){
				__safe_fread(inputMsg,st->cacheSize-st->playSize);
			}
			return ;
		}
		usleep(100);//���رȽ�����˯�ߵȴ����� ,����ֹͣ״̬,���ص�һ���̶ȣ�����  (app�ȵ���һ��ʱ�����Զ�����)
	}
	pthread_mutex_lock(&st->mutex);
	if(!__safe_fread(inputMsg,inputSize))
		st->playSize +=inputSize;
	DEBUG_STREAM(" st->playSize = %d \n",st->playSize);
	pthread_mutex_unlock(&st->mutex);
}

static void *Netplaytuling(void *arg){
	st->SetI2SRate(8000);
	DecodePlayMusic(InputTulingStream);
	cleanStreamData(st);	//״̬�л��Ƿ����
	return NULL;
}

//��ʼ����, �ӿڼ��ݣ���Ҫȥ��streamLen
static void tulingStartDown(const char *filename,int streamLen){
	DEBUG_STREAM("filename =%s streamLen=%d\n",filename,streamLen);
	if(st->wfp==NULL){
		st->wfp = fopen("/home/cache.tmp","w+");
		if(st->wfp==NULL){
			perror("fopen write failed ");
			return ;
		}
		fseek(st->wfp,streamLen+1,SEEK_SET);
		fwrite("\0",1,1,st->wfp);
		fseek(st->wfp,0,SEEK_SET);
	}
	st->streamLen=streamLen;
	snprintf(st->mp3name,128,"%s",filename);
	st->player.playState=MAD_PLAY;
	pool_add_task(Netplaytuling,NULL);
}
//��ȡ��������
static void  tulingGetStreamData(const char *data,int size){
	pthread_mutex_lock(&st->mutex);
	if(!__safe_write(st->wfp,(const void *)data,size))
		st->cacheSize +=size;
	pthread_mutex_unlock(&st->mutex);
}
//��������
static void  tulingEndDown(int downLen){
	SetDecodeSize(downLen);
	if(st->wfp)
		fclose(st->wfp);
	st->wfp=NULL;
}
void downTulingMp3(const char *url){
	setDowning();
	st->cacheSize =0;
	st->playSize=0;
	st->player.playState=MAD_NEXT;
	demoDownFile(url,15,tulingStartDown,tulingGetStreamData,tulingEndDown);
}

