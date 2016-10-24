#include "comshead.h"
#include "sysdata.h"
#include "config.h"
#include "nvram.h"
#include "host/voices/callvoices.h"

HOST host;
static int NvramGetCamlist(char *camlist,char *passwd)
{
	char *nvram_list = nvram_bufget(RT2860_NVRAM, "DMZIPAddress");
	memcpy(camlist,nvram_list,strlen(nvram_list));

	char *nvram_passwd = nvram_bufget(RT2860_NVRAM, "NTPServerIP");
	memcpy(passwd,nvram_passwd,strlen(nvram_passwd));
	if(!strcmp(camlist,""))
		return -1;
	return 0;
}
static int NvramSetCamlist(char *camlist,char *passwd)
{
	char list[64];
	char pass[64];
	sprintf(list,"%s %s", "nvram_set 2860 DMZIPAddress", camlist);
	system(list);
	
	sprintf(pass,"%s %s", "nvram_set 2860 NTPServerIP", passwd);
	system(pass);

	//nvram_bufset(RT2860_NVRAM, "DMZIPAddress",camlist);
	//nvram_bufset(RT2860_NVRAM, "NTPServerIP",passwd);
	return 0;
}

int updateSysList(char *list,char *passwd)
{
	return NvramGetCamlist(list,passwd);
}
//----------------------------播放记录-------------------------------------
#ifdef LOCAL_MP3
static int playMp3Num = 0;
static int playStoryNum = 0;
static int playEnglishNum = 0;
static unsigned char play_get_num=0;
void get_paly_num(int *size,unsigned char str)
{
	switch(str){
		case mp3:
			*size = playMp3Num;
			break;
		case story:
			*size = playStoryNum;
			break;
		case english:
			*size = playEnglishNum;
			break;
	}
}
void set_paly_num(int size,unsigned char str)
{
	switch(str){
		case mp3:
			playMp3Num = size;
			play_get_num++;
			break;
		case story:
			playStoryNum = size;
			play_get_num++;
			break;
		case english:
			playEnglishNum = size;
			play_get_num++;
			break;
	}
	if(play_get_num>20){
		pool_add_task(set_paly_sys_num,NULL);
		play_get_num=0;
	}
}

//获取播放记录
void get_paly_sys_num(void)
{
	char *NUM;
	NUM = nvram_bufget(RT2860_NVRAM, "playMp3Num");
	playMp3Num = (unsigned char)atoi(NUM);
	usleep(100);
	NUM= nvram_bufget(RT2860_NVRAM, "playStoryNum");
	playStoryNum = (unsigned char)atoi(NUM);
	usleep(100);
	NUM= nvram_bufget(RT2860_NVRAM, "playEnglishNum");
	playEnglishNum = (unsigned char)atoi(NUM);
	usleep(100);
}

//设置播放记录
void set_paly_sys_num(void)
{
	char buf_s[64]={0};
	sprintf(buf_s,"%s %d", "nvram_set 2860 playMp3Num", playMp3Num);
	system(buf_s);
	usleep(100);
	sprintf(buf_s,"%s %d", "nvram_set 2860 playStoryNum", playStoryNum);
	system(buf_s);
	usleep(100);
	sprintf(buf_s,"%s %d", "nvram_set 2860 playEnglishNum", playEnglishNum);
	system(buf_s);
	usleep(100);
	//nvram_bufset(RT2860_NVRAM, "VoiceSIZE",buf_s);
}
#endif
//----------------------------音量-------------------------------------
//获取音量
void get_vol_size(unsigned char *size)
{
	char *vol = nvram_bufget(RT2860_NVRAM, "VoiceSIZE");
	*size = (unsigned char)atoi(vol);
}

//设置音量
void set_vol_size(unsigned char size)
{
	char buf_s[64]={0};
	sprintf(buf_s,"%s %d", "nvram_set 2860 VoiceSIZE", size);
	system(buf_s);
	//nvram_bufset(RT2860_NVRAM, "VoiceSIZE",buf_s);
}
#ifdef VOICS_CH
//----------------------------播音人-------------------------------------
//设置声音选择
void set_vol_ch(unsigned char ch)
{
	set_volch(ch);//在线
	char buf_s[64]={0};
	sprintf(buf_s,"%s %d", "nvram_set 2860 VoiceCH", ch);
	system(buf_s);
	//nvram_bufset(RT2860_NVRAM, "VoiceCH",buf_s);
}

//获取声音选择
void get_vol_ch(unsigned char *ch)
{
	char *buf_ch = nvram_bufget(RT2860_NVRAM, "VoiceCH");
	*ch = (unsigned char)atoi(buf_ch);
}
#endif //end VOICS_CH
//----------------------------开关机-------------------------------------
//设置开关机选择
void set_host_time(int type,unsigned char *time)
{
	char buf_s[64]={0};
	if(type==0)
		sprintf(buf_s,"%s %s", "nvram_set 2860 Closetime", time);
	else if (type==1)
		sprintf(buf_s,"%s %s", "nvram_set 2860 Opentime", time);
	system(buf_s);
#if 0
	if(type==0)
		nvram_bufset(RT2860_NVRAM, "Closetime",time);
	else if (type==1)
		nvram_bufset(RT2860_NVRAM, "Opentime",time);
#endif
}

//获取开关机选择
void get_host_time(int type, char *time)
{
	char *buf=NULL;
	if(type==0){
		buf= nvram_bufget(RT2860_NVRAM, "Closetime");
	}
	else if (type==1){
		buf= nvram_bufget(RT2860_NVRAM, "Opentime");
	}
	memcpy(time,buf,strlen(buf));
}
//----------------------------end-------------------------------------
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
