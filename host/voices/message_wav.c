#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
#include "config.h"

#if 1
void WriteqttsPcmData(char *data,int len){
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
void WritePcmData(char *data,int size){
	if(I2S.play_size==I2S_PAGE_SIZE)//fix me end is < do?
	{
		I2S.play_size=0;
		write_pcm(play_buf);
	}
	memcpy(play_buf+I2S.play_size,data,size);
	I2S.play_size +=size;
}
#endif
#ifdef SYSVOICE
static unsigned char systemsign=0;
void InterruptSystemVoice(void){
	systemsign=1;
}
static void PlaySystemVoice(void){
	systemsign=0;
}
int GetSystemSign(void){
	return systemsign;
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
	start_event_play_wav(2);
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
void playAmrVoices(const char *filename){
	char *outfile ="speek.wav";
	AmrToWav8k(filename,(const char *)outfile);
	int r_size=0,pos=0;
	char readbuf[2];
	FILE *fp= fopen(outfile,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return;
	}
	start_event_play_wav(3);
#ifdef SYSVOICE
	PlaySystemVoice();
#endif
	fseek(fp,WAV_HEAD,SEEK_SET);		//跳过wav头部	
	playsysvoicesLog("playsys amr start \n");
	while(1){
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			if(pos>0){
				memset(play_buf+pos,0,I2S_PAGE_SIZE-pos);		//清空上一次尾部杂音
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
#ifdef SYSVOICE
		if(GetSystemSign()==1){
			CleanI2S_PlayCachedata();		//清理
			stopclean();	//最后一片数据丢掉
			break;
		}
#endif	
	}
	playsysvoicesLog("playsys amr end \n");
	fclose(fp);
	memset(play_buf,0,pos);
	stait_qtts_cache();
	CleanI2S_PlayCachedata();
#ifdef SYSVOICE
	InterruptSystemVoice();
#endif
	if(strstr(filename,"TuLin_Wint_8K")){
		return ;
	}
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
	SetWm8960Rate(RECODE_RATE);
	playAmrVoices(filename);
#ifdef CLOSE_VOICE
	sleep(2);
	Mute_voices(MUTE);
#endif	//end CLOSE_VOICE
}
#endif
void playsysvoices(char *filePath){
	char path[128];
	playsysvoicesLog("playsysvoices start \n");
	SetWm8960Rate(RECODE_RATE);
	if(strstr(filePath,"no_voices_8K")||strstr(filePath,"start_haha_talk_8k")){
	}else{
		playsysvoicesLog("playsysvoices while start \n");
		while(1){
			start_event_play_wav(4);
			if(get_qtts_cache()==1){
				pause_record_audio(11);
				clean_qtts_cache_2();
				return;
			}
			if(get_qtts_cache()==0)
				break;
			usleep(1000);
		}	//----fix me
		playsysvoicesLog("playsysvoices while end\n");
		sleep(1);
	}
	snprintf(path,128,"%s%s",sysMes.sd_path,filePath);
	printf("system voice path : %s\n",path);
	playAmrVoices(path);
#ifdef CLOSE_VOICE
	usleep(800*1000);
	clean_qtts_cache_2();
	pause_record_audio(12);
	usleep(1000*1000);
	Mute_voices(MUTE);
#else
	clean_qtts_cache_2();
	pause_record_audio(13);
#endif
}

/********************************************************
@ 函数功能:	播放系统音
@ filePath:	路径
@ 返回值: 无
*********************************************************/
void play_sys_tices_voices(char *filePath){
	char path[128]={0};
	snprintf(path,128,"%s%s",sysMes.sd_path,filePath);
	SetWm8960Rate(RECODE_RATE);
	if(strstr(path,"TuLin_Wint_8K")){ 
		mute_recorde_vol(105);
	}
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
#ifdef CLOSE_VOICE
	usleep(800*1000);
	pause_record_audio(14);
	usleep(1000*1000);
	Mute_voices(MUTE);
#else
	pause_record_audio(15);
#endif
}
void exitqttsPlay(void){
	clean_qtts_cache();
	__exitqttsPlay();
}
void PlayTuLingTaibenQtts(char *text,unsigned char type){
	start_event_play_wav(5);
	stait_qtts_cache();
	PlayQttsText(text,type);
	pause_record_audio(16);

}
/********************************************************
@ 函数功能:	播放QTTS数据
@ text:文本		type:文本类型
@ 返回值: 无
*********************************************************/
void PlayQttsText(char *text,unsigned char type){
	SetWm8960Rate(RECODE_RATE);
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
#if 1
		Mute_voices(MUTE);
#endif
		pause_record_audio(17);	//退出播放状态
		CleanI2S_PlayCachedata();		//清理
		stopclean();	//最后一片数据丢掉
		return;
	}
	if(I2S.qttspos!=0&&I2S.qttsend!=1)
	{
		memset(play_buf+I2S.qttspos,0,I2S_PAGE_SIZE-I2S.qttspos);
		write_pcm(play_buf);
		I2S.qttspos =0;
	}
	tolkLog("tolk qtts clean\n");
#ifdef CLOSE_VOICE
	usleep(800*1000);
	tolkLog("tolk qtts pause\n");
	pause_record_audio(18);	//退出播放状态
	CleanI2S_PlayCachedata();		//清理
	usleep(1000*1000);
	Mute_voices(MUTE);
#else
	pause_record_audio(19);	//退出播放状态
	CleanI2S_PlayCachedata();		//清理
#endif
}

/********************************************************
@ 函数功能:	播放图灵数据
@ text:文本		type:文本类型
@ 返回值: 0 正常退出 -1非正常退出
*********************************************************/
int PlayTulingText(const char *url)
{
	//stait_qtts_cache();
	SetWm8960Rate(RECODE_RATE);
	tolkLog("tolk tuling start\n");
	downTulingMp3((const char*)url);
	tolkLog("tolk tuling end\n");
	if(I2S.qttsend==1){
		tolkLog("tolk qtts qttsend == 1\n");
#if 1
		Mute_voices(MUTE);	//-----bug
#endif
		pause_record_audio(20);	//退出播放状态
		CleanI2S_PlayCachedata();		//清理
		stopclean();	//最后一片数据丢掉
		return -1;
	}
	printf("=======qttspos=%d============\n",I2S.qttsend);
	if(I2S.qttspos!=0&&I2S.qttsend!=1)
	{
		memset(play_buf+I2S.qttspos,0,I2S_PAGE_SIZE-I2S.qttspos);
		write_pcm(play_buf);
		I2S.qttspos =0;
	}
#ifdef CLOSE_VOICE
	usleep(800*1000);
	tolkLog("tolk tuling pause\n");
	pause_record_audio(21);	//退出播放状态
	CleanI2S_PlayCachedata();		//清理
	usleep(1000*1000);
	Mute_voices(MUTE);
#else
	pause_record_audio(22);	//退出播放状态
	CleanI2S_PlayCachedata();		//清理
#endif
	return 0;
}

