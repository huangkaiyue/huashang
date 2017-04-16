#include "comshead.h"
#include "qtts_qisc.h"
#include "host/voices/wm8960i2s.h"
#include "host/studyvoices/prompt_tone.h"

/********************************************************
@函数功能:	匹配文字语音控制
@参数:	text 匹配的文本
@返回:	0表示匹配成功
@	  	-1表示匹配失败
*********************************************************/
int CheckinfoText_forContorl(const char *text){
	int ret = -1;
	if(strstr(text,"本地歌曲")){		
		SetplayNetwork_unLock();
#ifdef TANGTANG_LUO
		PlaySystemAmrVoices(NO_MUSIC);
#else
		pause_record_audio();
#ifdef DATOU_JIANG
		ret=Create_playMusicEvent((const void *)"mp3",1);
#else
		ret=Create_playMusicEvent((const void *)XIAI_DIR,1);
#endif
		if(ret == -1){
			PlaySystemAmrVoices(NO_MUSIC);
		}
#endif
		return 0;
	}
#ifdef DATOU_JIANG
	else if(strstr(text,"本地故事")){
		pause_record_audio();		
		SetplayNetwork_unLock();
		ret=Create_playMusicEvent((const void *)"story",1);
		if(ret == -1){
			PlaySystemAmrVoices(NO_STORY);
		}
		return 0;
	}
#endif
	else if(strstr(text,"名字")||strstr(text,"你是谁")){		
		SetplayNetwork_unLock();
		PlaySystemAmrVoices(TULING_HAHAXIONG);
		return 0;
	}
	else if(strstr(text,"音量")){
		if((strstr(text,"加")&&strstr(text,"减"))||(strstr(text,"大")&&strstr(text,"小")))
			return -1;
		else if(strstr(text,"加")||strstr(text,"大")){			
			SetplayNetwork_unLock();
			Setwm8960Vol(VOL_ADD,0);
			PlaySystemAmrVoices(VOICE_ADD);
			ack_VolCtr("add",GetVol());//----------->音量减
			return 0;
		}
		else if(strstr(text,"减")||strstr(text,"小")){			
			SetplayNetwork_unLock();
			Setwm8960Vol(VOL_SUB,0);
			PlaySystemAmrVoices(VOICE_SUB);
			ack_VolCtr("sub",GetVol());//----------->音量减
			return 0;
		}
	}
	return -1;
}
