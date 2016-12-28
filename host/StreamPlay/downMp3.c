#include "comshead.h"
#include "base/pool.h"
#include "base/head_mp3.h"
#include "StreamFile.h"
#include "madplay.h"
#include "config.h"
#include "curldown.h"

//实现写入音频流的接口, 需要输入的数据内存存放位置 inputMsg  inputSize 输入的数据流大小
static void InputTulingStream(char * inputMsg,int inputSize){
	while(st->playSize+inputSize>st->cacheSize){
		if(getDownState()==DOWN_QUIT){	//已经退出下载，停止播放	
			if( st->playSize+inputSize>=st->cacheSize){
				__safe_fread(inputMsg,st->cacheSize-st->playSize);
			}
			return ;
		}
		usleep(100);//下载比较慢，睡眠等待下载 ,发送停止状态,下载到一定程度，唤醒  (app等到下一个时刻再自动唤醒)
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
	cleanStreamData(st);	//状态切换是否加锁
	return NULL;
}

//开始下载, 接口兼容，需要去掉streamLen
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
//获取到流数据
static void  tulingGetStreamData(const char *data,int size){
	pthread_mutex_lock(&st->mutex);
	if(!__safe_write(st->wfp,(const void *)data,size))
		st->cacheSize +=size;
	pthread_mutex_unlock(&st->mutex);
}
//结束下载
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

