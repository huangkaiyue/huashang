#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
#include "host/voices/resexampleRate.h"
#include "host/studyvoices/std_worklist.h"
#include "config.h"
#include "log.h"

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
static int PlayStartWavVoices(const char *playfilename){
	int r_size=0,pos=0;
	char readbuf[2]={0};
	int ret=0;
	FILE *fp= fopen(playfilename,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return -1;
	}
	fseek(fp,WAV_HEAD,SEEK_SET);		//����wavͷ��	
	FILE *file = fopen("out2.pcm","w+");
	if(file==NULL){
		printf("open sys failed \n");
		return -1;
	}
	while(1){
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			break;
		}
		fwrite(readbuf,2,1,file);
		fwrite(readbuf,2,1,file);
	}
	fclose(fp);
	fclose(file);
	file = fopen("out2.pcm","r");
	if(file==NULL){
		printf("open sys failed \n");
		return -1;
	}
	while(1){
		r_size= fread(play_buf,1,I2S_PAGE_SIZE,file);
		if(r_size==0){
			break;
		}
		if(r_size!=I2S_PAGE_SIZE){
			memset(play_buf+r_size,0,I2S_PAGE_SIZE-r_size);
		}
		write_pcm(play_buf);
	}
	memset(play_buf,0,I2S_PAGE_SIZE);	
	remove("out2.pcm");
	return ret;
}
void PlayStartPcm(const char *filename){
	char *outfile ="speek.wav";
	char path[128]={0};
	snprintf(path,128,"%s%s",sysMes.localVoicesPath,filename);
	AmrToWav8k(path,(const char *)outfile);
	PlayStartWavVoices(outfile);

}
//���ŵ�����wav��ʽ��Ƶ����
static int PlaySignleWavVoices(const char *playfilename,unsigned char playMode,unsigned int playEventNums){
	int r_size=0,pos=0;
	char readbuf[2]={0};
	int ret=0;
	FILE *fp= fopen(playfilename,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return -1;
	}
	fseek(fp,WAV_HEAD,SEEK_SET);		//����wavͷ��	
	Show_tlak_Light();
	while(1){
		if(playMode==PLAY_IS_INTERRUPT&&playEventNums!=GetCurrentEventNums()){
			CleanI2S_PlayCachedata();//����
			StopplayI2s();			 //���һƬ���ݶ���
			ret=-1;
			Mute_voices(MUTE);
			break;
		}	
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			if(pos>0){
				memset(play_buf+pos,0,I2S_PAGE_SIZE-pos);		//�����һ��β������,������β��
				write_pcm(play_buf);
			}
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

	}
	fclose(fp);
	memset(play_buf,0,I2S_PAGE_SIZE);
	Close_tlak_Light();
	return ret;
}

int __playResamplePlayPcmFile(const char *pcmFile,unsigned int playEventNums){
	int currentRate = 0;
	StreamPause();
	currentRate = GetWm8960Rate();
	start_event_play_soundMix();//�л�����������״̬		
	SetWm8960Rate(RECODE_RATE,(const char *)"__playResamplePlayPcmFile set rate");
	int ret =PlaySignleWavVoices((const char *)pcmFile,PLAY_IS_INTERRUPT,playEventNums);	
	start_event_play_Mp3music();
	//��Ҫ�Ż�һ�£���鵽������Ƶ�ļ�
	SetWm8960Rate(currentRate,(const char *)"__playResamplePlayPcmFile set rate");
	keyStreamPlay();
	return ret;
}
static int playResamplePlayAmrFile(const char *filename,unsigned int playEventNums){
	char *outfile ="speek.wav";
	int  ret =AmrToWav8k(filename,(const char *)outfile);
	if(ret){
		printf("AmrToWav8k failed \n");
		return -1;
	}
	ret = __playResamplePlayPcmFile(outfile,playEventNums);
	remove(outfile);
	return ret ; 
}

//���ŵ�����amr��ʽ��Ƶ����
static int __playAmrVoices(const char *filename,unsigned char playMode,unsigned int playEventNums){
	char *outfile ="speek.wav";
	AmrToWav8k(filename,(const char *)outfile);
	SetWm8960Rate(RECODE_RATE,(const char *)"__playAmrVoices set rate");
	int ret = PlaySignleWavVoices((const char *)outfile,playMode,playEventNums);
	if(ret==0){	//���������꣬��¼���̹߳�����
		pause_record_audio();
	}
	remove(outfile);
	return ret;
}
static int __playSystemAmrVoices(const char *filePath,unsigned char playMode,unsigned int playEventNums){
	char path[128]={0};
	snprintf(path,128,"%s%s",sysMes.localVoicesPath,filePath);
	return __playAmrVoices(path,playMode,playEventNums);
}
/********************************************************
@ ���Ž��յ��ֻ����͵ĶԽ���Ϣ
@ filename:���浽���ص�wav���ݵ��ļ�·�� (��������Ҫɾ��)
@
*********************************************************/
int PlayWeixin_SpeekAmrFileVoices(const char *filename,unsigned int playEventNums,unsigned char mixMode){
	int ret =-1;
	if(mixMode==MIX_PLAY_PCM){
		ret =playResamplePlayAmrFile(filename,playEventNums);
	}else{
		start_event_play_wav();
		ret= __playAmrVoices(filename,PLAY_IS_INTERRUPT,playEventNums);
	}
	remove(filename);
	return ret;
}
/********************************************************
@ ��������:	����ϵͳ��
@ filePath:	·��
@ ����ֵ: ��
*********************************************************/
int PlaySystemAmrVoices(const char *filePath,unsigned int playEventNums){
	return __playSystemAmrVoices(filePath,PLAY_IS_INTERRUPT,playEventNums);
}
//���Ź���������������
void PlayImportVoices(const char *filePath,unsigned int playEventNums){
	__playSystemAmrVoices(filePath,PLAY_IS_COMPLETE,playEventNums);
}

/********************************************************
@ ��������:	����QTTS����
@ text:�ı�		type:�ı�����
@ ����ֵ: ��
*********************************************************/
int PlayQttsText(const char *text,unsigned char type,const char *playVoicesName,unsigned int playEventNums,int playSpeed){
	SetWm8960Rate(RECODE_RATE,(const char *)"PlayQttsText set rate");
	return Qtts_voices_text(text,type,playVoicesName,playEventNums,playSpeed);

}
/********************************************************
@ ��������:	����ͼ������
@ url:ͼ�鷢��
@ ����ֵ: 0 �����˳� -1�������˳�
*********************************************************/
int PlayTulingText(HandlerText_t *handtext){
	return downTulingMp3_forPlay(handtext);
}	
