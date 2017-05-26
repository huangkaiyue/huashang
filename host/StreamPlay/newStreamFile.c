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
	fseek(st->rfp,st->playSize,SEEK_SET);	//��ת��ָ����λ�ò���
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
//ʵ��д����Ƶ���Ľӿ�, ��Ҫ����������ڴ���λ�� inputMsg  inputSize �������������С
static void InputNetStream(const void * inputMsg,int inputSize){
	while(st->playSize+inputSize>st->cacheSize){
		if(getDownState()==DOWN_QUIT){	//�Ѿ��˳����أ�ֹͣ����	
			DEBUG_STREAM("exit ...st->cacheSize =%d st->playSize%d\n",st->cacheSize,st->playSize);
			if(st->cacheSize > st->playSize){
				if(st->streamLen != 0){
					st->player.progress = (st->playSize*100)/st->streamLen;
					st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);	//��Ӧ���Ž���
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
		usleep(100);//���رȽ�����˯�ߵȴ����� ,����ֹͣ״̬,���ص�һ���̶ȣ�����  (app�ȵ���һ��ʱ�����Զ�����)
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

#define LOVE_MP3_UNKOWN_EVENT				0		//δ֪״̬
#define LOVE_MP3_SAVE_LOVE_MP3_EVENT		1		//���ϲ��
#define LOVE_MP3_DELETE_EVENT				2		//ɾ��ϲ��
#define DEFAULT_SAVE_MP3_EVENT				3		//Ĭ�ϱ���mp3�ļ�

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
		case LOVE_MP3_SAVE_LOVE_MP3_EVENT:		//���ϲ��
			if(CheckSdcardInfo(MP3_SDPATH)){	//�ڴ治��50Mɾ��ָ����Ŀ�ĸ���
				printf("%s: delete memory \n",__func__);
				break;
			}	
			if(strcmp(st->mp3name,"")){
				//�������ݿ�ɹ�
				if(InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)st->mp3name)==0){
					snprintf(runCmd,200,"cp %s %s%s",filepath,MP3_LIKEPATH,st->mp3name);
					system(runCmd);
				}
				
			}	
			like_mp3_sign=LOVE_MP3_UNKOWN_EVENT;	
			break;
		case LOVE_MP3_DELETE_EVENT:		//ɾ��ϲ��
			like_mp3_sign=LOVE_MP3_UNKOWN_EVENT;
			if(!strcmp(st->mp3name,"")){	//���ڿ�
				int size=GetFileNameForPath(filepath);
				memcpy(st->mp3name,filepath+size,strlen(filepath));
			}
			printf("filepath: %s \t mp3name: %s \n",filepath,st->mp3name);
			if(DelXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)st->mp3name)==0)
				remove(filepath);
			break;
		case DEFAULT_SAVE_MP3_EVENT:	//Ĭ�ϱ��浽sdcard ����
			//snprintf(buf,200,"cp %s %s%s",URL_SDPATH,MP3_SDPATH,st->mp3name);
			//system(buf);
			break;
		default:	
			break;
	}
#endif	
}

//ϲ��
void Save_like_music(void){
	like_mp3_sign=LOVE_MP3_SAVE_LOVE_MP3_EVENT;
}

void Del_like_music(void){
	like_mp3_sign=LOVE_MP3_DELETE_EVENT;
}
#endif
//������������Ƶ�ļ�
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
	if(st->cacheSize==st->streamLen){	//���ؽ���
		SaveLoveMp3File((const char *)URL_SDPATH);
	}
#endif
	st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//���ͽ���״̬
	cleanStreamData(st);	//״̬�л��Ƿ����
	
	pause_record_audio();
	DEBUG_STREAM("exit play ok \n");
	return NULL;
}

//��ʼ����, �ӿڼ��ݣ���Ҫȥ��streamLen
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
//��ȡ��������
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
	//���л��浽8*KB���ݣ���ʼ����
	if(st->player.playState==MAD_NEXT&&st->cacheSize>CACHE_PLAY_SIZE&&getDownState()==DOWN_ING)
	{
		st->player.playState=MAD_PLAY;
		pool_add_task(NetplayStreamMusic,NULL);
	}
}
//��������
static void NetEndDown(int downLen){
	DEBUG_STREAM("OK \n");
	//st->cacheSize=downLen;
	SetDecodeSize(downLen);
	if(st->wfp)
		fclose(st->wfp);
	st->wfp=NULL;
}

//�˳���ǰ�����غͲ���
void NetStreamExitFile(void){
	if(getDownState()==DOWN_ING){		//�˳�����
		quitDownFile();
		WriteEventlockLog("eventlock quitDownFile \n",2);
	}
	WriteEventlockLog("rate \n",st->rate);
	int error_timeout_check=0;
	while(st->player.playState==MAD_PLAY||st->player.playState==MAD_PAUSE){	//�˳�����
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

//�������͹�������Ϣ
static void CopyUrlMessage(Player_t *srcPlayer,Player_t *DestPlayer){
	snprintf(DestPlayer->playfilename,128,"%s",srcPlayer->playfilename);	
	snprintf(DestPlayer->musicname,64,"%s",srcPlayer->musicname);
	DestPlayer->musicTime = srcPlayer->musicTime;
}
//��ʼ���±߲��� 
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
//��ͣ
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
//����
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
//���õ�ǰ������״̬
void SetStreamPlayState(unsigned char playliststate){
	st->player.playListState=playliststate;
}
//�л��������б���
void GpioKey_SetStreamPlayState(void){
	if(st->player.playListState==MUSIC_PLAY_LIST){
		st->player.playListState=MUSIC_SINGLE_LIST;
	}else{
		st->player.playListState=MUSIC_PLAY_LIST;
	}
}
//��ȡ��ǰ������״̬
int GetStreamPlayState(void){
	return (int)st->player.playListState; 
}
//�����л�����״̬
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
		GetSdcardMusicNameforPlay(xiai,XIMALA_MUSIC_DIRNAME,PLAY_NEXT);//��ͣ״̬�����ϲ����������
#elif defined(HUASHANG_JIAOYU) 
		GetScard_forPlayHuashang_Music((const void *)HUASHANG_GUOXUE_DIR,PLAY_NEXT,EXTERN_PLAY_EVENT);//��ͣ״̬����ӻ��Ͻ�����������
#endif
		usleep(1000);//��ֹ��Ӱ���̫��	
#endif		
	}
}
//���������Ʋ��� 
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
		st->playSize=st->streamLen*progress/100;//�������ǰ���ų���	
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
		st->ack_playCtr(UDP_ACK,&st->player,st->player.playState);	//��Ӧ���Ž���
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
//���ű�����Ƶ�� 
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
	printf("============playLocalMp3==============\n"); //---bug,����ɾ�������쳣
	pthread_mutex_lock(&st->mutex);
	st->player.playState =MAD_PLAY;
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	
	printf("=============ack_playCtr=============\n"); //---bug,����ɾ
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

	printf("=============fseek=============\n"); //---bug,����ɾ

	pthread_mutex_unlock(&st->mutex);
	if(st->rate==0||st->rate==8000)
		st->rate=44100;
	
	DEBUG_STREAM("music st->rate =%d st->channel=%d \n",st->rate,st->channel);
	st->SetI2SRate(st->rate);
	SetDecodeSize(st->streamLen);
	DEBUG_STREAM("music start play \n");
	DecodePlayMusic(InputlocalStream);
	st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//���ͽ���״̬
#ifdef PALY_URL_SD
	SaveLoveMp3File(mp3file);		//ɾ��ϲ������
#endif
	cleanStreamData(st);	//״̬�л��Ƿ����
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
//cacheFilename :΢�Ŷ����ػ����·��  /Down/xxxxxxxxxx.mp3
void HandlerWeixinDownMp3(const char *cacheFilename){
	char rumCmd[200]={0};
	char *filename= STRSTR(cacheFilename,'/');	//��ȡ��׺�ļ���
	if(!strcmp(filename,"")){
		goto exit1;
	}
	WritePlayUrl_Log(cacheFilename);
	WritePlayUrl_Log(filename);
	keydown_flashingLED();	
#if defined(QITUTU_SHI)	
	if(InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)filename)==0){	//�������ݿ�ɹ�
		snprintf(rumCmd,200,"cp %s %s%s",cacheFilename,MP3_LIKEPATH,filename);
		system(rumCmd);
	}
#endif	
exit1:	
	remove(cacheFilename);
}
#endif
//��ȡ������״̬
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
