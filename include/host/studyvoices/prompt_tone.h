#ifndef	_PROMPT_TONE_H
#define _PROMPT_TONE_H

#include "config.h"

/*******************************************************************************
固定合成的音频文件
*******************************************************************************/
#define NO_NETWORK_VOICES 	"qtts/no_network_8k.amr"		//请帮我连接网络
#define START_SYS_VOICES	"qtts/start_haha_talk_8k.amr"	//启动音
#define END_SYS_VOICES		"qtts/end_haha_talk_8k.amr"		//结束音

#define RESET_HOST_V		"qtts/reset_host_8k.amr"		//亲，恢复出厂设置当中，请勿断电
#define REQUEST_FAILED		"qtts/request_failed_8k.amr"	//请求服务器数据失败

#define CHANGE_NETWORK		"qtts/change_network_8k.amr"	//正在连接，请稍等
#define CONNET_TIME			"qtts/waittime_network_8k.amr"	//联网过渡音
#define START_INTERNET		"qtts/start_internet_8K.amr"	//开始配网，请发送wifi名以及密码
#define CHECK_INTERNET		"qtts/check_internet_8K.amr"	//正在检查网络是否可用，可等待，或重新配网

#define CHECK_WIFI			"qtts/check_wifi_8K.amr"		//检查WiFi
#define CHECK_WIFI_WAIT		"qtts/check_wifi_wait_8K.amr"	//检查WiFi过渡音
#define CHECK_WIFI_NO		"qtts/check_wifi_no_8K.amr"		//检查WiFi
#define CHECK_WIFI_YES		"qtts/check_wifi_yes_8K.amr"	//检查WiFi

#define LINK_SUCCESS		"qtts/link_conect_8k.amr"		//连接成功,我们来聊天吧!!
#define ERROR_INTER			"qtts/error_internet_8k.amr"	//您的网络有误，请检查网络并重新连接
#define ERROR_PASSWORD		"qtts/error_password_8k.amr"	//您输入的密码有误，请重新输入
#define NO_WIFI		        "qtts/NO_WIFI_8k.amr"			//无法扫描到您的wifi,请检查您的网络
#define NOT_REAVWIFI		"qtts/not_reavwifi_8K.amr"		//没有收到你发送的wifi，请重新发送一遍
#define YES_REAVWIFI		"qtts/yes_reavwifi_8K.amr"		//成功收到你发送的wifi

#define TULING_DIDI			"qtts/TuLin_Di_8K.amr"			//叮
#define TULING_WINT			"qtts/TuLin_Wint_8K.amr"		//请稍等  
#define TULING_WINTXUNFEI	"qtts/TuLin_WintXunfei_8K.amr"	//这个问题有点难，我在问我的智慧爷爷
#define TULING_HAHAXIONG	"qtts/TuLin_Hahaxiong_8K.amr"	//我叫哈哈熊，聪明又可爱的哈哈熊
#define LOW_BATTERY			"qtts/low_battery_8K.amr"

#define NO_VOICES			"qtts/no_voices_8K.amr"			//上传数据小于0.5秒
#define NO_VOICES_1			"qtts/no_voices_8K_1.amr"		//上传数据小于0.5秒
#define NO_VOICES_2			"qtts/no_voices_8K_2.amr"		//上传数据小于0.5秒
#define NO_VOICES_3			"qtts/no_voices_8K_3.amr"		//上传数据小于0.5秒
#define NO_VOICES_4			"qtts/no_voices_8K_4.amr"		//上传数据小于0.5秒
#define NO_VOICES_5			"qtts/no_voices_8K_5.amr"		//上传数据小于0.5秒
#define NO_VOICES_6			"qtts/no_voices_8K_6.amr"		//上传数据小于0.5秒
#define NO_VOICES_7			"qtts/no_voices_8K_7.amr"		//上传数据小于0.5秒
#define NO_VOICES_8			"qtts/no_voices_8K_8.amr"		//上传数据小于0.5秒
#define NO_VOICES_9			"qtts/no_voices_8K_9.amr"		//上传数据小于0.5秒

#define UPDATA_END			"qtts/end_updata_8K.amr"		//更新固件结束

#define SEND_OK				"qtts/send_ok_8K.amr"			//发送成功
#define SEND_ERROR			"qtts/send_error_8K.amr"		//发送失败
#define SEND_LINK			"qtts/send_link_8K.amr"			//正在发送

#define KEY_VOICE_DOWN		"qtts/key_down_8K.amr"			//
#define KEY_VOICE_UP		"qtts/key_up_8K.amr"			//

#define PLAY_ERROR			"qtts/play_error_8K.amr"		//播放失败
#define LIKE_ERROR			"qtts/like_error_8K.amr"		//当前没有喜爱内容，快去收藏喜爱内容吧
#define TF_ERROR			"qtts/tf_error_8K.amr"			//tf卡加载失败

#define NETWORK_ERROR_1		"qtts/network_error_8K_1.amr"		//网络连接失败
#define NETWORK_ERROR_2		"qtts/network_error_8K_2.amr"		//网络连接失败
#define NETWORK_ERROR_3		"qtts/network_error_8K_3.amr"		//网络连接失败
#define NETWORK_ERROR_4		"qtts/network_error_8K_4.amr"		//网络连接失败
#define NETWORK_ERROR_5		"qtts/network_error_8K_5.amr"		//网络连接失败
//=============================================================================
#define VOICE_ADD			"qtts/voice_add_8K.amr"				//音量加设置成功。
#define VOICE_SUB			"qtts/voice_sub_8K.amr"				//音量减设置成功。
#define SPEEK_WARNING		"qtts/speek_warning_8K.amr"			//小朋友，你去哪里了，和我一起来玩吧!!
#define WELCOME_PLAY		"qtts/welcome_play_8K.amr"			//来自开普勒星球的小培老师正在检查网络
#define TIMEOUT_music		"qtts/timeOut_music.amr"			//要不我们今天听一下歌曲
#define TIMEOUT_guoxue		"qtts/timeOut_guoxue.amr"			//要不我们今天学习一下国学内容吧，我的国学知识很丰富的!
#define TIMEOUT_chengyu		"qtts/timeOut_chengyu.amr"			//要不我们今天学习一下成语
#define TIMEOUT_baike		"qtts/timeOut_baike.amr"			//上知天文，下知地理，我给你讲百科知识吧!
#define TIMEOUT_sleep		"qtts/timeOut_sleep.amr"			//小朋友今天已经很晚了，早睡早起做一个乖宝宝，我给你放睡眠曲吧!

#define BIND_SSID			"qtts/bind_ssid_8K.amr"				//成功收到小伙伴的绑定请求。
#define BIND_OK				"qtts/bind_ok_8K.amr"				//成功处理小伙伴的绑定请求。
#define SEND_LINK_ER		"qtts/send_linkerror_8K.amr"		//当前网络环境差，语音发送失败，请检查网络。
#define TALK_CONFIRM		"qtts/talk_confirm_8K.amr"			//在家么，在家么，有人在家么，有重要消息通知你哟，请按按键回复我。
#define TALK_CONFIRM_OK		"qtts/talk_confirm_ok_8K.amr"		//确认消息回复成功，请发上传语音。
#define TALK_CONFIRM_ER		"qtts/talk_confirm_er_8K.amr"		//当前还没有人呼叫你。

#ifdef DOWN_IMAGE
#define DOWNLOAD_ING		"qtts/download_ing_8K.amr"			//正在下载固件。
#define DOWNLOAD_ERROE		"qtts/download_error_8K.amr"		//下载固件错误。
#define DOWNLOAD_END		"qtts/download_end_8K.amr"			//下载固件结束。
#define DOWNLOAD_25			"qtts/download_25_8K.amr"			//下载到百分之二十五。
#define DOWNLOAD_50			"qtts/download_50_8K.amr"			//下载到百分之五十。
#define DOWNLOAD_75			"qtts/download_75_8K.amr"			//下载到百分之七十五。
#define UPDATA_NEW			"qtts/updata_new_8K.amr"			//有新版本，需要更新。
#define UPDATA_START		"qtts/updata_start_8K.amr"			//开始更新固件。
#define UPDATA_ERROR		"qtts/updata_error_8K.amr"			//更新固件错误。
#endif
#define PLAY_CONTINUE_MUSIC	"qtts/please_playmusic.amr"			//请继续点播吧

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
#define CACHE_WAV_PATH		"/upload/"

#endif

