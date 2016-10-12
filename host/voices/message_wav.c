#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
#include "config.h"

void playLocalVoices(char *path)
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
}

void play_sys_tices_voices(char *filePath)
{
	char path[128];
	snprintf(path,128,"%s%s",sysMes.sd_path,filePath);
	i2s_start_play(8000);
#ifdef CLOSE_VOICE
	open_wm8960_voices();
	//SET_MUTE_ENABLE();
#ifdef MUTE_TX
	mute_recorde_vol(UNMUTE);
#endif
#endif
	if(strstr(path,"TuLin_Wint_8K")||strstr(path,"TuLin_Di_8K")){
		mute_recorde_vol(100);
	}
	playLocalVoices(path);
	memset(play_buf,0,I2S_PAGE_SIZE);
	write_pcm(play_buf);
#ifdef CLOSE_VOICE
	if(!strcmp(filePath,"qtts/yes_reavwifi_8K.wav")){
		return;
	}
	sleep(2);
#ifdef MUTE_TX
	mute_recorde_vol(MUTE);
#endif
	close_wm8960_voices();
	//SET_MUTE_DISABLE();
#endif
}
/********************************************************
@ 播放接收到手机发送的对讲消息
@ filePath:缓存到本地的wav数据的文件路径 	
@
*********************************************************/
void playRecvVoices(char *filePath)
{
	play_sys_tices_voices(MSG_VOICES);//播放提示音
	playLocalVoices(filePath);//播放接收到语音
	sleep(2);//睡眠2s等待播放完
	remove(filePath);//删除缓存语音
}
void PlayQttsText(char *text,unsigned char type)
{
//音频开关
	i2s_start_play(8000);
#ifdef CLOSE_VOICE
	open_wm8960_voices();
	//SET_MUTE_ENABLE();
#ifdef MUTE_TX
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
#ifdef CLOSE_VOICE
	sleep(2);
#ifdef MUTE_TX
	mute_recorde_vol(MUTE);
#endif
	close_wm8960_voices();
	//SET_MUTE_DISABLE();
#endif
	free(textbuf);
}
#ifdef SPEEK_VOICES
void playspeekVoices(const char *filename){
	i2s_start_play(8000);
#ifdef CLOSE_VOICE
	open_wm8960_voices();
		//SET_MUTE_ENABLE();
#ifdef MUTE_TX
	mute_recorde_vol(UNMUTE);
#endif
#endif
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
	fseek(fp,WAV_HEAD,SEEK_SET);//跳过wav头部	

	while(1){
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			if(pos>0){
				memset(play_buf+pos,0,I2S_PAGE_SIZE-pos);//清空上一次尾部杂音
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
#ifdef CLOSE_VOICE
	sleep(2);
#ifdef MUTE_TX
	mute_recorde_vol(MUTE);
#endif
	close_wm8960_voices();
	//SET_MUTE_DISABLE();
#endif
}
#endif
