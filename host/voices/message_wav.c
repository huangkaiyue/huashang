#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
#include "config.h"

//------------------------------------------------------
static unsigned short playNetwork_pos=0;
static unsigned char  playNetworkState=0;

void SetPlayNetworkState(unsigned char state){
	playNetworkState = state;
}
int safeSetPlayNetworkState(unsigned char state){
	if(playNetworkState==INTERRUPT_PLAY_WAV){
		return -1;	
	}
	playNetworkState=state;
	return 0;
}
int GetPlayNetworkState(void){
	return (int)playNetworkState;
}
void ExitPlayNetworkState(void){
	SetPlayNetworkState(INTERRUPT_PLAY_WAV);
	exit_tulingplay();
}
int WriteStreamPcmData(char *data,int len){
	int i=0;
	for(i=0;i<len;i+=2){
		if(playNetwork_pos==I2S_PAGE_SIZE){
			write_pcm(play_buf);
			playNetwork_pos=0;
		}
		if(playNetwork_pos+4>I2S_PAGE_SIZE){
			printf(".......................error write data ................\n");
			playNetwork_pos=0;
			continue;
		}
		memcpy(play_buf+playNetwork_pos,data+i,2);
		playNetwork_pos += 2;
		memcpy(play_buf+playNetwork_pos,data+i,2);
		playNetwork_pos += 2;
		if(playNetworkState==INTERRUPT_PLAY_WAV){
			return -1;
		}
	}
	return 0;
}
//------------------------------------------------------
static unsigned char playWavState=0;
void SetPlayWavState(unsigned char state){
	playWavState=state;
}
int SafeSetPlayWavState(unsigned char state){
	if(playWavState==INTERRUPT_PLAY_WAV){
		return -1;	
	}
	playWavState=state;
	return 0;
}
int GetPlayWavState(void){
	return (int)playWavState;
}
void ExitPlay_WavVoices(void){
	SetPlayWavState(INTERRUPT_PLAY_WAV);
}
//------------------------------------------------------
void WritePcmData(char *data,int size){
	if(I2S.play_size==I2S_PAGE_SIZE){
		I2S.play_size=0;
		write_pcm(play_buf);
	}
	memcpy(play_buf+I2S.play_size,data,size);
	I2S.play_size +=size;
}
//���ŵ�����wav��ʽ��Ƶ����
static void PlaySignleWavVoices(const char *playfilename,unsigned char playMode){
	int r_size=0,pos=0;
	char readbuf[2]={0};
	FILE *fp= fopen(playfilename,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return ;
	}
	SetWm8960Rate(RECODE_RATE);
	if(playMode==PLAY_IS_INTERRUPT&&SafeSetPlayWavState(START_PLAY_WAV)){
		printf("set play wav state failed ; current is interrupt play system wav voices \n");
		goto exit;
	}
	fseek(fp,WAV_HEAD,SEEK_SET);		//����wavͷ��	
	while(1){
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			if(pos>0){
				memset(play_buf+pos,0,I2S_PAGE_SIZE-pos);		//�����һ��β������,������β��
				write_pcm(play_buf);
			}
			pause_record_audio();
			CleanI2S_PlayCachedata();
			break;
		}
		memcpy(play_buf+pos,readbuf,2);
		pos+=2;
		memcpy(play_buf+pos,readbuf,2);
		pos+=2;
		if(pos==I2S_PAGE_SIZE){
			write_pcm(play_buf);
			pos=0;
		}
		if(GetPlayWavState()==INTERRUPT_PLAY_WAV&&playMode==PLAY_IS_INTERRUPT){
			pause_record_audio();
			CleanI2S_PlayCachedata();//����
			StopplayI2s();			 //���һƬ���ݶ���
			break;
		}	
	}
	fclose(fp);
	memset(play_buf,0,I2S_PAGE_SIZE);
	SetPlayWavState(START_PLAY_WAV);
	return ;
exit:
	pause_record_audio();
	SetPlayWavState(START_PLAY_WAV);
}
//���ŵ�����amr��ʽ��Ƶ����
static void playAmrVoices(const char *filename,unsigned char playMode){
	char *outfile ="speek.wav";
	AmrToWav8k(filename,(const char *)outfile);
	PlaySignleWavVoices((const char *)outfile,playMode);
	remove(outfile);
}
static void __playAmrVoices(const char *filePath,unsigned char playMode){
	char path[128]={0};
	snprintf(path,128,"%s%s",sysMes.localVoicesPath,filePath);
	playAmrVoices(path,playMode);
}

#ifdef SPEEK_VOICES
/********************************************************
@ ���Ž��յ��ֻ����͵ĶԽ���Ϣ
@ filename:���浽���ص�wav���ݵ��ļ�·�� (��������Ҫɾ��)
@
*********************************************************/
void playspeekVoices(const char *filename){
	playAmrVoices(filename,PLAY_IS_INTERRUPT);
	remove(filename);
}
#endif
/********************************************************
@ ��������:	����ϵͳ��
@ filePath:	·��
@ ����ֵ: ��
*********************************************************/
void PlaySystemAmrVoices(const char *filePath){
	__playAmrVoices(filePath,PLAY_IS_INTERRUPT);
}

void play_waitVoices(const char *filePath){
	__playAmrVoices(filePath,PLAY_IS_COMPLETE);
}
//��鲥����·������Ƶ�ļ�β��
static int checkPlayNetwork_endVoices(void){
	int ret=0;
	if(playWavState==INTERRUPT_PLAY_WAV){
		Mute_voices(MUTE);
		pause_record_audio();		//�˳�����״̬
		CleanI2S_PlayCachedata();	//����
		StopplayI2s();				//���һƬ���ݶ���
		memset(play_buf,0,I2S_PAGE_SIZE);
		ret=-1;
	}else{
		if(playNetwork_pos!=0){		//����β��
			memset(play_buf+playNetwork_pos,0,I2S_PAGE_SIZE-playNetwork_pos);
			write_pcm(play_buf);
		}
		memset(play_buf,0,I2S_PAGE_SIZE);
		pause_record_audio();		//�˳�����״̬
		CleanI2S_PlayCachedata();	//����
	}
	playNetwork_pos =0;	
	SetPlayNetworkState(START_PLAY_WAV);
	SetTuling_playunLock();
	return ret;
}
/********************************************************
@ ��������:	����QTTS����
@ text:�ı�		type:�ı�����
@ ����ֵ: ��
*********************************************************/
void PlayQttsText(char *text,unsigned char type){
	SetWm8960Rate(RECODE_RATE);
	char *textbuf= (char *)calloc(1,strlen(text)+2);
	if(textbuf==NULL){
		perror("calloc error !!!");
		goto exit;
	}
	if(safeSetPlayNetworkState(START_PLAY_WAV)){
		SetPlayNetworkState(START_PLAY_WAV);
		goto exit;
	}
	sprintf(textbuf,"%s%s",text,",");	//�ı�β�����",",��֤�ı���������
	PlayQtts_log("play qtts start\n");
	printf("start play qtts \n");
	Qtts_voices_text(textbuf,type);
	free(textbuf);
	checkPlayNetwork_endVoices();
	return ;
exit:
	SetTuling_playunLock();
	pause_record_audio();
}
/********************************************************
@ ��������:	����ͼ������
@ url:ͼ�鷢��
@ ����ֵ: 0 �����˳� -1�������˳�
*********************************************************/
int PlayTulingText(const char *url){
	if(safeSetPlayNetworkState(START_PLAY_WAV)){
		SetPlayNetworkState(START_PLAY_WAV);
		SetTuling_playunLock();
		return -1;
	}
	SetWm8960Rate(RECODE_RATE); 
	downTulingMp3((const char*)url);
	return checkPlayNetwork_endVoices();
}	
