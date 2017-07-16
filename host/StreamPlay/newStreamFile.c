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

static void GetMusicMessage(int rate,int channels){
	st->rate = rate;
	st->SetI2SRate(rate,"NetplayStreamMusic set rate");
}
//ʵ��д����Ƶ���Ľӿ�, ��Ҫ����������ڴ���λ�� inputMsg  inputSize �������������С
static void InputNetStream(const void * inputMsg,int inputSize){
	printf("InputNetStream inputSize=%d\n",inputSize);
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

//������������Ƶ�ļ�
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

	st->ack_playCtr(TCP_ACK,&st->player,MAD_EXIT);	//���ͽ���״̬
	cleanStreamData(st);	//״̬�л��Ƿ����
	
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
	if(st->player.playState==MAD_NEXT&&st->cacheSize>CACHE_PLAY_SIZE&&getDownState()==DOWN_ING){
		st->player.playState=MAD_PLAY;
		DecodeStart();
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
		WriteEventlockLog("eventlock quitDownFile",2);
	}
	printf("%s: rate =%d\n",__func__,st->rate);
	int error_timeout_check=0;
	while(st->player.playState==MAD_PLAY||st->player.playState==MAD_PAUSE){	//�˳�����
		pthread_mutex_lock(&st->mutex);
		st->player.progress=0;
		st->player.musicTime=0;
		st->player.proflag=0;
		memset(st->player.musicname,0,64);
		DecodeExit();
		pthread_mutex_unlock(&st->mutex);
		WriteEventlockLog("eventlock wait exit mp3 state",(int)st->player.playState);
		printf("%s: while wait exit ...\n",__func__);
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
			WriteEventlockLog("error exit ,and set  ",(int)st->player.playState);
			printf("%s: error RECODE_PAUSE exit ,and set ... st->player.playState=%d\n",__func__,st->player.playState);
			st->player.playState=MAD_NEXT;
			break;
		}
		if(++error_timeout_check>300000){
			WriteEventlockLog("error timeout_check ,and set exit",(int)st->player.playState);
			printf("%s: error timeout_check exit ,and set ... st->player.playState=%d\n",__func__,st->player.playState);
			st->player.playState=MAD_NEXT;
			break;
		}
		usleep(100);
	}
	printf("%s: paly end ... st->player.playState=%d\n",__func__,st->player.playState);
	WriteEventlockLog("eventlock exit end",st->player.playState);
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
	WritePlayUrl_Log("play url",play->playfilename);
	st->player.playState=MAD_NEXT;
	st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	demoDownFile(play->playfilename,15,NetStartDown,NetGetStreamData,NetEndDown);
	while(st->player.playState!=MAD_EXIT){
		printf("wait exit play state: %d \n",GetRecordeVoices_PthreadState());
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
	}
	pause_record_audio();
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
	if(st->player.playState==MAD_PAUSE){
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
		Show_musicPicture();
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	}else if(st->player.playState==MAD_PLAY){
		st->player.playState=MAD_PAUSE;
		DecodePause();
		PlayorPause();
		showFacePicture(WAIT_CTRL_NUM4);
		st->ack_playCtr(TCP_ACK,&st->player,st->player.playState);
	}else if(st->player.playState==MAD_EXIT){
		GetScard_forPlayHuashang_Music(PLAY_NEXT,EXTERN_PLAY_EVENT);//��ͣ״̬����ӻ��Ͻ�����������
		usleep(1000);//��ֹ��Ӱ���̫��	
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
	if(st->GetWm8960Rate==8000){
		DecodeExit();
	}
	fread(inputMsg,1,inputSize,st->rfp);
	st->playSize+=inputSize;
}
//���ű�����Ƶ�� 
static void playLocalMp3(const char *mp3file){
	st->player.progress=0;
	st->streamLen=0;
	st->playSize=0;
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

	fseek(st->rfp,0,SEEK_END);
	st->streamLen = ftell(st->rfp);
	fseek(st->rfp,0,SEEK_SET);

	printf("=============fseek=============\n"); //---bug,����ɾ

	pthread_mutex_unlock(&st->mutex);

	SetDecodeSize(st->streamLen);
	DecodeStart();
	DEBUG_STREAM("music start play \n");
	DecodePlayMusic(GetMusicMessage,InputlocalStream);
	cleanStreamData(st);	//״̬�л��Ƿ����
	DEBUG_STREAM(" exit play ok \n");
	WriteEventlockLog("playLocalMp3  exit play ok ",(int)st->player.playState);
}
//���Ÿ����ӿ�  play: ������Ϣ�ṹ��
int Mad_PlayMusic(Player_t *play){
	int ret =1;
	start_event_play_Mp3music();
	char domain[64] = {0};
	char filename[128]={0};
	int port = 80;
	printf("%s: play->playfilename =%s\n",__func__,play->playfilename);
	if(!access(play->playfilename,F_OK)){
		playLocalMp3(play->playfilename);
	}else{
		if(strstr(play->playfilename,"/media/mmcblk0p1/")){
			goto exit0;
		}
		parse_url(play->playfilename, domain, &port, filename);
		CopyUrlMessage(play,(Player_t *)&st->player);
		snprintf(st->mp3name,128,"%s",filename);			
		DEBUG_STREAM("network play music : %s \n",filename);
		NetStreamDownFilePlay(play);
	}
	ret =0;
exit0:	
	return ret;
}

//��ȡ������״̬
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
