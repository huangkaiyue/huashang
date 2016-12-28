#include "comshead.h"
#include "qtts_qisc.h"
#include "host/voices/wm8960i2s.h"
#include "host/studyvoices/prompt_tone.h"

/********************************************************
@函数功能:	匹配文字语音控制
@参数:	text 匹配的文本
@返回:	1表示匹配成功
@	  	0表示匹配失败
*********************************************************/
int check_text_cmd(char *text)
{
	int ret = -1;
	if(strstr(text,"播放音乐")){
#ifdef TANGTANG_LUO
		playsysvoices(NO_MUSIC);
		//PlayQttsText("小朋友，我还不会唱歌，你教我唱吧。",QTTS_UTF8);
#else
		pause_record_audio();
#ifdef DATOU_JIANG
		ret=createPlayEvent((const void *)"mp3",1);
#else
		ret=createPlayEvent((const void *)"xiai",1);
#endif
		if(ret == -1){
			PlayQttsText("小朋友，我还不会唱歌，赶紧收藏歌曲，教我唱歌吧。",QTTS_UTF8);
		}
#endif
		return 1;
	}
#ifdef DATOU_JIANG
	else if(strstr(text,"讲故事")){
		ret=createPlayEvent((const void *)"story",1);
		if(ret == -1){
			PlayQttsText("小朋友，我还没有故事，赶紧收藏故事，教我讲故事吧。",QTTS_UTF8);
		}
		return 1;
	}
#endif
	else if(strstr(text,"图灵")){
		playsysvoices(TULING_HAHAXIONG);
		//PlayQttsText("我叫大头，聪明又可爱的大头。",QTTS_UTF8);
		//PlayQttsText("我就是风流倜傥，玉树临风，人见人爱，花见花开，车见爆胎，聪明又可爱的糍粑糖，你也可以叫我糖糖，我们做好朋友吧。",QTTS_UTF8);
		return 1;
	}
	else if(strstr(text,"音量")){
		if((strstr(text,"加")&&strstr(text,"减"))||(strstr(text,"大")&&strstr(text,"小")))
			return 0;
		else if(strstr(text,"加")||strstr(text,"大")){
			PlayQttsText("音量加设置成功。",QTTS_UTF8);
			SetVol(VOL_ADD,0);
			pause_record_audio();
		}
		else if(strstr(text,"减")||strstr(text,"小")){
			SetVol(VOL_SUB,0);
			PlayQttsText("音量减设置成功。",QTTS_UTF8);
			pause_record_audio();
		}
		return 1;
	}
	return 0;
}