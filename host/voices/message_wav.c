#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
#include "config.h"

#if 1
void WriteqttsPcmData(char *data,int len)
{
	int i=0;
	for(i=0;i<len;i+=2){
		memcpy(play_buf+I2S.qttspos,data+i,2);
		I2S.qttspos += 2;
		memcpy(play_buf+I2S.qttspos,data+i,2);
		I2S.qttspos += 2;
		if(I2S.qttsend==1){
			I2S.qttspos=0;
			break;
		}
		if(I2S.qttspos==I2S_PAGE_SIZE){
			write_pcm(play_buf);
			I2S.qttspos=0;
		}
	}
}
void WritePcmData(char *data,int size)
{
	if(I2S.play_size==I2S_PAGE_SIZE)//fix me end is < do?
	{
		I2S.play_size=0;
		write_pcm(play_buf);
	}
	memcpy(play_buf+I2S.play_size,data,size);
	I2S.play_size +=size;
}
#endif
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
	playsysvoicesLog("playsys amr start \n");
	while(1){
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			if(pos>0){
				memset(play_buf+pos,0,I2S_PAGE_SIZE-pos);//清空上一次尾部杂音
				write_pcm(play_buf);
			}
			break;
		}
		memcpy(play_buf+pos,readbuf,2);
		pos+=2;
		memcpy(play_buf+pos,readbuf,2);
		pos+=2;
		if(pos==I2S_PAGE_SIZE){
			playsysvoicesLog("playsys amr while \n");
			write_pcm(play_buf);
			pos=0;
		}
	}
	playsysvoicesLog("playsys amr end \n");
	fclose(fp);
	memset(play_buf,0,pos);
	stait_qtts_cache();
	if(strstr(filename,"TuLin_Wint_8K")){
		return ;
	}
	clean_play_cache();
	remove(outfile);
}
#endif
#ifdef SPEEK_VOICES
/********************************************************
@ 播放接收到手机发送的对讲消息
@ filename:缓存到本地的wav数据的文件路径 	
@
*********************************************************/
void playspeekVoices(const char *filename){
	i2s_start_play(8000);
	playAmrVoices(filename);
#ifdef CLOSE_VOICE
	sleep(2);
	Mute_voices(MUTE);
#endif	//end CLOSE_VOICE
}
#endif
#if 1
void playsysvoices(char *filePath){
	char path[128];
	playsysvoicesLog("playsysvoices start \n");
	i2s_start_play(8000);
	if(strstr(filePath,"no_voices_8K")||strstr(filePath,"start_haha_talk_8k")){
	}else{
		playsysvoicesLog("playsysvoices while start \n");
		while(1){
			if(get_qtts_cache()==0)
				break;
			usleep(1000);
		}	//----fix me
		playsysvoicesLog("playsysvoices while end \n");
		sleep(1);
	}
	snprintf(path,128,"%s%s",sysMes.sd_path,filePath);
	playAmrVoices(path);
	usleep(800*1000);
	clean_qtts_cache();
	pause_record_audio();
#ifdef CLOSE_VOICE
	usleep(1000*1000);
	Mute_voices(MUTE);
#endif
}

/********************************************************
@ 函数功能:	播放系统音
@ filePath:	路径
@ 返回值: 无
*********************************************************/
void play_sys_tices_voices(char *filePath)
{
	char path[128];
	snprintf(path,128,"%s%s",sysMes.sd_path,filePath);
	i2s_start_play(8000);
#ifdef CLOSE_VOICE
	if(strstr(path,"TuLin_Wint_8K")){
		mute_recorde_vol(102);
	}else{
		mute_recorde_vol(UNMUTE);
	}
#endif	//end CLOSE_VOICE
#if 1
	playsysvoicesLog("playsys voices start \n");
	playAmrVoices(path);
	playsysvoicesLog("playsys voices end \n");
#else
	playWavVoices(path);
#endif
	if(strstr(filePath,"TuLin_Wint_8K")||strstr(filePath,"key_down")){
		return;
	}
	clean_qtts_cache_2();	//qtts 正常播放
	usleep(800*1000);
	pause_record_audio();
#ifdef CLOSE_VOICE
	usleep(1000*1000);
	Mute_voices(MUTE);
#endif
}

#else
void playsysvoices(char *filePath){
	playsysvoicesLog("playsysvoices start \n");
	if(strstr(filePath,"no_voices_8K")){
	}else{
		printf("=======what=%d==========\n",get_qtts_cache());
		playsysvoicesLog("playsysvoices while start \n");
		while(1){
			if(get_qtts_cache())
				break;
			usleep(1000);
		}	//----fix me
		playsysvoicesLog("playsysvoices while end \n");
		sleep(1);
	}
	play_sys_tices_voices(filePath);
	clean_qtts_cache();
	pause_record_audio();
}

/********************************************************
@ 函数功能:	播放系统音
@ filePath:	路径
@ 返回值: 无
*********************************************************/
void play_sys_tices_voices(char *filePath)
{
	char path[128];
	snprintf(path,128,"%s%s",sysMes.sd_path,filePath);
	i2s_start_play(8000);
#ifdef CLOSE_VOICE
#ifdef DATOU_JIANG
	if(strstr(path,"TuLin_Wint_8K")){
		mute_recorde_vol(107);
	}else{
		mute_recorde_vol(UNMUTE);
	}
#endif
#endif	//end CLOSE_VOICE
#if 1
	playsysvoicesLog("playsys voices start \n");
	playAmrVoices(path);
	playsysvoicesLog("playsys voices end \n");
#else
	playWavVoices(path);
#endif
	if(strstr(filePath,"TuLin_Wint_8K")){
		return;
	}
	usleep(800*1000);
	if(strstr(filePath,"request_failed_8k")||strstr(filePath,"no_voices_8K")||strstr(filePath,"40002_8k")||strstr(filePath,"no_music_8K")||strstr(filePath,"TuLin_Hahaxiong_8K")){
		pause_record_audio();	//退出播放状态
	}
#ifdef CLOSE_VOICE
	usleep(1000*1000);
	Mute_voices(MUTE);
#endif
}
#endif
void exitqttsPlay(void){
	clean_qtts_cache();
	__exitqttsPlay();
}
void PlayTuLingTaibenQtts(char *text,unsigned char type){
	start_event_play_wav();
	stait_qtts_cache();
	PlayQttsText(text,type);
	pause_record_audio();

}
/********************************************************
@ 函数功能:	播放QTTS数据
@ text:文本		type:文本类型
@ 返回值: 无
*********************************************************/
void PlayQttsText(char *text,unsigned char type)
{
	i2s_start_play(8000);
#if 1
	char *textbuf= (char *)calloc(1,strlen(text)+2);
	if(textbuf==NULL){
		perror("calloc error !!!");
		return;
	}
	sprintf(textbuf,"%s%s",text,",");	//文本尾部添加",",保证文本播报出来
	tolkLog("tolk qtts start\n");
	Qtts_voices_text(textbuf,type);
	free(textbuf);
#else
	Qtts_voices_text(text,type);
#endif
	if(I2S.qttsend==1){
		tolkLog("tolk qtts qttsend == 1\n");
		clean_play_cache();		//清理
		pause_record_audio();	//退出播放状态
#ifdef CLOSE_VOICE
		Mute_voices(MUTE);
#endif
		return;
	}
	if(I2S.qttspos!=0&&I2S.qttsend!=1)
	{
		memset(play_buf+I2S.qttspos,0,I2S_PAGE_SIZE-I2S.qttspos);
		write_pcm(play_buf);
		I2S.qttspos =0;
	}
	tolkLog("tolk qtts clean\n");
	clean_play_cache();		//清理
	usleep(800*1000);
	tolkLog("tolk qtts pause\n");
	pause_record_audio();	//退出播放状态
#ifdef CLOSE_VOICE
	usleep(1000*1000);
	Mute_voices(MUTE);
#endif
}
