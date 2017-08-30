#include "comshead.h"
#include "sysdata.h"
#include "config.h"
#include "nvram.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/qtts_qisc.h"
#include "host/voices/wm8960i2s.h"

//----------------------------音量-------------------------------------
//从路由表当中获取音量
void GetVol_formRouteTable(unsigned char *size){
	char *vol = nvram_bufget(RT2860_NVRAM, "VoiceSIZE");
	if(!strcmp(vol,"")){
		*size=SYSTEM_DEFALUT_VOL;
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
static unsigned char runplay=0;
void Set_VersionRun(void){
	char buf_s[128]={0};
	sprintf(buf_s,"nvram_set 2860 run_ok %s", "rhw");
	system(buf_s);
}
int Get_VersionRun(void){
	char *buf= nvram_bufget(RT2860_NVRAM, "run_ok");
	if(!strncmp(buf,"rhw",3)){
		return 0;
	}	
	runplay=1;
	return -1;
}
int getFactoryTest(void){
	return (int)runplay;
}
void Set7688Wifi(const char *wifi){
	char buf_s[128]={0};
	sprintf(buf_s,"nvram_set 2860 SSID1 %s", wifi);
	system(buf_s);
}
//----------------------------end-------------------------------------
