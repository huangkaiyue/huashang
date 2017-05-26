#include "comshead.h"
#include "qtts_qisc.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"

#define CMD_UNKOWN					-1	//Î´ÖªÃüÁî
#define CMD_MUSIC_MEUN				0	//µã²¥ÒôÀÖ
#define CMD_WHO_NAME				1	//´ò¶Ï¹Ø¼üÊı¾İ²¥·Å
#define CMD_ADD_VOL					2	//·¢ÏÖÖØÒªµÄtext
#define CMD_SUB_VOL					3	//·¢ÏÖÖØÒªµÄtext


/********************************************************
@å‡½æ•°åŠŸèƒ½:	åŒ¹é…æ–‡å­—è¯­éŸ³æ§åˆ¶
@å‚æ•°:	text åŒ¹é…çš„æ–‡æœ¬
@è¿”å›:	0è¡¨ç¤ºåŒ¹é…æˆåŠŸ
@	  	-1è¡¨ç¤ºåŒ¹é…å¤±è´¥
*********************************************************/
int CheckinfoText_forContorl(const char *text,char *getPlayMusicName){
	int ret = CMD_UNKOWN;
	if(strstr(text,"åå­—")||strstr(text,"ä½ æ˜¯è°")){	
		ret =CMD_WHO_NAME;
	}
	else if(strstr(text,"éŸ³é‡")){
		if((strstr(text,"åŠ ")&&strstr(text,"å‡"))||(strstr(text,"å¤§")&&strstr(text,"å°")))
			ret =CMD_UNKOWN;
		else if(strstr(text,"åŠ ")||strstr(text,"å¤§")){	
			ret =CMD_ADD_VOL;
		}
		else if(strstr(text,"å‡")||strstr(text,"å°")){	
			ret =CMD_SUB_VOL;
		}
	}else if(strstr(text,"æ’­æ”¾")){
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
			pause_record_audio();//ĞèÒªÇĞ»»µ½ÔİÍ£×´Ì¬£¬²ÅÄÜÌí¼Ó¸èÇú½øÈ¥²¥·Å------------>µ±Ç°×´Ì¬Îª²¥·Åwav×´Ì¬
			Write_huashangTextLog(playname);
			ret =__AddLocalMp3ForPaly(playname,EXTERN_PLAY_EVENT);		
			break;
		case CMD_WHO_NAME:
			ret =PlaySystemAmrVoices(TULING_HAHAXIONG,playEventNums);
			break;
		case CMD_ADD_VOL:
			Setwm8960Vol(VOL_ADD,0);
			ret =PlaySystemAmrVoices(VOICE_ADD,playEventNums);
			ack_VolCtr("add",GetVol());//----------->éŸ³é‡å‡			
			break;
		case CMD_SUB_VOL:
			Setwm8960Vol(VOL_SUB,0);
			ret =PlaySystemAmrVoices(VOICE_SUB,playEventNums);
			ack_VolCtr("sub",GetVol());//----------->éŸ³é‡å‡
			break;
	}
	return ret;
}
