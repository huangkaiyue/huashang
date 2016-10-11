#ifndef	_PROMPT_TONE_H
#define _PROMPT_TONE_H

/*******************************************************************************
固定合成的音频文件
*******************************************************************************/
#define ERROR_40002			"qtts/40002_8k.wav"				//靠近一点
#define ERROR_40003			"qtts/40003_8k.wav"				//账号有误
#define ERROR_40004			"qtts/40004_8k.wav"				//对话次数用完
#define ERROR_40005			"qtts/40005_8k.wav"				//不支持问题
#define ERROR_40006			"qtts/40006_8k.wav"				//服务器正在升级
#define ERROR_40007			"qtts/40007_8k.wav"				//近一些

#define NO_NETWORK_VOICES 	"qtts/no_network_8k.wav"		//请帮我连接网络
#define START_SYS_VOICES	"qtts/start_haha_talk_8k.wav"	//启动音
#define END_SYS_VOICES		"qtts/end_haha_talk_8k.wav"		//结束音

#define EXIT_TALK_VOICES	"qtts/exit_haha_talk_8k.wav"	//退出，请按我的鼻子
#define OPEN_TALK_VOICES	"qtts/open_std_8k.wav"			//宝贝有什么吩咐
#define CLOSE_TALK_VOICES	"qtts/close_std_8k.wav"			//我先去休息了，拜拜
#define CONNECT_NETWORK_V	"qtts/connect_network_8k.wav"	//亲，正在连接网路当中请勿断电
#define RESET_HOST_V		"qtts/reset_host_8k.wav"		//亲，恢复出厂设置当中，请勿断电
#define REQUEST_FAILED		"qtts/request_failed_8k.wav"	//请求服务器数据失败

#define CHANGE_NETWORK		"qtts/change_network_8k.wav"	//正在连接，请稍等
#define START_INTERNET		"qtts/start_internet_8K.wav"	//开始配网，请发送wifi名以及密码
#define CHECK_INTERNET		"qtts/check_internet_8K.wav"	//正在检查网络是否可用，可等待，或重新配网
#define LINK_SUCCESS		"qtts/link_conect_8k.wav"		//连接成功
#define ERROR_INTER			"qtts/error_internet_8k.wav"	//您的网络有误，请检查网络并重新连接
#define ERROR_PASSWORD		"qtts/error_password_8k.wav"	//您输入的密码有误，请重新输入
#define NO_WIFI		        "qtts/NO_WIFI_8k.wav"			//无法扫描到您的wifi,请检查您的网络
#define NOT_REAVWIFI		"qtts/not_reavwifi_8K.wav"		//没有收到你发送的wifi，请重新发送一遍
#define YES_REAVWIFI		"qtts/yes_reavwifi_8K.wav"		//成功收到你发送的wifi

#define NO_BADY	      		"qtts/Nobady_8K.wav"			//没有用户在线
#define MSG_VOICES			"qtts/MSG_VOICES_8k.wav"		//叮咚

#define TULING_DIDI			"qtts/TuLin_Di_8K.wav"			//叮
#define TULING_WINT			"qtts/TuLin_Wint_8K.wav"		//请稍等
#define TULING_WINTXUNFEI	"qtts/TuLin_WintXunfei_8K.wav"	//这个问题有点难，我在问我的智慧爷爷
#define TULING_HAHAXIONG	"qtts/TuLin_Hahaxiong_8K.wav"	//我叫哈哈熊，聪明又可爱的哈哈熊
#define NO_MUSIC			"qtts/no_music_8K.wav"			//不支持音乐播放
#define NO_VOICES			"qtts/no_voices_8K.wav"			//上传数据小于0.5秒
#define LOW_BATTERY			"qtts/low_battery_8K.wav"

#define UPDATA_END			"qtts/end_updata_8K.wav"		//更新固件结束

/*******************************************************************************
零时音频文件路径
*******************************************************************************/
#define CACHE_CHAR_TO_VOICES		"/home/char.wav"				//app端发送过来需要零时转换输出汉字路径
#define VOICES_QTTS_PATH_SINGLE		"/home/tts_voices_single.pcm"  	//获取qtts单声道数据
#define QTTS_TULING_TEXT_PATH		"/home/qtts.wav"				//qtts 汉字转语音零时输出文件				
#define QTTS_LONG_TEXT_PATH			"/home/qlist"					//qtts 汉字转语音零时输出文件				
#define SAVE_WAV_VOICES_DATA		"/home/send_file.wav"
#define AMR_WAV_FILE				"/home/amr_to_wav.wav"
#define SINGLE_TO_DOUBLE			"/home/double.wav"

/*******************************************************************************
保存app端发送过来的音频文件 路径
*******************************************************************************/
//#define CACHE_WAV_PATH		"/cache/wav/"
#define CACHE_WAV_PATH		"/home/"

#endif

