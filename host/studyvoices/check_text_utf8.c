#include "comshead.h"
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
	if(strstr(text,"音乐")){
		//playsysvoices(NO_MUSIC);
		//PlayQttsText("小朋友，我还不会唱歌，你教我唱吧。",QTTS_UTF8);
		pause_record_audio();
		//createPlayEvent((const void *)"mp3",1);
		createPlayEvent((const void *)"xiai",1);
		return 1;
	}
	else if(strstr(text,"图灵")){
		playsysvoices(TULING_HAHAXIONG);
		//PlayQttsText("我叫大头，聪明又可爱的大头。",QTTS_UTF8);
		//PlayQttsText("我就是风流倜傥，玉树临风，人见人爱，花见花开，车见爆胎，聪明又可爱的糍粑糖，你也可以叫我糖糖，我们做好朋友吧。",QTTS_UTF8);
		return 1;
	}
	else if(strstr(text,"音量")){
		if((strstr(text,"加")&&strstr(text,"减"))||(strstr(text,"大")&&strstr(text,"小")))
			return 0;
		else if(strstr(text,"加")||strstr(text,"大"))
			SetVol(VOL_ADD,0);
		else if(strstr(text,"减")||strstr(text,"小"))
			SetVol(VOL_SUB,0);	
		return 1;
	}
	return 0;
}