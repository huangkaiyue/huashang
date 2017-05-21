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
	while(1){
		if(playMode==PLAY_IS_INTERRUPT&&playEventNums!=GetCurrentEventNums()){
			CleanI2S_PlayCachedata();//����
			StopplayI2s();			 //���һƬ���ݶ���
			ret=-1;
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
	return ret;
}
int __playResamplePlayPcmFile(const char *pcmFile,unsigned int playEventNums){
	char * resRateFile ="resRate.pcm";
	if(ResamplerState_wavfileVoices((const char * )pcmFile,(const char * )resRateFile,44100)){
		return -1;
	}
	StreamPause();
	start_event_play_soundMix();//�л�����������״̬		
	int ret =PlaySignleWavVoices((const char *)resRateFile,PLAY_IS_INTERRUPT,playEventNums);
	start_event_play_Mp3music();
	keyStreamPlay();
	remove(resRateFile);
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
	SetWm8960Rate(RECODE_RATE);
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
	SetWm8960Rate(RECODE_RATE);
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
