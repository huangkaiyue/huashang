#include "comshead.h"
#include "qtts_qisc.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"

#define CMD_UNKOWN					-1	//δ֪����
#define CMD_MUSIC_MEUN				0	//�㲥����
#define CMD_WHO_NAME				1	//��Ϲؼ����ݲ���
#define CMD_ADD_VOL					2	//������Ҫ��text
#define CMD_SUB_VOL					3	//������Ҫ��text


/********************************************************
@函数功能:	匹配文字语音控制
@参数:	text 匹配的文本
@返回:	0表示匹配成功
@	  	-1表示匹配失败
*********************************************************/
int CheckinfoText_forContorl(const char *text,char *getPlayMusicName){
	int ret = CMD_UNKOWN;
	if(strstr(text,"名字")||strstr(text,"你是谁")){	
		ret =CMD_WHO_NAME;
	}
	else if(strstr(text,"音量")){
		if((strstr(text,"加")&&strstr(text,"减"))||(strstr(text,"大")&&strstr(text,"小")))
			ret =CMD_UNKOWN;
		else if(strstr(text,"加")||strstr(text,"大")){	
			ret =CMD_ADD_VOL;
		}
		else if(strstr(text,"减")||strstr(text,"小")){	
			ret =CMD_SUB_VOL;
		}
	}else if(strstr(text,"播放")){
		Write_huashangTextLog(text);
		if(Huashang_Checkutf8(text,getPlayMusicName)==0){
			ret =CMD_MUSIC_MEUN;
		}
	}
	return ret;
}

int HandlerPlay_checkTextResult(int cmd,const char *playname,unsigned int playEventNums){
	int ret=-1;
	switch(cmd){
		case CMD_MUSIC_MEUN:
			pause_record_audio();//��Ҫ�л�����ͣ״̬��������Ӹ�����ȥ����------------>��ǰ״̬Ϊ����wav״̬
			Write_huashangTextLog(playname);
			ret =__AddLocalMp3ForPaly(playname,EXTERN_PLAY_EVENT);		
			break;
		case CMD_WHO_NAME:
			ret =PlaySystemAmrVoices(TULING_HAHAXIONG,playEventNums);
			break;
		case CMD_ADD_VOL:
			Setwm8960Vol(VOL_ADD,0);
			ret =PlaySystemAmrVoices(VOICE_ADD,playEventNums);
			ack_VolCtr("add",GetVol());//----------->音量减			
			break;
		case CMD_SUB_VOL:
			Setwm8960Vol(VOL_SUB,0);
			ret =PlaySystemAmrVoices(VOICE_SUB,playEventNums);
			ack_VolCtr("sub",GetVol());//----------->音量减
			break;
	}
	return ret;
}
