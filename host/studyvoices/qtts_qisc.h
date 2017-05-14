#ifndef _QTTS_QISC_H
#define _QTTS_QISC_H

#include <stdio.h>
#include "config.h"

#define QTTS_GBK	0
#define QTTS_UTF8	1



#define DOWN_QTTS_QUIT	0
#define DOWN_QTTS_ING	1
#define PLAY_QTTS_QUIT	0
#define PLAY_QTTS_ING	1
#define PLAY_QTTS_WAIT	2

//#define VINN_GBK	"vcn=vinn,aue = speex-wb,auf=audio/L16;rate=8000,spd=7,vol = 8,rdn = 3,tte = gbk"
#define VIMM_GBK	"voice_name=vinn,text_encoding=gbk,sample_rate=8000,speed=50,volume=50,pitch=50,rdn =3"
#define VIMM_UTF8	"voice_name=vinn,text_encoding=utf8,sample_rate=8000,speed=50,volume=50,pitch=50,rdn =3"

#define DBG_QTTS
#ifdef 	DBG_QTTS 
#define DEBUG_QTTS(fmt, args...) printf("%s: " ,__func__,fmt, ## args)
#else   
#define DEBUG_QTTS(fmt, args...) { }
#endif	//end DBG_AP_STA

extern int Qtts_voices_text(char *text,unsigned char type,const char *playVoicesName,unsigned int playEventNums);

extern int Init_Iat_MSPLogin(void);

extern void Iat_MSPLogout(void);

#endif
