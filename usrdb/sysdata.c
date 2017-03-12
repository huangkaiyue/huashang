#include "comshead.h"
#include "sysdata.h"
#include "config.h"
#include "nvram.h"
#include "host/voices/callvoices.h"
#include "../host/studyvoices/qtts_qisc.h"

#if 0
typedef struct {
	char list[24];
	char passwd[24];
}HOST;
extern HOST host;

HOST host;
static int NvramGetCamlist(char *camlist,char *passwd){
	char *nvram_list = nvram_bufget(RT2860_NVRAM, "DMZIPAddress");
	memcpy(camlist,nvram_list,strlen(nvram_list));

	char *nvram_passwd = nvram_bufget(RT2860_NVRAM, "NTPServerIP");
	memcpy(passwd,nvram_passwd,strlen(nvram_passwd));
	if(!strcmp(camlist,""))
		return -1;
	return 0;
}
static int NvramSetCamlist(char *camlist,char *passwd){
	char list[64];
	char pass[64];
	sprintf(list,"%s %s", "nvram_set 2860 DMZIPAddress", camlist);
	system(list);
	
	sprintf(pass,"%s %s", "nvram_set 2860 NTPServerIP", passwd);
	system(pass);

	return 0;
}

int updateSysList(char *list,char *passwd){
	return NvramGetCamlist(list,passwd);
}
void InitSysList(char *frist_camlist,char *camlist_passwd)
{
	memset(&host,0,sizeof(host));
	if(NvramGetCamlist(host.list,host.passwd))
	{
		//printf("get host list failed\n");
		strcpy(host.list,frist_camlist);
		strcpy(host.passwd,camlist_passwd);
		NvramSetCamlist(frist_camlist,camlist_passwd);
	}
}

#endif
//----------------------------播放记录-------------------------------------
#if 0
//获取播放记录
static enum{
	mp3_S=1,
	story_S,
	english_S,
	guoxue_S,
};
void get_paly_num(int *size,unsigned char str)
{
	char *NUM;
	switch(str){
		case mp3_S:
			NUM = nvram_bufget(RT2860_NVRAM, "playMp3Num");
			break;
		case story_S:
			NUM= nvram_bufget(RT2860_NVRAM, "playStoryNum");
			break;
		case english_S:
			NUM= nvram_bufget(RT2860_NVRAM, "playEnglishNum");
			break;
		case guoxue_S:
			NUM= nvram_bufget(RT2860_NVRAM, "playGuoxueNum");
			break;
	}
	*size = (unsigned char)atoi(NUM);
}

//设置播放记录
void set_paly_num(int size,unsigned char str)
{
	char buf_s[64]={0};
	switch(str){
		case mp3_S:
			sprintf(buf_s,"%s %d", "nvram_set 2860 playMp3Num", size);
			break;
		case story_S:
			sprintf(buf_s,"%s %d", "nvram_set 2860 playStoryNum", size);
			break;
		case english_S:
			sprintf(buf_s,"%s %d", "nvram_set 2860 playEnglishNum", size);
			break;
		case guoxue_S:
			sprintf(buf_s,"%s %d", "nvram_set 2860 playGuoxueNum", size);
			break;
	}
	system(buf_s);
	//nvram_bufset(RT2860_NVRAM, "VoiceSIZE",buf_s);
}
#endif
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
		SetSystemTime(1);
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
#ifdef VOICS_CH
//----------------------------播音人-------------------------------------
//设置声音选择
void SaveVolCh_toRouteTable(unsigned char ch){
	set_volch(ch);//在线
	char buf_s[64]={0};
	sprintf(buf_s,"nvram_set 2860 VoiceCH %d",  ch);
	system(buf_s);
}

//获取声音选择
void GetVolCh_formRouteTable(unsigned char *ch){
	char *buf_ch = nvram_bufget(RT2860_NVRAM, "VoiceCH");
	*ch = (unsigned char)atoi(buf_ch);
}
#endif //end VOICS_CH
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
