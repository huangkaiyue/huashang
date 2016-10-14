#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
#include "config.h"

#ifndef SPEEK_VOICES
void playWavVoices(char *path)
{
	int size=0;
	FILE *fp= fopen(path,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return;
	}
#ifdef TULIN_WINT_MUSIC
	if(strstr(path,"TuLin_Wint_8K")){
		size=fread(play_buf,1,I2S_PAGE_SIZE,fp);
		memset(play_buf,0,size);
	}
#endif
	start_event_play_wav();
	while(1)
	{
		size= fread(play_buf,1,I2S_PAGE_SIZE,fp);
		if(size==0){
			write_pcm(play_buf);
			usleep(1000);
			break;
		}
		write_pcm(play_buf);
		memset(play_buf,0,size);
	}
	fclose(fp);
	memset(play_buf,0,I2S_PAGE_SIZE);
	write_pcm(play_buf);
}
#else
void playAmrVoices(const char *filename)
{
	char *outfile ="speek.wav";
	AmrToWav8k(filename,(const char *)outfile);
	int r_size=0,pos=0;
	char readbuf[2];
	FILE *fp= fopen(outfile,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return;
	}
	start_event_play_wav();
	fseek(fp,WAV_HEAD,SEEK_SET);//����wavͷ��	

#ifdef TULIN_WINT_MUSIC
	if(strstr(filename,"TuLin_Wint_8K")){
		r_size=fread(play_buf,1,I2S_PAGE_SIZE/2,fp);
		memset(play_buf,0,r_size);
	}
#endif
	while(1){
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			if(pos>0){
				memset(play_buf+pos,0,I2S_PAGE_SIZE-pos);//�����һ��β������
				write_pcm(play_buf);
				usleep(1000);
			}
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
	write_pcm(play_buf);
	remove(outfile);
}

#endif
#ifdef SPEEK_VOICES
void playspeekVoices(const char *filename){
#if 1
	i2s_start_play(8000);
#else
#ifdef CLOSE_VOICE
	open_wm8960_voices();
	//SET_MUTE_ENABLE();
	mute_recorde_vol(UNMUTE);
#endif
#endif
	playAmrVoices(filename);
#ifdef CLOSE_VOICE
	sleep(2);
	mute_recorde_vol(MUTE);
	close_wm8960_voices();
	//SET_MUTE_DISABLE();
#endif	//end CLOSE_VOICE
}
#endif
void play_sys_tices_voices(char *filePath)
{
	char path[128];
	snprintf(path,128,"%s%s",sysMes.sd_path,filePath);
	//i2s_start_play(8000);
#ifdef CLOSE_VOICE
	open_wm8960_voices();
	//SET_MUTE_ENABLE();
	if(strstr(path,"TuLin_Wint_8K")||strstr(path,"TuLin_Di_8K")){
		mute_recorde_vol(100);
	}else{
		mute_recorde_vol(UNMUTE);
	}
#endif
#ifdef SPEEK_VOICES
	playAmrVoices(path);
#else
	playWavVoices(path);
#endif
#ifdef CLOSE_VOICE
	if(!strcmp(filePath,"qtts/yes_reavwifi_8K")){
		return;
	}
	if(strstr(filePath,"40002_8k")){
		pause_record_audio();	//�˳�����
	}
	usleep(1800*1000);
	mute_recorde_vol(MUTE);
	close_wm8960_voices();
	//SET_MUTE_DISABLE();
#endif
}
#if 0
/********************************************************
@ ���Ž��յ��ֻ����͵ĶԽ���Ϣ
@ filePath:���浽���ص�wav���ݵ��ļ�·�� 	
@
*********************************************************/
void playRecvVoices(char *filePath)
{
	play_sys_tices_voices(MSG_VOICES);//������ʾ��
	playLocalVoices(filePath);//���Ž��յ�����
	sleep(2);//˯��2s�ȴ�������
	remove(filePath);//ɾ����������
}
#endif
void PlayQttsText(char *text,unsigned char type)
{
#if 1
	i2s_start_play(8000);
#else
#ifdef CLOSE_VOICE
	open_wm8960_voices();
	//SET_MUTE_ENABLE();	//��Ƶ����
	mute_recorde_vol(UNMUTE);
#endif
#endif
	char *textbuf= (char *)calloc(1,strlen(text)+2);
	sprintf(textbuf,"%s%s",text,",");
	Qtts_voices_text(textbuf,type);
	if(I2S.qttspos!=0)
	{
		memset(play_buf+I2S.qttspos,0,I2S_PAGE_SIZE-I2S.qttspos);
		write_pcm(play_buf);
		I2S.qttspos =0;
	}
	memset(play_buf,0,I2S_PAGE_SIZE);
	write_pcm(play_buf);
	usleep(800*1000);
	pause_record_audio();	//�˳�����
#ifdef CLOSE_VOICE
	usleep(1000*1000);
	mute_recorde_vol(MUTE);
	close_wm8960_voices();
	//SET_MUTE_DISABLE();
#endif
	free(textbuf);
}
