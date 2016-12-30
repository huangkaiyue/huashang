#ifndef _CALLVOICES_H
#define _CALLVOICES_H

#include "base/pool.h"
//----------------------音频事件---------------------------------------
#define START_SPEEK_VOICES 		1  	//录音、语音识别
#define START_TAIK_MESSAGE		2	//短消息,发送给其它设备(app/其它主机)
#define RECODE_PAUSE 			3 	//录音挂起
//#define PLAY_STD_VOICES			4	//播放识别音
//#define PLAY_MP3				5	//播放mp3文件音
#define PLAY_WAV				6	//播放wav原始数据音
#define END_SPEEK_VOICES		7	//结束录音
#define PLAY_URL				8	//播放url数据
#define TIME_OUT				9	//关闭系统
#define TIME_SIGN				12	//长时间无事件
#define PLAY_OUT				13	//关闭系统
#define SPEEK_WAIT				14	//对讲事件
#define PLAY_TULING				15	//播放图灵

#define RECODE_STOP 			10  //录音停止,退出整个录音线程
#define RECODE_EXIT_FINNISH		11	//录音正常退出

//#define DBG_VOICES
#ifdef DBG_VOICES
#define DEBUG_VOICES(fmt, args...) printf("Call voices: " fmt, ## args) 
#else
#define DEBUG_VOICES(fmt, args...) {} 
#endif

#define DEBUG_VOICES_ERROR(fmt, args...) printf("Call voices: " fmt, ## args) 

#define STD_RECODE_SIZE	((5*RECODE_RATE*16*1/8)+WAV_HEAD)
//----------------------事件类型---------------------------------------
#define STUDY_WAV_EVENT 		1		//学习音
#define SYS_VOICES_EVENT		2		//系统声音事件
#define URL_VOICES_EVENT		3		//url播放事件
#define SET_RATE_EVENT			4		//修改采样率事件
#define QTTS_PLAY_EVENT			5		//qtts事件
#define LOCAL_MP3_EVENT			7
#define SPEEK_VOICES_EVENT		8		//接收到语音消息	
#define TALK_EVENT_EVENT		9		//对讲事件
#define QUIT_MAIN				10		//退出main函数
#define TULING_URL_MAIN			11		//图灵URL事件

//----------------------系统音---------------------------------------
#define END_SYS_VOICES_PLAY			1	//结束音
#define TULING_WINT_PLAY			2	//请稍等
#define LOW_BATTERY_PLAY			3	//低电关机
#define RESET_HOST_V_PLAY			4	//恢复出厂设置
#define REQUEST_FAILED_PLAY			5	//重连，请求服务器数据失败
#define UPDATA_END_PLAY				6	//更新固件结束
#define CONNET_ING_PLAY				9	//正在连接
#define CONNECT_OK_PLAY				10	//连接成功
#define START_SMARTCONFIG_PLAY		11	//启动配网
#define SMART_CONFIG_OK_PLAY		12	//接受密码成功
#define NOT_FIND_WIFI_PLAY			13	//没有扫描到wifi
#define SMART_CONFIG_FAILED_PLAY	14	//没有收到用户发送的wifi
#define NOT_NETWORK_PLAY			18	//板子没有连接上网络
#define CONNET_CHECK_PLAY			19	//正在检查网络是否可用
#define SEND_OK_PLAY				20	//发送成功
#define SEND_ERROR_PLAY				21	//发送失败
#define SEND_LINK_PLAY				22	//正在发送
#define KEY_DOWN_PLAY				23	//按下按键音
#define PLAY_ERROT_PLAY				24	//播放失败
#define TF_ERROT_PLAY				25	//TF加载失败
#define NETWORK_ERROT_PLAY			26	//网络连接失败
#define WIFI_CHECK_PLAY				27	//检查WiFi
#define WIFI_NO						28	//检查网络环境失败
#define WIFI_YES					29	//检查网络环境成功
#define LIKE_ERROT_PLAY				30	//当前没有喜爱内容，快去收藏喜爱内容吧

#define NETWORK_OK 0	//联网成功
#define NETWORK_ER 1	//联网失败
//---------------------------------------------------------
#define VOICES_MIN	13200	//是否是大于0.5秒的音频，采样率16000、量化位16位
#define VOICES_ERR	1000	//误触发

#define SEC				1
#define MIN				60*SEC
#define SYSTEMOUTSIGN	5*MIN
#define SYSTEMOUTTIME	15*MIN
#define PLAYOUTTIME		60*MIN
#define ERRORTIME		30*24*60*MIN
//--------------------------------------------------------
typedef struct sys_message{
	unsigned char recorde_live;
	unsigned char oldrecorde_live;
	char Play_sign:4;//停止标志位
	char sd_path[20];
	int Starttime;
	int Playlocaltime;
	unsigned char localplayname;
	unsigned char network_live;
}SysMessage;
extern SysMessage sysMes;
//--------------------eventVoices.c----------------------------------------

static enum{
	mp3=1,
	story,
	english,
	guoxue,
	xiai,
};

//--------------------callvoices.c-----------------------------------------
extern void start_event_std(void);
extern void end_event_std(void);
extern void start_event_play_wav(void);
extern void start_event_play_url(void);
extern void pause_record_audio(void);
extern int GetRecordeLive(void);
#ifdef TIMEOUT_CHECK
extern void exit_handle_event(void);
extern void start_event_talk_message(void);
extern void keep_recorde_live(int change);
#endif
extern int SetSystemTime(unsigned char outtime);

extern void init_record_pthread(void);
extern void exit_record_pthread(void);
//--------------------eventVoices.c-----------------------------------------------
extern void setNetWorkLive(unsigned char live);
extern int createPlayEvent(const void *play,unsigned char Mode);
extern void CleanUrlEvent(void);
extern void QttsPlayEvent(const char *txt,int type);
extern void down_voices_sign(void);
extern void Net_work(void);
extern void create_event_system_voices(int sys_voices);
extern void handle_event_system_voices(int sys_voices);
extern void init_wm8960_voices(void);
extern void clean_main_voices(void);
extern void handle_voices_key_event(unsigned int state);
extern void create_event_voices_key(unsigned int state);
extern void save_recorder_voices(const char *voices_data,int size);

#endif
