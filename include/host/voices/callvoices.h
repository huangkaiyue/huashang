#ifndef _CALLVOICES_H
#define _CALLVOICES_H

#include "base/pool.h"

#define START_SPEEK_VOICES 		1  	//录音、语音识别
#define START_TAIK_MESSAGE		2	//短消息,发送给其它设备(app/其它主机)
#define RECODE_PAUSE 			3 	//录音挂起
#define PLAY_STD_VOICES			4	//播放识别音
#define PLAY_MP3				5	//播放mp3文件音
#define PLAY_WAV				6	//播放wav原始数据音
#define END_SPEEK_VOICES		7	//结束录音
#define PLAY_URL				8	//播放url数据

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

#define STUDY_WAV_EVENT 		1		//学习音
#define SYS_VOICES_EVENT		2		//系统声音事件
#define URL_VOICES_EVENT		3		//url播放事件
#define SET_RATE_EVENT			4		//修改采样率事件
#define QTTS_PLAY_EVENT			5		//qtts事件
#define LOCAL_MP3_EVENT			7
#define SPEEK_VOICES_EVENT		8		//接收到语音消息	
#define TALK_EVENT_EVENT		9		//对讲事件

typedef struct sys_message
{
	unsigned char networkstate:1,
		network_timeout:7;			//网络状态  0:没有网络 1 有网络
	unsigned char recorde_live:4,
		oldrecorde_live:4;
	unsigned char error_400002;	
	char Play_sign:4;//停止标志位
	char sd_path[20];
	int Starttime;
	unsigned char localplayname;
}SysMessage;
extern SysMessage sysMes;
//--------------------eventVoices.c----------------------------------------

static enum{
	mp3=1,
	story,
	english,
	testmp3,
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
extern void init_record_pthread(void);
extern void exit_record_pthread(void);
//--------------------eventVoices.c-----------------------------------------------
extern void createPlayEvent(const void *play,unsigned char Mode);
extern void CleanUrlEvent(void);
extern void QttsPlayEvent(char *txt,int type);
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
