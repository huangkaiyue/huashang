#include "comshead.h"
#include "sysdata.h"
#include "config.h"
#include "nvram.h"
#include "host/voices/callvoices.h"
#include "../host/studyvoices/qtts_qisc.h"


//----------------------------权限次数-------------------------------------
#ifdef	SYSTEMLOCK
//设置权限次数
void setSystemLock(int size){
	char buf_s[64]={0};
	sprintf(buf_s,"%s %d", "nvram_set 2860 SystemLock", size);
	system(buf_s);
	//nvram_bufset(RT2860_NVRAM, "VoiceSIZE",buf_s);
}
//获取权限次数,检查是否到达上线
void checkSystemLock(void){
	char *number = nvram_bufget(RT2860_NVRAM, "SystemLock");
	int opennumber = (unsigned char)atoi(number);
	if(opennumber>SYSTEMLOCKNUM){	//检查开机次数
		Create_PlayQttsEvent("权限次数不够。请联系软件所属公司，深圳日晖网讯有限公司，常先生。或者唐工 QQ ：121109281。",QTTS_GBK);
		sleep(10);
		Create_PlayQttsEvent("权限次数不够。请联系软件所属公司，深圳日晖网讯有限公司，常先生。或者唐工 QQ ：121109281。",QTTS_GBK);
		sleep(10);
		Create_PlayQttsEvent("权限次数不够。请联系软件所属公司，深圳日晖网讯有限公司，常先生。或者唐工 QQ ：121109281。",QTTS_GBK);
		sleep(10);
		Create_PlayQttsEvent("权限次数不够。请联系软件所属公司，深圳日晖网讯有限公司，常先生。或者唐工 QQ ：121109281。",QTTS_GBK);
		sleep(10);
		SetMucClose_Time(1);
	}
	setSystemLock((opennumber+1));
}
#endif

//----------------------------音量-------------------------------------
//从路由表当中获取音量
void GetVol_formRouteTable(unsigned char *size){
	char *vol = nvram_bufget(RT2860_NVRAM, "VoiceSIZE");
	if(!strcmp(vol,"")){
		*size=105;
	}else{
		*size = (unsigned char)atoi(vol);
	}
}

//设置音量到路由表当中
void SaveVol_toRouteTable(unsigned char vol){
	char buf_s[64]={0};
	sprintf(buf_s,"nvram_set 2860 VoiceSIZE %d", vol);
	system(buf_s);
}
//----------------------------开关机-------------------------------------
//设置开关机到路由表当中
void Save_OpenCloseTime_toRouteTable(int type,unsigned char *time){
	char buf_s[64]={0};
	if(type==0)	//关机时间
		sprintf(buf_s,"nvram_set 2860 Closetime %s", time);
	else if (type==1)//开机时间
		sprintf(buf_s,"nvram_set 2860 Opentime %s", time);
	system(buf_s);
}

//获取开关机时间
void Get_OpenCloseTime_formRouteTable(int type, char *time){
	char *buf=NULL;
	if(type==0){
		buf= nvram_bufget(RT2860_NVRAM, "Closetime");
	}
	else if (type==1){
		buf= nvram_bufget(RT2860_NVRAM, "Opentime");
	}
	memcpy(time,buf,strlen(buf));
}

//关机时候保存图灵的token值到路由表当中
void Save_TulingToken_toRouteTable(const char *tokenVal){
	char buf_s[128]={0};
	sprintf(buf_s,"nvram_set 2860 tokenVal %s", tokenVal);
	system(buf_s);
}
//----------------------------end-------------------------------------
