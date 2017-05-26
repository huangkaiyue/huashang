#include "comshead.h"
#include "base/pool.h"
#include "base/head_mp3.h"
#include "base/fileMes.h"
#include "StreamFile.h"
#include "mplay.h"
#include "config.h"
#include "curldown.h"
#include "../sdcard/musicList.h"
#include "host/voices/callvoices.h"
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
static void ack_progress(void){
	st->player.progress= (st->playSize*100)/st->streamLen;
	if(st->player.progress>23&&st->player.proflag==0){
		st->player.proflag=25;
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	}else if(st->player.progress>48&&st->player.proflag==25){
		st->player.proflag=50;
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	}else if(st->player.progress>73&&st->player.proflag==50){
		st->player.proflag=75;
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	}
}
//实现写入音频流的接口, 需要输入的数据内存存放位置 inputMsg  inputSize 输入的数据流大小
static void InputNetStream(const void * inputMsg,int inputSize){
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
#ifdef SEEK_TO
	if(st->streamLen != 0)
	{
#ifdef SHOW_progressBar	
		float getPercent=0;
		progressBar(st->playSize,st->streamLen,&getPercent);
#endif		
		ack_progress();
	}
	else{
		DEBUG_STREAM("\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n ");
		DEBUG_STREAM(" InputNetStream : streamLen == %d \n ",st->streamLen);
		DEBUG_STREAM(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
	}
#endif
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
	pthread_mutex_unlock(&st->mutex);
}

#define LOVE_MP3_UNKOWN_EVENT				0		//未知状态
#define LOVE_MP3_SAVE_LOVE_MP3_EVENT		1		//添加喜爱
#define LOVE_MP3_DELETE_EVENT				2		//删除喜爱
#define DEFAULT_SAVE_MP3_EVENT				3		//默认保存mp3文件

static unsigned char like_mp3_sign=LOVE_MP3_UNKOWN_EVENT;
#ifdef PALY_URL_SD
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
static void SaveLoveMp3File(const char *filepath){
#if defined(QITUTU_SHI)	
	char runCmd[200]={0};
	switch(like_mp3_sign){
		case LOVE_MP3_SAVE_LOVE_MP3_EVENT:		//添加喜爱
			if(CheckSdcardInfo(MP3_SDPATH)){	//内存不足50M删除指定数目的歌曲
				printf("%s: delete memory \n",__func__);
				break;
			}	
			if(strcmp(st->mp3name,"")){
				//插入数据库成功
				if(InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)st->mp3name)==0){
					snprintf(runCmd,200,"cp %s %s%s",filepath,MP3_LIKEPATH,st->mp3name);
					system(runCmd);
				}
				
			}	
			like_mp3_sign=LOVE_MP3_UNKOWN_EVENT;	
			break;
		case LOVE_MP3_DELETE_EVENT:		//删除喜爱
			like_mp3_sign=LOVE_MP3_UNKOWN_EVENT;
			if(!strcmp(st->mp3name,"")){	//等于空
				int size=GetFileNameForPath(filepath);
				memcpy(st->mp3name,filepath+size,strlen(filepath));
			}
			printf("filepath: %s \t mp3name: %s \n",filepath,st->mp3name);
			if(DelXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)st->mp3name)==0)
				remove(filepath);
			break;
		case DEFAULT_SAVE_MP3_EVENT:	//默认保存到sdcard 当中
			//snprintf(buf,200,"cp %s %s%s",URL_SDPATH,MP3_SDPATH,st->mp3name);
			//system(buf);
			break;
		default:	
			break;
	}
#endif	
}

//喜爱
void Save_like_music(void){
	like_mp3_sign=LOVE_MP3_SAVE_LOVE_MP3_EVENT;
}

void Del_like_music(void){
	like_mp3_sign=LOVE_MP3_DELETE_EVENT;
}
#endif
//播放网络流音频文件
static void *NetplayStreamMusic(void *arg){
	unsigned short rate_one=0,rate_two=0;
	pthread_mutex_lock(&st->mutex);
	if(st->rfp==NULL){
		st->rfp = fopen(URL_SDPATH,"r");
		if(st->rfp==NULL){
			perror("fopen read failed ");
			pthread_mutex_unlock(&st->mutex);
			return NULL;
		}
	}
#if 0
	get_mp3head(st->rfp,&st->rate,&st->channel);
#else
	get_mp3head(st->rfp,&st->rate,&st->channel);
	if(st->rfp != 44100){
		get_mp3head(st->rfp,&rate_one,&st->channel);
		get_mp3head(st->rfp,&rate_two,&st->channel);
		st->rate=rate_one>rate_two?rate_one:rate_two;
	}
#endif
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
	st->SetI2SRate(st->rate);
	DecodePlayMusic(InputNetStream);

#ifdef PALY_URL_SD
	if(st->cacheSize==st->streamLen){	//下载结束
		SaveLoveMp3File((const char *)URL_SDPATH);
	}
#endif
	st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//发送结束状态
	cleanStreamData(st);	//状态切换是否加锁
	
	pause_record_audio();
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
	if(st->player.playState==MAD_NEXT&&st->cacheSize>CACHE_PLAY_SIZE&&getDownState()==DOWN_ING)
	{
		st->player.playState=MAD_PLAY;
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

//退出当前流下载和播放
void NetStreamExitFile(void){
	if(getDownState()==DOWN_ING){		//退出下载
		quitDownFile();
		WriteEventlockLog("eventlock quitDownFile \n",2);
	}
	WriteEventlockLog("rate \n",st->rate);
	int error_timeout_check=0;
	while(st->player.playState==MAD_PLAY||st->player.playState==MAD_PAUSE){	//退出播放
		pthread_mutex_lock(&st->mutex);
		st->player.progress=0;
		st->player.musicTime=0;
		st->player.proflag=0;
		memset(st->player.musicname,0,64);
		DecodeExit();
		pthread_mutex_unlock(&st->mutex);
		WriteEventlockLog("eventlock wait exit mp3 state \n",(int)st->player.playState);
		DEBUG_STREAM(" while wait exit ...\n");
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
			WriteEventlockLog("error exit ,and set  \n",(int)st->player.playState);
			st->player.playState=MAD_NEXT;
			break;
		}
		if(++error_timeout_check>300000){
			WriteEventlockLog("error timeout_check ,and set exit \n",(int)st->player.playState);
			st->player.playState=MAD_NEXT;
			break;
		}
		usleep(100);
	}
	DEBUG_STREAM(" end ...\n");
	WriteEventlockLog("eventlock exit end\n",4);
}

//拷贝推送过来的信息
static void CopyUrlMessage(Player_t *srcPlayer,Player_t *DestPlayer){
	snprintf(DestPlayer->playfilename,128,"%s",srcPlayer->playfilename);	
	snprintf(DestPlayer->musicname,64,"%s",srcPlayer->musicname);
	DestPlayer->musicTime = srcPlayer->musicTime;
}
//开始边下边播放 
static int NetStreamDownFilePlay(Player_t *play){
	int ret=0;
	setDowning();
	st->player.progress=0;
	st->streamLen=0;
	st->playSize=0;
	st->cacheSize=0;
	WritePlayUrl_Log("play url:\n");
	WritePlayUrl_Log(play->playfilename);
	WritePlayUrl_Log("\n");
	st->player.playState=MAD_NEXT;
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);

	demoDownFile(play->playfilename,15,NetStartDown,NetGetStreamData,NetEndDown);
	DEBUG_STREAM("NetStreamDownFilePlay end ...\n");
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
	if(st->player.playState==MAD_PAUSE)
	{
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
}
//切换单曲和列表播放
void GpioKey_SetStreamPlayState(void){
	if(st->player.playListState==MUSIC_PLAY_LIST){
		st->player.playListState=MUSIC_SINGLE_LIST;
	}else{
		st->player.playListState=MUSIC_PLAY_LIST;
	}
}
//获取当前流播放状态
int GetStreamPlayState(void){
	return (int)st->player.playListState; 
}
//按键切换播放状态
void keyStreamPlay(void){
	if(st->player.playState==MAD_PAUSE){
		st->player.playState=MAD_PLAY;
		DecodeStart();
		PlayorPause();
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	}else if(st->player.playState==MAD_PLAY){
		st->player.playState=MAD_PAUSE;
		DecodePause();
		PlayorPause();
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	}else if(st->player.playState==MAD_EXIT){
#ifdef LOCAL_MP3
#if defined(QITUTU_SHI)
		GetSdcardMusicNameforPlay(xiai,XIMALA_MUSIC_DIRNAME,PLAY_NEXT);//暂停状态，添加喜爱歌曲播放
#elif defined(HUASHANG_JIAOYU) 
		GetScard_forPlayHuashang_Music((const void *)HUASHANG_GUOXUE_DIR,PLAY_NEXT,EXTERN_PLAY_EVENT);//暂停状态，添加华上教育歌曲播放
#endif
		usleep(1000);//防止添加按键太快	
#endif		
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
#ifdef SEEK_TO
	if(st->streamLen != 0){
#ifdef SHOW_progressBar		
		float getPercent=0;
		progressBar(st->playSize,st->streamLen,&getPercent);
#endif	
#if 0
		st->player.progress = (st->playSize*100)/st->streamLen;
		st->ack_playCtr(UDP_ACK,&st->player,st->player.playState);	//回应播放进度
#else
		ack_progress();
#endif
	}
	else{
		DEBUG_STREAM(" InputNetStream : streamLen == %d \n ");
	}
#endif
	fread(inputMsg,1,inputSize,st->rfp);
	st->playSize+=inputSize;
}
//播放本地音频流 
static void playLocalMp3(const char *mp3file){
	st->player.progress=0;
	st->streamLen=0;
	st->playSize=0;
	unsigned short rate_one=0,rate_two=0;
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
#if 0
	get_mp3head(st->rfp,&st->rate,&st->channel);
#else
	get_mp3head(st->rfp,&st->rate,&st->channel);
	if(st->rfp != 44100){
		get_mp3head(st->rfp,&rate_one,&st->channel);
		get_mp3head(st->rfp,&rate_two,&st->channel);
		st->rate=rate_one>rate_two?rate_one:rate_two;
	}
#endif
	fseek(st->rfp,0,SEEK_END);
	st->streamLen = ftell(st->rfp);
	fseek(st->rfp,0,SEEK_SET);

	printf("=============fseek=============\n"); //---bug,不能删

	pthread_mutex_unlock(&st->mutex);
	if(st->rate==0||st->rate==8000)
		st->rate=44100;
	
	DEBUG_STREAM("music st->rate =%d st->channel=%d \n",st->rate,st->channel);
	st->SetI2SRate(st->rate);
	SetDecodeSize(st->streamLen);
	DEBUG_STREAM("music start play \n");
	DecodePlayMusic(InputlocalStream);
	st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//发送结束状态
#ifdef PALY_URL_SD
	SaveLoveMp3File(mp3file);		//删除喜爱歌曲
#endif
	cleanStreamData(st);	//状态切换是否加锁
	DEBUG_STREAM(" exit play ok \n");
	WriteEventlockLog("playLocalMp3  exit play ok \n",(int)st->player.playState);
}
void Mad_PlayMusic(Player_t *play){
	char domain[64] = {0};
	char filename[128]={0};
	int port = 80;
	if(!access(play->playfilename,F_OK)){
		playLocalMp3(play->playfilename);
	}else{
		parse_url(play->playfilename, domain, &port, filename);
		CopyUrlMessage(play,(Player_t *)&st->player);
		char likebuf[256]={0};
		snprintf(likebuf,256,"%s%s",MP3_LIKEPATH,filename);	
		snprintf(st->mp3name,128,"%s",filename);			
		if(!access(likebuf,F_OK)){
			DEBUG_STREAM("find music in sdcard:%s \n",filename);
			playLocalMp3(likebuf);
		}else{
			DEBUG_STREAM("network play music : %s \n",filename);
			NetStreamDownFilePlay(play);
		}
	}
}
#ifdef PALY_URL_SD
//cacheFilename :微信端下载缓存的路径  /Down/xxxxxxxxxx.mp3
void HandlerWeixinDownMp3(const char *cacheFilename){
	char rumCmd[200]={0};
	char *filename= STRSTR(cacheFilename,'/');	//获取后缀文件名
	if(!strcmp(filename,"")){
		goto exit1;
	}
	WritePlayUrl_Log(cacheFilename);
	WritePlayUrl_Log(filename);
	keydown_flashingLED();	
#if defined(QITUTU_SHI)	
	if(InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)filename)==0){	//插入数据库成功
		snprintf(rumCmd,200,"cp %s %s%s",cacheFilename,MP3_LIKEPATH,filename);
		system(rumCmd);
	}
#endif	
exit1:	
	remove(cacheFilename);
}
#endif
//获取播放流状态
void getStreamState(void *data,void StreamState(void *data,Player_t *player)){
	st->player.vol = st->GetVol();
	StreamState(data,&st->player);
}
void initStream(void ack_playCtr(int nettype,Player_t *player,unsigned char playState),void WritePcmData(char *data,int size),void SetI2SRate(int rate),int GetVol(void)){
	st = calloc(1,MP3STEAM_SIZE);
	if(st==NULL){
		perror("calloc st memory failed");
		return ;
	}
	st->ack_playCtr=ack_playCtr;
	st->SetI2SRate=SetI2SRate;
	st->GetVol=GetVol;
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
