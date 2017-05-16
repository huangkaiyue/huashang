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
//播放单声道wav格式音频数据
static void PlaySignleWavVoices(const char *playfilename,unsigned char playMode,unsigned int playEventNums){
	int r_size=0,pos=0;
	char readbuf[2]={0};
	FILE *fp= fopen(playfilename,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return ;
	}
	SetWm8960Rate(RECODE_RATE);
	fseek(fp,WAV_HEAD,SEEK_SET);		//跳过wav头部	
	while(1){
		if(playMode==PLAY_IS_INTERRUPT&&playEventNums!=GetCurrentEventNums()){
			pause_record_audio();
			CleanI2S_PlayCachedata();//清理
			StopplayI2s();			 //最后一片数据丢掉
			break;
		}	
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			if(pos>0){
				memset(play_buf+pos,0,I2S_PAGE_SIZE-pos);		//清空上一次尾部杂音,并播放尾音
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
//播放单声道amr格式音频数据
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
@ 播放接收到手机发送的对讲消息
@ filename:缓存到本地的wav数据的文件路径 (播放完需要删除)
@
*********************************************************/
void playspeekVoices(const char *filename,unsigned int playEventNums){
	playAmrVoices(filename,PLAY_IS_INTERRUPT,playEventNums);
	remove(filename);
}
/********************************************************
@ 函数功能:	播放系统音
@ filePath:	路径
@ 返回值: 无
*********************************************************/
void PlaySystemAmrVoices(const char *filePath,unsigned int playEventNums){
	__playAmrVoices(filePath,PLAY_IS_INTERRUPT,playEventNums);
}
//播放过渡音，不允许打断
void play_waitVoices(const char *filePath,unsigned int playEventNums){
	__playAmrVoices(filePath,PLAY_IS_COMPLETE,playEventNums);
}

/********************************************************
@ 函数功能:	播放QTTS数据
@ text:文本		type:文本类型
@ 返回值: 无
*********************************************************/
int PlayQttsText(const char *text,unsigned char type,const char *playVoicesName,unsigned int playEventNums){
	int ret =-1;
	SetWm8960Rate(RECODE_RATE);
	char *textbuf= (char *)calloc(1,strlen(text)+2);
	if(textbuf==NULL){
		perror("calloc error !!!");
		return ret;
	}
	sprintf(textbuf,"%s%s",text,",");	//文本尾部添加",",保证文本播报出来
	PlayQtts_log("play qtts start\n");
	ret =Qtts_voices_text(textbuf,type,playVoicesName,playEventNums);
	free(textbuf);
	return ret;

}
/********************************************************
@ 函数功能:	播放图灵数据
@ url:图灵发送
@ 返回值: 0 正常退出 -1非正常退出
*********************************************************/
int PlayTulingText(HandlerText_t *handtext){
	return downTulingMp3_forPlay(handtext);
}	
