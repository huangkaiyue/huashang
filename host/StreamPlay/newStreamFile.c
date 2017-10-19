#include "comshead.h"
#include "base/pool.h"

#include "base/fileMes.h"
#include "StreamFile.h"
#include "mplay.h"
#include "config.h"
#include "curldown.h"
#include "host/voices/callvoices.h"
#include "../voices/gpio_7620.h"
#include "uart/uart.h"
#include "log.h"

Mp3Stream *st=NULL;

#ifdef SAFE_READ_WRITER
int __safe_fread(char *data,int input_size){
	int ret=0,r_size=0;
	if(st->rfp==NULL){
		st->rfp = fopen(URL_SDPATH,"r");
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
void cleanStreamData(Mp3Stream *st){
	st->channel=0;
	st->rate=0;
#ifndef SAFE_READ_WRITER
	if(st->rfp)
		fclose(st->rfp);
	st->rfp=NULL;
#endif
	memset(st->mp3name,0,128);
	st->player.playState=MAD_EXIT;
}

static void GetMusicMessage(int rate,int channels){
	st->rate = rate;
	if(GetCurrentEventNums()!=st->eventNums){
		DecodeExit();
		st->lockSetRate=0;
		return ;
	}
	st->SetI2SRate(rate,"NetplayStreamMusic set rate");
	st->lockSetRate=0;
}
//实现写入音频流的接口, 需要输入的数据内存存放位置 inputMsg  inputSize 输入的数据流大小
static void InputNetStream(const void * inputMsg,int inputSize){
	printf("InputNetStream inputSize=%d\n",inputSize);
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
			st->player.proflag=0;
			DecodeExit();
			return ;
		}
		if(st->wait==0){
			st->wait=1;
			st->ack_playCtr(TCP_ACK,&st->player,MAD_PAUSE);
		}
		usleep(100);//下载比较慢，睡眠等待下载 ,发送停止状态,下载到一定程度，唤醒  (app等到下一个时刻再自动唤醒)
	}
	if(st->wait==1){
		st->wait=0;
		st->ack_playCtr(TCP_ACK,&st->player,MAD_PLAY);
	}

	pthread_mutex_lock(&st->mutex);
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
	if(GetCurrentEventNums()!=st->eventNums){
		quitDownFile();
		DecodeExit();
		printf("\n InputNetStream-------------------------\n interrupt exit\n ------------------\n");
	}

	pthread_mutex_unlock(&st->mutex);
}
int GetFileNameForPath(const char *path){
	int i=0;
	int size=strlen(path);
	char *p;
	for(i=size;i>0;i--){
		p=strstr(path+i, "/");
		if(p==NULL)
			continue;
		printf("path :%s\n",path);
		break;
	}
	return (size-strlen(p)+1);
}

//播放网络流音频文件
static void *NetplayStreamMusic(void *arg){
	pthread_mutex_lock(&st->mutex);
	if(st->rfp==NULL){
		st->rfp = fopen(URL_SDPATH,"r");
		if(st->rfp==NULL){
			perror("fopen read failed ");
			st->player.playState=MAD_EXIT;
			DecodeExit();
			pthread_mutex_unlock(&st->mutex);
			return NULL;
		}
	}
	
#ifdef SAFE_READ_WRITER
	fclose(st->rfp);
	st->rfp=NULL;
#else
	fseek(st->rfp,0,SEEK_SET);
#endif
	pthread_mutex_unlock(&st->mutex);
	if(st->rate==0||st->rate==8000)
		st->rate=44100;

	DEBUG_STREAM("music st->rate =%d st->channel=%d \n",st->rate,st->channel);
	
	DecodePlayMusic(GetMusicMessage,InputNetStream);

	st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//发送结束状态
	cleanStreamData(st);	//状态切换是否加锁
	
	DEBUG_STREAM("exit play ok \n");
	return NULL;
}

//开始下载, 接口兼容，需要去掉streamLen
static void NetStartDown(const char *filename,int streamLen){
	DEBUG_STREAM("filename =%s streamLen=%d\n",filename,streamLen);
	if(st->wfp==NULL){
		st->wfp = fopen(URL_SDPATH,"w+");
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
static void NetGetStreamData(const char *data,int size){
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
	if(st->player.playState==MAD_NEXT&&st->cacheSize>CACHE_PLAY_SIZE&&getDownState()==DOWN_ING){
		st->player.playState=MAD_PLAY;
		DecodeStart();
		pool_add_task(NetplayStreamMusic,NULL);
	}
}
//结束下载
static void NetEndDown(int downLen){
	DEBUG_STREAM("OK \n");
	//st->cacheSize=downLen;
	SetDecodeSize(downLen);
	if(st->wfp)
		fclose(st->wfp);
	st->wfp=NULL;
}
static int error_timeout_check=0;
//退出当前流下载和播放
void NetStreamExitFile(void){
	if(error_timeout_check>0){
		printf("\n ..........error exit ...........\n");
		return;
	}
	printf("start exit NetStreamExitFile \n");
	error_timeout_check=1;
#if 0	
	if(getDownState()==DOWN_ING){		//退出下载
		printf("start exit quitDownFile \n");
		quitDownFile();
		WriteEventlockLog("eventlock quitDownFile",2);
	}
#endif	

	WriteEventlockLog("NetStreamExitFile",(int)st->player.playState);

	printf("%s: ... v2 rate =%d\n",__func__,st->rate);
	while(st->player.playState==MAD_PLAY||st->player.playState==MAD_PAUSE){	//退出播放
		if(st->lockNetwork==0&&st->lockLocalPlay==0){
			break;
		}
		WriteEventlockLog("eventlock wait exit mp3 state",(int)st->player.playState);
		//pthread_mutex_lock(&st->mutex);
		st->player.progress=0;
		st->player.musicTime=0;
		st->player.proflag=0;
		memset(st->player.musicname,0,64);
		DecodeExit();
		//quitDownFile();
		//pthread_mutex_unlock(&st->mutex);
		printf("%s: while wait exit error_timeout_check=%d st->player.playState=%d\n",__func__,error_timeout_check,st->player.playState);
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
			WriteEventlockLog("error exit ,and set  ",(int)st->player.playState);
			printf("%s: error RECODE_PAUSE exit ,and set ... st->player.playState=%d\n",__func__,st->player.playState);
			st->player.playState=MAD_NEXT;
			break;
		}
		if(++error_timeout_check>20){
			WriteEventlockLog("error timeout_check ,and set exit",(int)st->player.playState);
			printf("%s: error timeout_check exit ,and set ... st->player.playState=%d\n",__func__,st->player.playState);
			st->player.playState=MAD_NEXT;
			break;
		}
		usleep(1000);
	}
	printf("%s: play end ... st->player.playState=%d\n",__func__,st->player.playState);
	WriteEventlockLog("eventlock exit end",st->player.playState);
	error_timeout_check=0;
}

//拷贝推送过来的信息
static void CopyUrlMessage(Player_t *srcPlayer,Player_t *DestPlayer){
	snprintf(DestPlayer->playfilename,128,"%s",srcPlayer->playfilename);	
	snprintf(DestPlayer->musicname,64,"%s",srcPlayer->musicname);
	DestPlayer->musicTime = srcPlayer->musicTime;
}
int getLockNetwork(void){
	return (int)st->lockNetwork;
}
int getlockSetRate(void){
	return (int)st->lockSetRate;
}
//开始边下边播放 
static int NetStreamDownFilePlay(Player_t *play,int EventNums){
	int ret=0;
	setDowning();
	st->player.progress=0;
	st->streamLen=0;
	st->playSize=0;
	st->cacheSize=0;
	WritePlayUrl_Log("play url",play->playfilename);
	st->player.playState=MAD_NEXT;
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	st->lockNetwork=1;
	demoDownFile(play->playfilename,15,NetStartDown,NetGetStreamData,NetEndDown);
	while(st->player.playState!=MAD_EXIT){
		printf("wait exit play state: %d GetStreamPlayState()=%d \n",GetRecordeVoices_PthreadState(),GetStreamPlayState());
		usleep(500000);
		if(st->GetWm8960Rate==8000){
			printf("\n-------------------------\nplay music rate is error exit\n ------------------\n");
			DecodeExit();
			st->player.playState=MAD_EXIT;
			break;
		}
		if(GetDecodeState()==MAD_EXIT){
			printf("\n-------------------------\n not run play pthread exit\n ------------------\n");
			break;
		}
		if(GetCurrentEventNums()!=EventNums){
			//quitDownFile();	// fix bug interrupt network for main pthread loop ( meybe fix problem 2017-09-19-1:59  )
			DecodeExit();
			printf("\n-------------------------\n interrupt exit\n ------------------\n");
			break;
		}
	}
	if(GetCurrentEventNums()==EventNums){	// fix tuling interrupt change recoder voices (meybe fix problem  2017-09-19-03:33)
		pause_record_audio();
	}
	st->lockNetwork=0;
	//printf("NetStreamDownFilePlay end ...GetStreamPlayState()=%d\n",GetStreamPlayState());
	return ret;
}
//暂停
void StreamPause(void){
	if(st->player.playState==MAD_PLAY){
		st->player.playState=MAD_PAUSE;
		DecodePause();
		PlayorPause();
	}else{
		DEBUG_STREAM("set pause failed\n");
	}
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
}
//播放
void StreamPlay(void){
	if(st->player.playState==MAD_PAUSE){
		st->player.playState=MAD_PLAY;
		DecodeStart();
		PlayorPause();
	}else{
		DEBUG_STREAM("set play failed\n");
	}
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
}
//设置当前流播放状态
void SetStreamPlayState(unsigned char playliststate){
	st->player.playListState=playliststate;
	printf("\n %s: st->player.playListState =%d\n",st->player.playListState);
}
//切换单曲和列表播放
void GpioKey_SetStreamPlayState(void){
	if(st->player.playListState==MUSIC_PLAY_LIST){
		st->player.playListState=MUSIC_SINGLE_LIST;
		printf("\n %s :is single music\n",__func__);
	}else{
		printf("\n %s:is list play\n",__func__);
		st->player.playListState=MUSIC_PLAY_LIST;
	}
}
//获取当前流播放状态
int GetStreamPlayState(void){
	return (int)st->player.playListState; 
}
int GetPlayMusicState(void){
	return (int)st->player.playState;
}
//按键切换播放状态
void keyStreamPlay(void){
	if(st->player.playState==MAD_PAUSE){
		st->player.playState=MAD_PLAY;
		DecodeStart();
		PlayorPause();
		Show_musicPicture();
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	}else if(st->player.playState==MAD_PLAY){
		st->player.playState=MAD_PAUSE;
		DecodePause();
		PlayorPause();
		showFacePicture(PLAY_PAUSE);
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	}else if(st->player.playState==MAD_EXIT){
		Huashang_GetScard_forPlayMusic(PLAY_NEXT,EXTERN_PLAY_EVENT);//暂停状态，添加华上教育歌曲播放
		usleep(1000);//防止添加按键太快	
	}
}
//进度条控制播放 
void seekToStream(int progress){
#ifdef SEEK_TO
	if(st->player.playState!=MAD_PLAY){
		return;
	}
	int curprogress=0;
	if(progress<=0)
		curprogress=1;
	else if(progress>100)
		progress=100;
	DEBUG_STREAM("progress =%d\n",progress);
	if(st->player.playState==MAD_PLAY){
		pthread_mutex_lock(&st->mutex);
		st->playSize=st->streamLen*progress/100;//计算出当前播放长度	
		st->player.progress = progress;
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
		pthread_mutex_unlock(&st->mutex);
	}
#endif
}

static void InputlocalStream(const void * inputMsg,int inputSize){
	while(st->playSize+inputSize>=st->streamLen){
		fread(inputMsg,1,st->streamLen-st->playSize,st->rfp);
		memset(inputMsg+st->streamLen-st->playSize,0,st->playSize+inputSize-st->streamLen);
		DecodeExit();
		return ;
	}
	if(GetCurrentEventNums()!=st->eventNums){
		DecodeExit();
	}
	if(st->GetWm8960Rate==8000){
		DecodeExit();
	}
	fread(inputMsg,1,inputSize,st->rfp);
	st->playSize+=inputSize;
}
//播放本地音频流 
static void playLocalMp3(const char *mp3file){
	st->player.progress=0;
	st->streamLen=0;
	st->playSize=0;
	st->rfp = fopen(mp3file,"r");
	if(st->rfp==NULL){
		perror("fopen read failed ");
		return ;
	}
	printf("============playLocalMp3==============\n"); //---bug,不能删，浮点异常
	pthread_mutex_lock(&st->mutex);
	st->player.playState =MAD_PLAY;
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	
	printf("=============ack_playCtr=============\n"); //---bug,不能删

	fseek(st->rfp,0,SEEK_END);
	st->streamLen = ftell(st->rfp);
	fseek(st->rfp,0,SEEK_SET);

	printf("=============fseek=============\n"); //---bug,不能删

	pthread_mutex_unlock(&st->mutex);

	SetDecodeSize(st->streamLen);
	DecodeStart();
	DEBUG_STREAM("music start play \n");
	DecodePlayMusic(GetMusicMessage,InputlocalStream);
	cleanStreamData(st);	//状态切换是否加锁
	DEBUG_STREAM(" exit play ok \n");
	WriteEventlockLog("playLocalMp3  exit play ok ",(int)st->player.playState);
}
//播放歌曲接口  play: 播放信息结构体
int Mad_PlayMusic(Player_t *play,int EventNums){
	int ret =1;
	start_event_play_Mp3music();
	char domain[64] = {0};
	char filename[128]={0};
	int port = 80;
	st->eventNums=EventNums;
	printf("%s: play->playfilename =%s\n",__func__,play->playfilename);
	if(!access(play->playfilename,F_OK)){
		st->lockSetRate=1;
		st->lockLocalPlay=1;
		Show_musicPicture();
		playLocalMp3(play->playfilename);
		st->lockSetRate=0;
		st->lockLocalPlay=0;
	}else{
		if(strstr(play->playfilename,"/media/mmcblk0p1/")){
			goto exit0;
		}
		if(strstr(play->playfilename,"http")==NULL){
			pause_record_audio();
			goto exit0;
		}
		Show_musicPicture();
		parse_url(play->playfilename, domain, &port, filename);
		CopyUrlMessage(play,(Player_t *)&st->player);
		snprintf(st->mp3name,128,"%s",filename);			
		DEBUG_STREAM("network play music : %s \n",filename);
		st->lockSetRate=1;
		NetStreamDownFilePlay(play, EventNums);
		st->lockSetRate=0;	// if don't down voices >8k size  ,nerver enter  void GetMusicMessage(int rate,int channels)
	}
	ret =0;
exit0:	
	return ret;
}

//获取播放流状态
void getStreamState(void *data,void StreamState(void *data,Player_t *player)){
	st->player.vol = st->GetVol();
	StreamState(data,&st->player);
}
void initStream(void ack_playCtr(int nettype,Player_t *player,unsigned char playState),void WritePcmData(char *data,int size),void SetI2SRate(int rate,const char *function),int GetVol(void),int GetWm8960Rate(void)){
	st = calloc(1,MP3STEAM_SIZE);
	if(st==NULL){
		perror("calloc st memory failed");
		return ;
	}
	st->ack_playCtr=ack_playCtr;
	st->SetI2SRate=SetI2SRate;
	st->GetVol=GetVol;
	st->GetWm8960Rate;
	st->player.playState=MUSIC_PLAY_LIST;
	pthread_mutex_init(&st->mutex, NULL);
	initCurl();
	DEBUG_STREAM("ok ...\n");
	InitDecode(WritePcmData);
}
void cleanStream(void){
	if(st!=NULL){
		pthread_mutex_destroy(&(st->mutex)); 
		free(st);
		st=NULL;
	}
	cleanCurl();
	CleanDecode();
}
