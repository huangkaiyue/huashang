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
#define RECODE_PAUSE 			3 	//录音挂起
#define PLAY_WAV				4	//播放wav原始数据音
#define END_SPEEK_VOICES		5	//结束录音
#define PLAY_MP3_MUSIC			6	//播放音乐数据
#define TIME_SIGN				8	//长时间无事件
#define SPEEK_WAIT				9	//对讲事件
#define PLAY_DING_VOICES		10	//播放过渡音
#define RECODE_STOP 			11  //录音停止,退出整个录音线程
#define RECODE_EXIT_FINNISH		12	//录音正常退出
#define SOUND_MIX_PLAY			13	//混音播放
#define HUASHANG_SLEEP			14	//华上睡眠状态
#define HUASHANG_SLEEP_OK		15	//华上睡眠状态成功

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
#define TULING_URL_VOICES		12		//图灵mp3事件
#define WEIXIN_DOWN_MP3_EVENT	13		//微信端下载歌曲事件

//----------------------系统音---------------------------------------
#define END_SYS_VOICES_PLAY			1	//结束音
#define TULING_WINT_PLAY			2	//请稍等
#define LOW_BATTERY_PLAY			3	//低电关机
#define RESET_HOST_V_PLAY			4	//恢复出厂设置
#define REQUEST_FAILED_PLAY			5	//重连，请求服务器数据失败
#define UPDATA_END_PLAY				6	//更新固件结束
#define TIMEOUT_PLAY_LOCALFILE		7	//请求服务器超时，播放本地已经录制好的音频
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

#define LIKE_ERROT_PLAY				30	//当前没有喜爱内容，快去收藏喜爱内容吧

#define BIND_SSID_PLAY				31	//成功收到小伙伴的绑定请求。
#define BIND_OK_PLAY				32	//成功处理小伙伴的绑定请求。
#define SEND_LINK_ER_PLAY			33	//当前网络环境差，语音发送失败，请检查网络。
#define TALK_CONFIRM_PLAY			34	//在家么，在家么，有人在家么，有重要消息通知你哟，请按按键回复我。
#define TALK_CONFIRM_OK_PLAY		35	//确认消息回复成功，请发上传语音。
#define TALK_CONFIRM_ER_PLAY		36	//当前还没有人呼叫你。

#ifdef DOWN_IMAGE
#define DOWNLOAD_ING_PLAY			37	//正在下载固件。
#define DOWNLOAD_ERROE_PLAY			38	//下载固件错误。
#define DOWNLOAD_END_PLAY			39	//下载固件结束。
#define DOWNLOAD_25_PLAY			40	//下载到百分之二十五。
#define DOWNLOAD_50_PLAY			41	//下载到百分之五十。
#define DOWNLOAD_75_PLAY			42	//下载到百分之七十五。
#define UPDATA_NEW_PLAY				43	//有新版本，需要更新。
#define UPDATA_START_PLAY			44	//开始更新固件。
#define UPDATA_ERROR_PLAY			45	//更新固件错误。
#endif

#define AI_KEY_TALK_ERROR			46	//智能会话按键误触发，播放系统音
#define MIN_10_NOT_USER_WARN		47	//10分钟不用提示用户
#define TULING_WAIT_VOICES			48	//播放图灵系统等待音
#define CONTINUE_PLAY_MUSIC_VOICES	49	//请继续点播吧

#define HUASHANG_SLEEP_VOICES 		50	//华上睡眠提示音
#define HUASHANG_START_10_VOICES 	51	//华上开机提示语音

//---------------------------------------------------------
#define VOICES_MIN	13200	//是否是大于0.5秒的音频，采样率16000、量化位16位
#define VOICES_ERR	2000	//误触发

#define SEC								1
#define MIN								60*SEC
#define SYSTEMOUTSIGN					1*MIN	//60s
#define TIME_OUT_NOT_USER_FOR_CLOSE		6*MIN	//8分钟不用，就自动关机
#define ERRORTIME						30*24*60*MIN

#define LONG_TIME_NOT_USER_MUTE_VOICES	15		//15s不用 mute音频

#define START_UPLOAD	1
#define END_UPLOAD		0

#define FREE_VOICE_NUMS	3
typedef struct{
	unsigned char recorde_live;
	unsigned char uploadState;
#if defined(HUASHANG_JIAOYU)
	unsigned char freeVoicesNum;
#endif	
	unsigned char closeTime;
	unsigned char WaitSleep;
	int len_voices;
	unsigned int CurrentuploadEventNums;
	char buf_voices[STD_RECODE_SIZE];
}RecoderVoices_t;
//--------------------------------------------------------

#define NETWORK_OK 0		//连接外网成功
#define NETWORK_ER 1		//联网失败
#define NETWORK_UNKOWN	2	//未知网络状态

#define ENABLE_CHECK_VOICES_PLAY	1	//使能检查过程当中播放--->断网添加播放系统语音事件
#define DISABLE_CHECK_VOICES_PLAY	0	//关闭检查网络状态播放--->断网添加播放系统语音事件

#define PLAY_MUSIC_NETWORK			1	//播放歌曲为网络播放类型	
#define PLAY_MUSIC_SDCARD			2	//播放歌曲为sdcard 存储歌曲

typedef struct{
	unsigned char wifiState;
	unsigned char localplayname;		//当前播放本地歌曲目录
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

static enum{
	networkUrl=1,
	mp3,
	story,
	english,
	guoxue,
	xiai,
	huashang,
};

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
extern void LongNetKeyDown_ForConfigWifi(void);

#define	EXTERN_PLAY_EVENT	1	//外部产生邋播放事件
#define AUTO_PLAY_EVENT		2	//内部自身产生播放事件

extern int __AddLocalMp3ForPaly(const char *localpath,unsigned char EventSource);
extern int GetSdcardMusicNameforPlay(unsigned char MuiscMenu,const char *MusicDir, unsigned char playMode);	
extern void Create_CleanUrlEvent(void);
extern void Create_PlayQttsEvent(const char *txt,int type);
extern void TulingKeyDownSingal(void);
extern void UartEventcallFuntion(int event);
extern void Create_PlaySystemEventVoices(int sys_voices);
extern void Handle_PlaySystemEventVoices(int sys_voices,unsigned int playEventNums);
extern void InitMtkPlatfrom76xx(void);
extern void CleanMtkPlatfrom76xx(void);
extern void Create_WeixinSpeekEvent(unsigned int gpioState);
extern void Handle_WeixinSpeekEvent(unsigned int gpioState,unsigned int playEventNums);
extern void SaveRecorderVoices(const char *voices_data,int size);



//--------------------message_wav.c-----------------------------------------------

#define PLAY_IS_COMPLETE		1		//完整播放   ---->适用在智能会话过渡音当中，不允许打断
#define PLAY_IS_INTERRUPT		2		//可以打断播放

extern int __playResamplePlayPcmFile(const char *pcmFile,unsigned int playEventNums);
extern int PlayWeixin_SpeekAmrFileVoices(const char *filename,unsigned int playEventNums,unsigned char mixMode);
extern int PlaySystemAmrVoices(const char *filePath,unsigned int playEventNums);
extern void PlayImportVoices(const char *filepath,unsigned int playEventNums);

#endif
