#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
#include "host/studyvoices/std_worklist.h"
#include "gpio_7620.h"
#include "uart/uart.h"
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
static int PlayStartWavVoices(const char *playfilename,int eventsNum){
	int r_size=0,pos=0;
	char readbuf[2]={0};
	int ret=0;
	FILE *fp= fopen(playfilename,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return -1;
	}
	fseek(fp,WAV_HEAD,SEEK_SET);		//跳过wav头部	
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
	int playInterruptEvent =1;	//过滤第一次事件打断,直到第二次事件打断才会退出
	while(1){
		if(eventsNum!=0){
			if(GetCurrentEventNums()!=eventsNum){
				--playInterruptEvent;
			}
			if(playInterruptEvent==0){
				break;
			}
			
		}
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
//开机播放的起始音
void PlayStartPcm(const char *filename,int eventsNum){
	char *outfile ="speek.wav";
	char path[128]={0};
	snprintf(path,128,"%s%s",sysMes.localVoicesPath,filename);
	AmrToWav8k(path,(const char *)outfile);
	PlayStartWavVoices(outfile,eventsNum);

}
//播放单声道wav格式音频数据
static int PlaySignleWavVoices(const char *playfilename,unsigned char playMode,unsigned int playEventNums,unsigned char showFace){
	int r_size=0,pos=0;
	char readbuf[2]={0};
	int ret=0;
	FILE *fp= fopen(playfilename,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return -1;
	}
	fseek(fp,WAV_HEAD,SEEK_SET);		//跳过wav头部	
#if defined(HUASHANG_JIAOYU)	
	if(showFace==SHOW_FACE){
		Show_tlak_Light();	
		led_lr_oc(closeled);
		usleep(100000);
		showFacePicture(WAIT_CTRL_NUM3);
	}
#endif
	while(1){
		if(playMode==PLAY_IS_INTERRUPT&&playEventNums!=GetCurrentEventNums()){
			CleanI2S_PlayCachedata();//清理
			StopplayI2s();			 //最后一片数据丢掉
			ret=-1;
			Mute_voices(MUTE);
			break;
		}	
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			if(pos>0){
				memset(play_buf+pos,0,I2S_PAGE_SIZE-pos);		//清空上一次尾部杂音,并播放尾音
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
#if defined(HUASHANG_JIAOYU)	
	if(showFace==SHOW_FACE){
		Close_tlak_Light();
		led_lr_oc(openled);
		usleep(100000);
		showFacePicture(WAIT_CTRL_NUM4);
	}
#endif	
	return ret;
}

int __playResamplePlayPcmFile(const char *pcmFile,unsigned int playEventNums){
	int currentRate = 0,ret =-1;
	int timeOut=0;
	while(1){		
		currentRate = GetWm8960Rate();
		if(currentRate!=RECODE_RATE){
			break;
		}	
		if(GetCurrentEventNums()!=playEventNums){
			return -1;
		}
		if(++timeOut>8){
			return -1;
		}
		usleep(200000);	//wait main pthread set rate to music
	}
	StreamPause();
	start_event_play_soundMix();//切换到混音播放状态		
	ret = SetWm8960Rate(RECODE_RATE,(const char *)"__playResamplePlayPcmFile set rate");
	usleep(100000);
	ret =PlaySignleWavVoices((const char *)pcmFile,PLAY_IS_INTERRUPT,playEventNums,SHOW_FACE);	
	start_event_play_Mp3music();
	//需要优化一下，检查到还有音频文件
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

//播放单声道amr格式音频数据
static int __playAmrVoices(const char *filename,unsigned char playMode,unsigned int playEventNums,unsigned char showFace){
	char *outfile ="speek.wav";
	AmrToWav8k(filename,(const char *)outfile);
	SetWm8960Rate(RECODE_RATE,(const char *)"__playAmrVoices set rate");
	int ret = PlaySignleWavVoices((const char *)outfile,playMode,playEventNums,showFace);
	if(ret==0){	//正常播放完，把录音线程挂起来
		pause_record_audio();
	}
	remove(outfile);
	return ret;
}
static int __playSystemAmrVoices(const char *filePath,unsigned char playMode,unsigned int playEventNums,unsigned char showFace){
	char path[128]={0};
	snprintf(path,128,"%s%s",sysMes.localVoicesPath,filePath);
	return __playAmrVoices(path,playMode,playEventNums,showFace);
}
/********************************************************
@ 播放接收到手机发送的对讲消息
@ filename:缓存到本地的wav数据的文件路径 (播放完需要删除)
@
*********************************************************/
int PlayWeixin_SpeekAmrFileVoices(const char *filename,unsigned int playEventNums,unsigned char mixMode){
	int ret =-1;
	if(mixMode==MIX_PLAY_PCM){
		ret =playResamplePlayAmrFile(filename,playEventNums);
	}else{
		start_event_play_wav();
		ret= __playAmrVoices(filename,PLAY_IS_INTERRUPT,playEventNums,SHOW_FACE);
	}
	if(strstr(filename,"/home/qtts/")==NULL){
		remove(filename);
	}
	return ret;
}
/********************************************************
@ 函数功能:	播放系统音
@ filePath:	路径
@ 返回值: 无
*********************************************************/
int PlaySystemAmrVoices(const char *filePath,unsigned int playEventNums){
	return __playSystemAmrVoices(filePath,PLAY_IS_INTERRUPT,playEventNums,SHOW_FACE);
}
//播放过渡音，不允许打断
void PlayImportVoices(const char *filePath,unsigned int playEventNums){
	__playSystemAmrVoices(filePath,PLAY_IS_COMPLETE,playEventNums,SHOW_FACE);
}

void playVoicesNotFace(const char *filePath,unsigned int playEventNums){
	__playSystemAmrVoices(filePath,PLAY_IS_INTERRUPT,playEventNums,NOT_FACE);
}

/********************************************************
@ 函数功能:	播放QTTS数据
@ text:文本		type:文本类型
@ 返回值: 无
*********************************************************/
int PlayQttsText(const char *text,unsigned char type,const char *playVoicesName,unsigned int playEventNums,int playSpeed){
	SetWm8960Rate(RECODE_RATE,(const char *)"PlayQttsText set rate");
	usleep(100000);
	Show_tlak_Light();					
	return Qtts_voices_text(text,type,playVoicesName,playEventNums,playSpeed);

}
/********************************************************
@ 函数功能:	播放图灵数据
@ url:图灵发送
@ 返回值: 0 正常退出 -1非正常退出
*********************************************************/
int PlayTulingText(HandlerText_t *handtext){
	return downTulingMp3_forPlay(handtext);
}	
