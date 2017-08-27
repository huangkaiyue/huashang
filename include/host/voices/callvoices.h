#ifndef _CALLVOICES_H
#define _CALLVOICES_H

#include "base/pool.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/WavAmrCon.h"
#include "config.h"
#define KB	1024


//----------------------音频事件---------------------------------------
#define START_SPEEK_VOICES 		1  	//录音、语音识别
#define START_TAIK_MESSAGE		2	//短消息,发送给其它设备(app/其它主机)
#define RECODE_PAUSE 				3 	//录音挂起
#define PLAY_WAV					4	//播放wav原始数据音
#define END_SPEEK_VOICES			5	//结束录音
#define PLAY_MP3_MUSIC				6	//播放音乐数据
#define TIME_SIGN					8	//长时间无事件
#define SPEEK_WAIT					9	//对讲事件
#define PLAY_DING_VOICES			10	//播放过渡音
#define RECODE_STOP 				11  //录音停止,退出整个录音线程
#define RECODE_EXIT_FINNISH		12	//录音正常退出
#define SOUND_MIX_PLAY				13	//混音播放
#define HUASHANG_SLEEP				14	//华上睡眠状态
#define HUASHANG_SLEEP_OK			15	//华上睡眠状态成功
#define HUASHANG_CLOSE_SYSTEM	16	//华上睡眠状态成功
//#define DBG_VOICES
#ifdef DBG_VOICES
#define DEBUG_VOICES(fmt, args...) printf("Call voices: " fmt, ## args) 
#else
#define DEBUG_VOICES(fmt, args...) {} 
#endif

#define DEBUG_VOICES_ERROR(fmt, args...) printf("Call voices: " fmt, ## args) 

#define TIME_RECODE_S	8		

#define STD_RECODE_SIZE	((TIME_RECODE_S*RECODE_RATE*16*1/8)+WAV_HEAD)

//----------------------事件类型---------------------------------------
#define STUDY_WAV_EVENT 				1		//学习音
#define SYS_VOICES_EVENT				2		//系统声音事件
#define URL_VOICES_EVENT				3		//url播放事件
#define SET_RATE_EVENT					4		//修改采样率事件
#define QTTS_PLAY_EVENT				5		//qtts事件
#define LOCAL_MP3_EVENT				7
#define SPEEK_VOICES_EVENT				8		//接收到语音消息	
#define TALK_EVENT_EVENT				9		//对讲事件
#define QUIT_MAIN						10		//退出main函数
#define TULING_URL_MAIN				11		//图灵URL事件
#define TULING_URL_VOICES				12		//图灵mp3事件
#define DIR_MENU_PLAY_EVENT			13		
//----------------------系统音---------------------------------------


/*
#define NOT_FIND_WIFI_PLAY				13	//没有扫描到wifi
#define NOT_NETWORK_PLAY				18	//板子没有连接上网络
#define SEND_OK_PLAY						20	//发送成功
#define SEND_ERROR_PLAY					21	//发送失败
#define SEND_LINK_PLAY					22	//正在发送
*/



#define MIN_10_NOT_USER_WARN			47	//10分钟不用提示用户
#define TULING_WAIT_VOICES				148	//播放图灵系统等待音
#define CONTINUE_PLAY_MUSIC_VOICES	149	//请继续点播吧
#define RECV_WEIXIN_MESSAGE_VOICES	150	//播放图灵系统等待音

#define CMD_11_START					11	

#define CMD_12_NOT_NETWORK			12	//网络连接失败
#define CMD_15_START_CONFIG			15	//15、开始配网，请发送wifi名以及密码！
#define CMD_16_CONNET_ING				16	//16、正在尝试连接网络，请稍等！
#define CMD_18_CONNET_ING				18	//18、小培老师正在努力与总部课堂连接中，请稍等！
#define CMD_19_CONNET_ING				19	//19、网络正在连接中，请耐心等一会儿吧！
#define CMD_20_CONNET_OK				20	//20、(8634代号)小培老师与总部课堂连接成功，我们来聊天吧！（每次连接成功的语音，包括唤醒）

#define CMD_21_NOT_SCAN_WIFI			21	//21、无法扫描到您的wifi,请检查您的网络
#define CMD_22_NOT_RECV_WIFI			22	//22、没有收到你发送的wifi,请重新发送一遍
#define CMD_23_NOT_WIFI				23	//23、正在检查网络是否可用，请等待，或重新配网。（注：开机过程离wifi远、或者到新的环境，出现连接不上）

#define CMD_24_WAKEUP_RECV_MSG		24	//24、你有新消息，请按信息键听取吧！（唤醒之后播放，播放网络成功之后）
#define CMD_25_WAKEUP_RECV_MSG		25	//25、你有新故事未听取,按信息键开始听吧！（唤醒之后播放，播放网络成功之后）

#define CMD_26_BIND_PLAY				26	//26、小朋友请让爸爸在微信界面当中邀请小伙伴一起来聊天吧！
#define CMD_27_RECV_BIND				27	//27、成功收到小伙伴的绑定请求。
#define CMD_28_HANDLE_BIND			28	//28、成功处理小伙伴的绑定请求。

#define CMD_29_NETWORK_FAILED		29	//29、当前网络环境差，语音发送失败，请检查网络！
#define CMD_35_39_REQUEST_FAILED		35	//35、我给你读首诗吧！--->42、按栏目键然后再按左右键切换你想要听的本地内容吧！

#define CMD_39_REQUEST_FAILED			39	//39


#define CMD_40_NOT_USER_WARN 		40	//40、小朋友，你去哪里了，请跟我一起来玩吧！！
#define CMD_44_WEIXIN_WARN			44
#define CMD_4547_SLEEP 					45	//45、亲我先去休息了，当你想我的时候，记得叫醒我喔!

#define CMD_48_TF_ERROT_PLAY			48	//48、没有读到本地内容，请联系总部!
#define CMD_4951_POWER_LOW			49	//49、我饿了，请帮我充电吧!


#define CMD_52_POWER_AC				52	//52、正在补充能量
#define CMD_53_POWER_FULL				53	//53、能量补充完毕
#define CMD_54_POWER_INTERRUPT		54	//54、能量补充断开

#define CMD_55_NEW_VERSION			55	//55、发现新程序版本，正在升级，请不要关机。
#define CMD_56_RESET_SYSTEM			56	//56、亲，我已经恢复到最初状态，正在重新启动。


#define CMD_6175_DIR_MENU				61
#define CMD_62_DIR_MENU				62
#define CMD_63_DIR_MENU				63
#define CMD_64_DIR_MENU				64
#define CMD_65_DIR_MENU				65
#define CMD_66_DIR_MENU				66
#define CMD_67_DIR_MENU				67
#define CMD_68_DIR_MENU				68
#define CMD_69_DIR_MENU				69
#define CMD_70_DIR_MENU				70
#define CMD_71_DIR_MENU				71
#define CMD_72_DIR_MENU				72
#define CMD_73_DIR_MENU				73
#define CMD_74_DIR_MENU				74
#define CMD_75_DIR_MENU				75

#define CMD_83_SMART_NOT_WIFI		83


#define CMD_90_UPDATE_OK				90	//90.更新固件结束

#define CMD_100_CANLE_WIFI				100
#define CMD_110_NOT_NETWORK			110	// restart check network
#define CMD_111_NOTWIFI_PLAYMUSIC	111
//---------------------------------------------------------
#define VOICES_MIN	13200	//是否是大于0.5秒的音频，采样率16000、量化位16位
#define VOICES_ERR	2000	//误触发

#define SEC								1
#define MIN								60*SEC
#define SYSTEMOUTSIGN					1*MIN	//60s
#define TIME_OUT_NOT_USER_FOR_CLOSE		6*MIN	//8分钟不用，就自动关机
#define ERRORTIME						30*24*60*MIN

#define LONG_TIME_NOT_USER_MUTE_VOICES	15		//15s不用 mute音频

#define CLOSE_SYSTEM_TIME				10*MIN		//long time not user system

#define START_UPLOAD	1
#define END_UPLOAD		0

#define SYSTEM_INIT		0			//系统处于初始状态
#define SYSTEM_SLEEP	4			//系统进入睡眠状态

#define TIME_OUT_CHECK_LOCK		1
#define TIME_OUT_CHECK_UNLOCK	0

#define FREE_VOICE_NUMS	3
typedef struct{
	unsigned char lockTimeOutcheck;
	unsigned char recorde_live;
	unsigned char uploadState;
	unsigned char freeVoicesNum;	
	unsigned char closeTime;
	unsigned char WaitSleep;
	int len_voices;
	unsigned int CurrentuploadEventNums;
	char buf_voices[STD_RECODE_SIZE];
}RecoderVoices_t;
//--------------------------------------------------------
#define NETWORK_OK 0					//连接外网成功
#define NETWORK_ER 1					//联网失败
#define NETWORK_UNKOWN	2				//未知网络状态
#define NETWORK_RESTART	3				//

#define ENABLE_CHECK_VOICES_PLAY	1	//使能检查过程当中播放--->断网添加播放系统语音事件
#define DISABLE_CHECK_VOICES_PLAY	0	//关闭检查网络状态播放--->断网添加播放系统语音事件

#define PLAY_MUSIC_NETWORK			1	//播放歌曲为网络播放类型	
#define PLAY_MUSIC_SDCARD			2	//播放歌曲为sdcard 存储歌曲

#define RESTART_NETWORK_LOCK		1
#define RESTART_NETWORK_UNLOCK		0

typedef struct{
	unsigned char startCheckNetworkFlag;
	unsigned char lockRestartNetwork;
	unsigned char wifiState;
	unsigned char netstate;				//板子连接外部网络状态
	char localVoicesPath[20];			//板子系统音存放路径
}SysMessage;
extern SysMessage sysMes;
//--------------------eventVoices.c----------------------------------------
#define DBG_EVENT
#ifdef DBG_EVENT  
#define DEBUG_EVENT(fmt, args...) printf("%s:"fmt,__func__, ## args)  
#else   
#define DEBUG_EVENT(fmt, args...) { }  
#endif //end DBG_EVENT

//--------------------callvoices.c-----------------------------------------
extern unsigned int GetCurrentEventNums(void);
extern unsigned int updateCurrentEventNums(void);	//更新当前事件编号，并返回事件编号值
extern void StartTuling_RecordeVoices(void);
extern void StopTuling_RecordeVoices(void);
extern void start_event_play_wav(void);
extern void start_event_play_Mp3music(void);
extern void start_event_play_soundMix(void);
extern void pause_record_audio(void);
extern int GetRecordeVoices_PthreadState(void);
extern void start_event_talk_message(void);
extern int SetMucClose_Time(unsigned char closeTime);
extern void InitRecord_VoicesPthread(void);
extern void ExitRecord_Voicespthread(void);
//--------------------eventVoices.c-----------------------------------------------

typedef struct{
	unsigned char freeVoiceNums;
	int file_len;
	FILE *savefilefp;
	int Starttime;	//录制微信对讲起始时间，用来检查文件录制长度，防止录制太短的音频
	pthread_mutex_t mutex;
}Speek_t;

extern void ShortKeyDown_ForPlayWifiMessage(void);
extern int LongNetKeyDown_ForConfigWifi(void);

#define	EXTERN_PLAY_EVENT	1	//外部产生邋播放事件
#define AUTO_PLAY_EVENT		2	//内部自身产生播放事件

extern int __AddLocalMp3ForPaly(const char *localpath,unsigned char EventSource);
extern void Create_CleanUrlEvent(void);
extern void Create_PlayQttsEvent(const char *txt,int type);
extern void TulingKeyDownSingal(void);
extern void UartEventcallFuntion(int event);
extern void Create_PlaySystemEventVoices(int sys_voices);
extern void *updateHuashangFacePthread(void *arg);
extern void Handle_PlaySystemEventVoices(int sys_voices,unsigned int playEventNums);
extern void InitMtkPlatfrom76xx(void);
extern void CleanMtkPlatfrom76xx(void);
extern void Create_WeixinSpeekEvent(unsigned int gpioState);
extern void Handle_WeixinSpeekEvent(unsigned int gpioState,unsigned int playEventNums);
extern void SaveRecorderVoices(const char *voices_data,int size);

//--------------------message_wav.c-----------------------------------------------
#define PLAY_IS_COMPLETE		1		//完整播放   ---->适用在智能会话过渡音当中，不允许打断
#define PLAY_IS_INTERRUPT		2		//可以打断播放
#define SHOW_FACE			1		//
#define NOT_FACE			0		//
extern int __playResamplePlayPcmFile(const char *pcmFile,unsigned int playEventNums);
extern int PlayWeixin_SpeekAmrFileVoices(const char *filename,unsigned int playEventNums,unsigned char mixMode);
extern int PlaySystemAmrVoices(const char *filePath,unsigned int playEventNums);
extern void PlayImportVoices(const char *filepath,unsigned int playEventNums);
extern void playVoicesNotFace(const char *filePath,unsigned int playEventNums);

#endif
