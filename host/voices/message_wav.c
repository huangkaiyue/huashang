#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
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
static void PlaySignleWavVoices(const char *playfilename,unsigned char playMode,unsigned int playEventNums){
	int r_size=0,pos=0;
	char readbuf[2]={0};
	FILE *fp= fopen(playfilename,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return ;
	}
	SetWm8960Rate(RECODE_RATE);
	fseek(fp,WAV_HEAD,SEEK_SET);		//����wavͷ��	
	while(1){
		if(playMode==PLAY_IS_INTERRUPT&&playEventNums!=GetCurrentEventNums()){
			pause_record_audio();
			CleanI2S_PlayCachedata();//����
			StopplayI2s();			 //���һƬ���ݶ���
			break;
		}	
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

	}
	fclose(fp);
	memset(play_buf,0,I2S_PAGE_SIZE);
	return ;
exit:
	pause_record_audio();
}
//���ŵ�����amr��ʽ��Ƶ����
static void playAmrVoices(const char *filename,unsigned char playMode,unsigned int playEventNums){
	char *outfile ="speek.wav";
	AmrToWav8k(filename,(const char *)outfile);
	PlaySignleWavVoices((const char *)outfile,playMode,playEventNums);
	remove(outfile);
}
static void __playAmrVoices(const char *filePath,unsigned char playMode,unsigned int playEventNums){
	char path[128]={0};
	snprintf(path,128,"%s%s",sysMes.localVoicesPath,filePath);
	playAmrVoices(path,playMode,playEventNums);
}


/********************************************************
@ ���Ž��յ��ֻ����͵ĶԽ���Ϣ
@ filename:���浽���ص�wav���ݵ��ļ�·�� (��������Ҫɾ��)
@
*********************************************************/
void playspeekVoices(const char *filename,unsigned int playEventNums){
	playAmrVoices(filename,PLAY_IS_INTERRUPT,playEventNums);
	remove(filename);
}
/********************************************************
@ ��������:	����ϵͳ��
@ filePath:	·��
@ ����ֵ: ��
*********************************************************/
void PlaySystemAmrVoices(const char *filePath,unsigned int playEventNums){
	__playAmrVoices(filePath,PLAY_IS_INTERRUPT,playEventNums);
}
//���Ź���������������
void play_waitVoices(const char *filePath,unsigned int playEventNums){
	__playAmrVoices(filePath,PLAY_IS_COMPLETE,playEventNums);
}

/********************************************************
@ ��������:	����QTTS����
@ text:�ı�		type:�ı�����
@ ����ֵ: ��
*********************************************************/
int PlayQttsText(const char *text,unsigned char type,const char *playVoicesName,unsigned int playEventNums){
	int ret =-1;
	SetWm8960Rate(RECODE_RATE);
	char *textbuf= (char *)calloc(1,strlen(text)+2);
	if(textbuf==NULL){
		perror("calloc error !!!");
		return ret;
	}
	sprintf(textbuf,"%s%s",text,",");	//�ı�β�����",",��֤�ı���������
	PlayQtts_log("play qtts start\n");
	ret =Qtts_voices_text(textbuf,type,playVoicesName,playEventNums);
	free(textbuf);
	return ret;

}
/********************************************************
@ ��������:	����ͼ������
@ url:ͼ�鷢��
@ ����ֵ: 0 �����˳� -1�������˳�
*********************************************************/
int PlayTulingText(HandlerText_t *handtext){
	return downTulingMp3_forPlay(handtext);
}	
