#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
#include "config.h"

#if 0
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
	fseek(fp,WAV_HEAD,SEEK_SET);//跳过wav头部	

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
	clean_play_cache();
	//memset(play_buf,0,I2S_PAGE_SIZE);
	//write_pcm(play_buf);
	remove(outfile);
}

#endif
#ifdef SPEEK_VOICES
void Mute_voices(unsigned char stat){
	switch(stat){
		case MUTE:
			SET_MUTE_DISABLE();
			mute_recorde_vol(MUTE);
			close_wm8960_voices();
			break;
		case UNMUTE:
			open_wm8960_voices();
			SET_MUTE_ENABLE();
			mute_recorde_vol(UNMUTE);
			break;
	}
}
/********************************************************
@ 播放接收到手机发送的对讲消息
@ filename:缓存到本地的wav数据的文件路径 	
@
*********************************************************/
void playspeekVoices(const char *filename){
#if 1
	i2s_start_play(8000);
#else
#ifdef CLOSE_VOICE
	Mute_voices(UNMUTE);
#endif
#endif
	playAmrVoices(filename);
#ifdef CLOSE_VOICE
	sleep(2);
	Mute_voices(MUTE);
#endif	//end CLOSE_VOICE
}
#endif
/********************************************************
@ 函数功能:	播放系统音
@ filePath:	路径
@ 返回值: 无
*********************************************************/
void play_sys_tices_voices(char *filePath)
{
	char path[128];
	snprintf(path,128,"%s%s",sysMes.sd_path,filePath);
#ifdef CLOSE_VOICE
	open_wm8960_voices();
#if 0
	if(strstr(path,"TuLin_Wint_8K")){
		mute_recorde_vol(107);
	}else{
		mute_recorde_vol(UNMUTE);
	}
	SET_MUTE_ENABLE();
#else
	mute_recorde_vol(UNMUTE);
	SET_MUTE_ENABLE();
#endif
#endif	//end CLOSE_VOICE

#if 1
	playAmrVoices(path);
#else
	playWavVoices(path);
#endif

	usleep(800*1000);
	if(strstr(filePath,"40002_8k")||strstr(filePath,"no_music_8K")||strstr(filePath,"TuLin_Hahaxiong_8K")){
		pause_record_audio();	//退出播放状态
	}
#ifdef CLOSE_VOICE
	usleep(1000*1000);
	Mute_voices(MUTE);
#endif
}
void exitqttsPlay(void){
	clean_qtts_cache();
	__exitqttsPlay();
}
/********************************************************
@ 函数功能:	播放QTTS数据
@ text:文本		type:文本类型
@ 返回值: 无
*********************************************************/
void PlayQttsText(char *text,unsigned char type)
{
#if 1
	i2s_start_play(8000);
#else
#ifdef CLOSE_VOICE
	Mute_voices(UNMUTE);
#endif
#endif
	char *textbuf= (char *)calloc(1,strlen(text)+2);
	sprintf(textbuf,"%s%s",text,",");
	tolkLog("tolk qtts start\n");
	stait_qtts_cache();
	Qtts_voices_text(textbuf,type);
	tolkLog("tolk qtts end\n");
	printf("qttspos = %d qttsend = %d\n",I2S.qttspos,I2S.qttsend);
	if(I2S.qttspos!=0&&I2S.qttsend==0)
	{
		memset(play_buf+I2S.qttspos,0,I2S_PAGE_SIZE-I2S.qttspos);
		write_pcm(play_buf);
		I2S.qttspos =0;
	}
	tolkLog("tolk qtts clean\n");
	clean_play_cache();
	//memset(play_buf,0,I2S_PAGE_SIZE);
	//write_pcm(play_buf);
	usleep(800*1000);
	tolkLog("tolk qtts pause\n");
	pause_record_audio();	//退出播放状态
	free(textbuf);
	if(I2S.qttsend==1){
		return;
	}
#ifdef CLOSE_VOICE
	usleep(1000*1000);
	Mute_voices(MUTE);
#endif
}
