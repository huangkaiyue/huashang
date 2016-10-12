#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "base/pool.h"
#include "base/head_mp3.h"
#include "StreamFile.h"
#include "madplay.h"
#include "config.h"
#include "curldown.h"

static Mp3Stream *st=NULL;

#ifdef SAFE_READ_WRITER
static int __safe_fread(char *data,int input_size)
{
	int ret=0,r_size=0;
	if(st->rfp==NULL){
		st->rfp = fopen("/home/cache.tmp","r");
		if(st->rfp==NULL){
			perror("fopen read failed ");
			return -1;
		}
	}
	fseek(st->rfp,st->playSize,SEEK_SET);	//跳转到指定的位置播放
	while(1){
		ret = fread(data+r_size,1,input_size-r_size,st->rfp);
		r_size +=ret;
		if(r_size==input_size){
			break;
		}
		DEBUG_STREAM("r_size =%d\n",r_size);
	}
	fclose(st->rfp);
	st->rfp=NULL;
	return 0;
}

#endif
static void cleanStreamData(Mp3Stream *st)
{
	st->channel=0;
	st->rate=0;
#ifndef SAFE_READ_WRITER
	if(st->rfp)
		fclose(st->rfp);
	st->rfp=NULL;
#endif
	st->player.playState=MAD_EXIT;
}
//实现写入音频流的接口, 需要输入的数据内存存放位置 inputMsg  inputSize 输入的数据流大小
static void InputNetStream(char * inputMsg,int inputSize)
{
	while(st->playSize+inputSize>st->cacheSize){
		if(getDownState()==DOWN_QUIT){	//已经退出下载，停止播放	
			DEBUG_STREAM("exit ...st->cacheSize =%d st->playSize%d\n",st->cacheSize,st->playSize);
			if(st->cacheSize > st->playSize){
				if(st->streamLen != 0){
					st->player.progress = (st->playSize*100)/st->streamLen;
					st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);	//回应播放进度
				}
				__safe_fread(inputMsg,st->cacheSize-st->playSize);
				memset(inputMsg+st->cacheSize-st->playSize,0,st->playSize+inputSize-st->cacheSize);
			}
			DecodeExit();
			return ;
		}
		usleep(100);//下载比较慢，睡眠等待下载
	}
	
#ifdef SEEK_TO
	if(st->streamLen != 0)
	{
#ifdef SHOW_progressBar	
		float getPercent=0;
		progressBar(st->playSize,st->streamLen,&getPercent);
#endif		
		st->player.progress = (st->playSize*100)/st->streamLen;
		if((st->player.progress>23&&st->player.progress<28)||(st->player.progress>48&&st->player.progress<53)\
			||(st->player.progress>73&&st->player.progress<78)||st->player.progress>90)
		{
			st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
		}
		else{
			st->ack_playCtr(UDP_ACK,&st->player,st->player.playState);	//回应播放进度
		}
		//DEBUG_STREAM("ack_playCtr  progress ok \n");
	}
	else{
		DEBUG_STREAM("\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n ");
		DEBUG_STREAM(" InputNetStream : streamLen == %d \n ",st->streamLen);
		DEBUG_STREAM(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
	}
	#endif	
	pthread_mutex_lock(&st->mutex);
	//DEBUG_STREAM(" inputSize =%d \n",inputSize);
#ifdef SAFE_READ_WRITER
	if(!__safe_fread(inputMsg,inputSize))
		st->playSize +=inputSize;
	DEBUG_STREAM(" st->playSize = %d \n",st->playSize);
#else
	if(st->rfp){
		fread(inputMsg,1,inputSize,st->rfp);
		st->playSize +=inputSize;
	}
#endif
	pthread_mutex_unlock(&st->mutex);
}

//播放网络流音频文件
static void *NetplayStreamMusic(void *arg)
{
	pthread_mutex_lock(&st->mutex);
	if(st->rfp==NULL){
		st->rfp = fopen("/home/cache.tmp","r");
		if(st->rfp==NULL){
			perror("fopen read failed ");
			pthread_mutex_unlock(&st->mutex);
			return ;
		}
	}
	get_mp3head(st->rfp,&st->rate,&st->channel);
#ifdef SAFE_READ_WRITER
	fclose(st->rfp);
	st->rfp=NULL;	
#else
	fseek(st->rfp,0,SEEK_SET);
#endif
	pthread_mutex_unlock(&st->mutex);
	if(st->rate==0)
		st->rate=44100;

	DEBUG_STREAM("music st->rate =%d st->channel=%d \n",st->rate,st->channel);
	st->SetI2SRate(st->rate);
	DecodePlayMusic(InputNetStream);
#if 0
	if(st->player.progress>95){
		st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//发送结束状态
	}else{
		st->player.playState=MAD_NEXT;
		st->ack_playCtr(TCP_ACK,&st->player,MAD_NEXT);
	}
#endif
	st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//发送结束状态
	cleanStreamData(st);	//状态切换是否加锁
	
	DEBUG_STREAM("exit play ok (%d)\n",get_playstate());
	return NULL;
}
//开始下载, 接口兼容，需要去掉streamLen
static void NetStartDown(const char *filename,int streamLen)
{
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
}
//获取到流数据
static void NetGetStreamData(const char *data,int size)
{
	pthread_mutex_lock(&st->mutex);
#ifdef SAFE_READ_WRITER	
	if(!__safe_write(st->wfp,(const void *)data,size))
		st->cacheSize +=size;
#else
	if(st->wfp){
		fwrite(data,1,size,st->wfp);
		st->cacheSize +=size;
	}
#endif
	pthread_mutex_unlock(&st->mutex);
	//队列缓存到8*KB数据，开始播放
	if(st->player.playState==MAD_NEXT&&st->cacheSize>CACHE_PLAY_SIZE&&getDownState()==DOWN_ING)
	{
		st->player.playState=MAD_PLAY;
		pool_add_task(NetplayStreamMusic,NULL);
	}
}
//结束下载
static void NetEndDown(int downLen)
{
	char buf[200]={0};
	DEBUG_STREAM("OK \n");
	//st->cacheSize=downLen;
	SetDecodeSize(downLen);
#ifdef PALY_URL_SD
	if(CheckSdcardInfo(MP3_SDPATH)){	//内存不足50M删除指定数目的歌曲
		DelSdcardMp3file(MP3_SDPATH);
	}
	if(downLen==st->streamLen){
		snprintf(buf,200,"%s %s %s%s","cp",URL_SDPATH,MP3_SDPATH,st->mp3name);
		system(buf);
	}
#endif
	if(st->wfp)
		fclose(st->wfp);
	st->wfp=NULL;
}

//退出当前流下载和播放
void NetStreamExitFile(void)
{
	DEBUG_STREAM("NetStreamExitFile start (%d)...\n",getDownState());
	if(getDownState()==DOWN_ING){		//退出下载
		quitDownFile();
	}
	//DEBUG_STREAM("NetStreamExitFile getDownState (%d)...\n",st->player.playState);
	pthread_mutex_lock(&st->mutex);
	while(st->player.playState==MAD_PLAY||st->player.playState==MAD_PAUSE){	//退出播放
		DecodeExit();
		usleep(100);
	}
	pthread_mutex_unlock(&st->mutex);
	DEBUG_STREAM("NetStreamExitFile end ...\n");
}

//拷贝推送过来的信息
static void CopyUrlMessage(Player_t *App,Player_t *Dev){
	if(strlen(App->playfilename)==0){
		
	}
	else{
		snprintf(Dev->playfilename,128,"%s",App->playfilename);	
	}
	if(strlen(App->musicname)==0){
		
	}
	else{
		snprintf(Dev->musicname,64,"%s",App->musicname);
		Dev->musicTime = App->musicTime;
	}
}
//开始边下边播放 
int NetStreamDownFilePlay(const void *data)
{
	int ret=0;
	setDowning();
	Player_t *play =(Player_t *)data; 
	st->player.progress=0;
	st->streamLen=0;
	st->playSize=0;
	st->cacheSize=0;
#ifndef PALY_URL_SD
	CopyUrlMessage((Player_t *)data,(Player_t *)&st->player);
	//snprintf(st->player.playfilename,128,"%s",play->playfilename);
#endif
	st->player.playState=MAD_NEXT;
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);

	demoDownFile(play->playfilename,15,NetStartDown,NetGetStreamData,NetEndDown);
	DEBUG_STREAM("NetStreamDownFilePlay end ...\n");
	return ret;
}
//暂停
void StreamPause(void)
{
	if(st->player.playState==MAD_PLAY)
	{
		st->player.playState=MAD_PAUSE;
		DecodePause();
	}else{
		DEBUG_STREAM("set pause failed\n");
	}
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
}
//播放
void StreamPlay(void)
{
	if(st->player.playState==MAD_PAUSE)
	{
		st->player.playState=MAD_PLAY;
		DecodeStart();
	}else{
		DEBUG_STREAM("set play failed\n");
	}
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
}
//进度条控制播放 
void seekToStream(int progress)
{
#ifdef SEEK_TO
	int curprogress=0;
	if(progress<=0)
		curprogress=1;
	else if(progress>100)
		progress=100;
	DEBUG_STREAM("progress =%d\n",progress);
	if(st->player.playState==MAD_PLAY){
		pthread_mutex_lock(&st->mutex);
		st->playSize=st->streamLen*progress/100;//计算出当前播放长度	
		pthread_mutex_unlock(&st->mutex);
	}
#endif
}

static void InputlocalStream(char * inputMsg,int inputSize)
{
	while(st->playSize+inputSize>=st->streamLen){
		fread(inputMsg,1,st->streamLen-st->playSize,st->rfp);
		memset(inputMsg+st->streamLen-st->playSize,0,st->playSize+inputSize-st->streamLen);
		DecodeExit();
		return ;
	}
#ifdef SEEK_TO
	if(st->streamLen != 0)
	{
#ifdef SHOW_progressBar		
		float getPercent=0;
		progressBar(st->playSize,st->streamLen,&getPercent);
#endif		
		st->player.progress = (st->playSize*100)/st->streamLen;
		st->ack_playCtr(UDP_ACK,&st->player,st->player.playState);	//回应播放进度
	}
	else{
		DEBUG_STREAM(" InputNetStream : streamLen == %d \n ");
	}
#endif
	fread(inputMsg,1,inputSize,st->rfp);
	st->playSize+=inputSize;
}

void playLocalMp3(const char *mp3file)
{
	st->player.progress=0;
	st->streamLen=0;
	st->playSize=0;
	st->rfp = fopen(mp3file,"r");
	if(st->rfp==NULL){
		perror("fopen read failed ");
		return ;
	}
	
	pthread_mutex_lock(&st->mutex);
	st->player.playState =MAD_PLAY;
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	
	get_mp3head(st->rfp,&st->rate,&st->channel);
	fseek(st->rfp,0,SEEK_END);
	st->streamLen = ftell(st->rfp);
	fseek(st->rfp,0,SEEK_SET);

	pthread_mutex_unlock(&st->mutex);
	if(st->rate==0)
		st->rate=44100;
	
	DEBUG_STREAM("music st->rate =%d st->channel=%d \n",st->rate,st->channel);
	st->SetI2SRate(st->rate);
	SetDecodeSize(st->streamLen);
	DecodePlayMusic(InputlocalStream);
	st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//发送结束状态
#if 0
		if(st->player.progress>95){
			st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//发送结束状态
		}else{
			st->player.playState=MAD_NEXT;
			st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);
		}
#endif
	cleanStreamData(st);	//状态切换是否加锁
	DEBUG_STREAM("exit play ok (%d)\n",get_playstate());
	//free((void *)mp3file);
}
#ifdef PALY_URL_SD
void PlayUrl(const void *data)
{
	char domain[64] = {0};
	char filecmd[128]={0};
	int port = 80;
	char filename[128];
	Player_t *play =(Player_t *)data;
	parse_url(play->playfilename, domain, &port, filename);
	
	CopyUrlMessage((Player_t *)data,(Player_t *)&st->player);
	char *buf= (char *)calloc(1,strlen(filename)+1+MP3_PATHLEN);
	if(buf){
		snprintf(buf,200,"%s%s",MP3_SDPATH,filename);	//更新文件播放时间
		if(!access(buf,F_OK)){
			snprintf(filecmd,128,"%s%s","chmod 777 ",buf);
			system(filecmd);
			usleep(1*1000);
			DEBUG_STREAM("PlayUrl playLocalMp3(%s) ... \n",filename);
			playLocalMp3(buf);
		}else{
			DEBUG_STREAM("PlayUrl NetStreamDownFilePlay (%s)... \n",filename);
			NetStreamDownFilePlay(data);
		}
		free((void *)buf);
	}
}
#endif

#ifdef WORK_INTER
void test_quikSeekTo(void){
	st->player.progress +=10;
	if(st->player.progress<=0)
		st->player.progress=1;
	else if(st->player.progress >=100)
		st->player.progress =100;
	seekToStream(st->player.progress);
}
void test_backSeekTo(void){
	st->player.progress -=10;
	if(st->player.progress<=0)
		st->player.progress=1;
	else if(st->player.progress >=100)
		st->player.progress =100;
	seekToStream( st->player.progress);
}
#endif
void getStreamState(void *data,void StreamState(void *data,Player_t *player))	//获取播放流状态
{
	st->player.vol = st->GetVol();
	StreamState(data,&st->player);
}
void initStream(void ack_playCtr(int nettype,Player_t *player,unsigned char playState),void WritePcmData(char *data,int size),void SetI2SRate(int rate),int GetVol(void))
{
	st = calloc(1,MP3STEAM_SIZE);
	if(st==NULL)
	{
		perror("calloc st memory failed");
		return ;
	}
	st->ack_playCtr=ack_playCtr;
	st->SetI2SRate=SetI2SRate;
	st->GetVol=GetVol;
	pthread_mutex_init(&st->mutex, NULL);
	initCurl();
	DEBUG_STREAM("ok ...\n");
	InitDecode(WritePcmData);
}
void cleanStream(void)
{
	if(st!=NULL){
		pthread_mutex_destroy(&(st->mutex)); 
		free(st);
		st=NULL;
	}
	cleanCurl();
	CleanDecode();
}
