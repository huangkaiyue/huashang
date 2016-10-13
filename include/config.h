#ifndef _CONFIG_H_
#define _CONFIG_H_
//----------------------版本类-----------------------------------

#define VERSION "\n10.12-15:00\n"	//版本号

//#define TEST_SDK					//测试SDK

#ifdef TEST_SDK						//使能命令输入行
	#define WORK_INTER
#endif

//----------------------功能类-----------------------------------

//#define SPEEK_VOICES	//开启对讲功能

//#define WAVTOAMR		//将wav格式文件转换成amr格式

#define PALY_URL_SD		//下载保存到本地

#define LOCAL_MP3		//MP3本地播放

#define	LED_LR			//LED左右灯以及多按键

#define UART			//串口开关

#define CLOSE_VOICE		//不工作处于关闭音频状态

#define MUTE_TX			//关闭音频设置关掉音量

#define TULIN_WINT_MUSIC//图灵过渡音位音乐

#define SELECT_UDP		//将udp添加到select 当中

//#define VOICS_CH		//播音人选择功能添加

//----------------------测试类-----------------------------------

//#define TEXT_UP		//测试上传语音接口

//#define TEST_MIC		//测试录音并直接播放出来

//#define TEST_SAVE_MP3	//测试用于保存语音识别之后，下载下来的MP3文本信息

#define UDP_LOG			//使能udp打印信息

#define TCP_LOG			//使能tcp打印信息

#define STRLOG			//使能播放器打印信息

//#define LOG_DELMP3	//使能删除MP3路径打印信息

#define LOG_MP3PLAY		//使能URL打印信息

//----------------------用户数据类-------------------------------

#define UDP_BRO_PORT 		20001						// 本地广播端口

#define FRIST_SMART_LIST	"smart12345678"
#define FRIST_PASSWD		"12345678"

#define TF_SYS_PATH 		"/media/mmcblk0p1/"			//tf卡路径
#define TF_TEST_PATH 		"testmp3/"					//本地测试路径
#ifdef LOCAL_MP3
#ifdef	LED_LR
#define TF_MP3_PATH 		"keji/"						//本地音乐路径
#define TF_STORY_PATH 		"why/"						//本地故事路径
#define TF_ENGLISH_PATH		"english/"					//本地英语路径
#else
#define TF_MP3_PATH 		"mp3/"						//本地音乐路径
#define TF_STORY_PATH 		"story/"					//本地故事路径
#endif
#endif
#ifdef PALY_URL_SD
#define URL_SDPATH			"/home/cache.tmp"			//url缓存路径
#define MP3_SDPATH			"/media/mmcblk0p1/music/"	//url保存路径
#define MP3_PATHLEN			sizeof(MP3_SDPATH)
#endif

#endif
