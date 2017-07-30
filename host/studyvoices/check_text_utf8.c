#include "comshead.h"
#include "host/studyvoices/qtts_qisc.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "nvram.h"

#define CMD_UNKOWN					-1	//未知命令
#define CMD_MUSIC_MEUN				0	//点播音乐
#define CMD_WHO_NAME				1	//打断关键数据播放
#define CMD_ADD_VOL					2	//发现重要的text
#define CMD_SUB_VOL					3	//发现重要的text
#define CMD_CLOSE					4	//发现重要的text
#define CMD_ID						5	//发现重要的text
#define CMD_AGE						6	//年龄

/********************************************************
@函数功能:      匹配文字语音控制
@参数:  text 匹配的文本
@返回:  匹配的关键命令
*********************************************************/
int CheckinfoText_forContorl(const char *infoText,const char *text,char *getPlayMusicName){
	int ret = CMD_UNKOWN;
	int playIndex=0;
	if(strstr(infoText,"你是谁")||strstr(infoText,"名字")||strstr(text,"日辉")||strstr(text,"智娃")){	
		ret =CMD_WHO_NAME;
	}
	else if(strstr(infoText,"音量")){
		if((strstr(infoText,"加")&&strstr(infoText,"减"))||(strstr(infoText,"大")&&strstr(infoText,"小")))
			ret =CMD_UNKOWN;
		else if(strstr(infoText,"加")||strstr(infoText,"大")){	
			ret =CMD_ADD_VOL;
		}
		else if(strstr(infoText,"减")||strstr(infoText,"小")){	
			ret =CMD_SUB_VOL;
		}
	}else if(strstr(infoText,"拜拜")||strstr(infoText,"晚安")||strstr(infoText,"你先休息")||strstr(infoText,"你先去睡觉")){
			ret =CMD_CLOSE;
	}else if(strstr(infoText,"id")&&strstr(infoText,"我的")){
		ret = CMD_ID;
	}
	else if(strstr(infoText,"播放")){
		Write_huashangTextLog(infoText);
		if(Huashang_Checkutf8(infoText,getPlayMusicName,&playIndex)==0){
			Huashang_updatePlayindex(playIndex);
			ret =CMD_MUSIC_MEUN;
		}
	}else if(strstr(infoText,"你")&&strstr(infoText,"岁")||strstr(infoText,"你")&&strstr(infoText,"多大")){
		ret =CMD_AGE;
	}else if(strstr(infoText,"大声")||strstr(infoText,"听不到")){
		ret =CMD_ADD_VOL;
	}else if(strstr(infoText,"小声")||strstr(infoText,"太吵")){
		ret =CMD_SUB_VOL;
	}
	return ret;
}

int HandlerPlay_checkTextResult(int cmd,const char *playname,unsigned int playEventNums){
	int ret=-1;
	char *list=NULL;
	char *PlayText=NULL;
	switch(cmd){		
		case CMD_MUSIC_MEUN:
			pause_record_audio();//需要切换到暂停状态，才能添加歌曲进去播放------------>当前状态为播放wav状态
			Write_huashangTextLog(playname);
			ret =__AddLocalMp3ForPaly(playname,EXTERN_PLAY_EVENT);		
			break;		
		case CMD_WHO_NAME:
			ret =PlaySystemAmrVoices(AMR_58_WHO_NAME,playEventNums);
			break;
		case CMD_ADD_VOL:
			Setwm8960Vol(VOL_ADD,0);
			ret =PlaySystemAmrVoices(AMR_ADD_VOL,playEventNums);
			ack_VolCtr("add",GetVol());	
			break;
		case CMD_SUB_VOL:
			Setwm8960Vol(VOL_SUB,0);
			ret =PlaySystemAmrVoices(AMR_SUB_VOL,playEventNums);
			ack_VolCtr("sub",GetVol());
			break;
		case CMD_CLOSE:
			closeSystem();
			ret =-1;
			break;
		case CMD_ID:
			list= nvram_bufget(RT2860_NVRAM, "list");
			PlayText=(char *)calloc(1,strlen(list)+8);
			if(PlayText==NULL){
				return ret;
			}
			sprintf(PlayText,"我的编号是 %s%s",list," ,");
			enabledownNetworkVoiceState();
			ret =PlayQttsText(PlayText,QTTS_UTF8,"vinn",playEventNums,50);
			free(PlayText);
			break;
		case CMD_AGE:
			ret =PlaySystemAmrVoices(AMR_60_AGE,playEventNums);
			break;
	}
	return ret;
}
