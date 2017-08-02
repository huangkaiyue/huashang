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


#define CHANGE_NETWORK		"qtts/change_network_8k.amr"	//正在连接，请稍等
#define CONNET_TIME			"qtts/waittime_network_8k.amr"	//联网过渡音
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

#define TULING_WINTXUNFEI	"qtts/TuLin_WintXunfei_8K.amr"	//这个问题有点难，我在问我的智慧爷爷
#define TULING_HAHAXIONG	"qtts/TuLin_Hahaxiong_8K.amr"	//我叫哈哈熊，聪明又可爱的哈哈熊





#define SEND_LINK			"qtts/send_link_8K.amr"			//正在发送



#define WELCOME_PLAY		"qtts/welcome_play_8K.amr"			//来自开普勒星球的小培老师正在检查网络
#define BIND_OK				"qtts/bind_ok_8K.amr"				//成功处理小伙伴的绑定请求。
#define SEND_LINK_ER		"qtts/send_linkerror_8K.amr"		//当前网络环境差，语音发送失败，请检查网络。
#define TALK_CONFIRM		"qtts/talk_confirm_8K.amr"			//在家么，在家么，有人在家么，有重要消息通知你哟，请按按键回复我。
#define TALK_CONFIRM_OK		"qtts/talk_confirm_ok_8K.amr"		//确认消息回复成功，请发上传语音。
#define TALK_CONFIRM_ER		"qtts/talk_confirm_er_8K.amr"		//当前还没有人呼叫你。


#define PLAY_CONTINUE_MUSIC			"qtts/please_playmusic.amr"	//请继续点播吧

#define REBOOT_SYSTEM				"qtts/reboot_system.amr"	//系统正在优化，重新启动，请稍后

#define AMR_9_START_PLAY			"qtts/9.amr"		//9、我的大脑程序正在启动中，请耐心等待一下吧！
#define AMR_10_START_PLAY			"qtts/10.amr"		//10、我的大脑程序正在开机中，请耐心等待一下吧！
#define AMR_11_START_SYSTEM_OK		"qtts/11.amr"		//11、启动成功	

#define AMR_12_NOT_NETWORK			"qtts/12.amr"		//12、小朋友你可以让爸爸妈妈帮我连接网络，我才会更聪明哦！
#define AMR_13_NOT_NETWORK			"qtts/13.amr"		//13、小培老师与华上总部课堂失去联系，情况十万火急，请按wifi配网键帮助小培老师与华上总部课堂取得联系吧！
#define AMR_14_NOT_NETWORK			"qtts/14.amr"		//14、当前没有网络，请帮我连接网络吧！
#define AMR_15_START_CONFIG			"qtts/15.amr"		//15、开始配网，请发送wifi名以及密码！
#define AMR_16_CONNET_ING			"qtts/16.amr"		//16、正在尝试连接网络，请稍等！
#define AMR_17_NETWORK_1			"qtts/17.amr"		//17、被强大的网络包围
#define AMR_18_CONNET_ING			"qtts/18.amr"		//18、小培老师正在努力与总部课堂连接中，请稍等！
#define AMR_19_CONNET_ING			"qtts/19.amr"		//19、网络正在连接中，请耐心等一会儿吧！
#define AMR_20_CONNET_OK			"qtts/20.amr"		//20、小培老师与总部课堂连接成功，我们来聊天吧！


#define AMR_21_NOT_SCAN_WIFI		"qtts/21.amr"		//21、无法扫描到您的wifi,请检查您的网络
#define AMR_22_NOT_RECV_WIFI		"qtts/22.amr"		//22、没有收到你发送的wifi,请重新发送一遍
#define AMR_23_CHECK_NETWORk		"qtts/23.amr"		//23、正在检查网络是否可用，请等待，或重新配网。（注：开机过程离wifi远、或者到新的环境，出现连接不上）
#define AMR_24_NEW_MESSAGE			"qtts/24.amr"		//24、你有新消息，请按信息键听取吧！
#define AMR_25_NEW_STROY			"qtts/25.amr"		//25、你有新故事未听取,按信息键开始听吧！

#define AMR_26_BIND					"qtts/26.amr"		//26、小朋友请让爸爸在微信界面当中邀请小伙伴一起来聊天吧！
#define AMR_27_RECV_BIND			"qtts/27.amr"		//27、成功收到小伙伴的绑定请求。
#define AMR_28_BIND_OK				"qtts/28.amr"		//28、成功处理小伙伴的绑定请求。

#define AMR_29_NETWORK_FAILED		"qtts/29.amr"		//29、当前网络环境差，语音发送失败，请检查网络！


#define AMR_30_AI_FAILED			"qtts/30.amr"		//30、不好意思，我没听清楚您说什么，请再说一遍！
#define AMR_31_AI_FAILED			"qtts/31.amr"		//31、你在跟我说悄悄话吗？我没听清楚，能再说一遍吗？
#define AMR_32_AI_FAILED			"qtts/32.amr"		//32、这个可难倒我了，我们换个话题吧！
#define AMR_33_AI_FAILED			"qtts/33.amr"		//33、长按我头顶上的按键有提示音时我才能听到你说什么呢！
#define AMR_34_AI_FAILED			"qtts/34.amr"		//34、请长按头顶按键听到提示音后才跟我对话

#define AMR_35_AI_STROY_0			"qtts/35.amr"		//35、我给你读首诗吧！
#define AMR_36_AI_STROY_1			"qtts/36.amr"		//36、我给你讲个笑话吧！
#define AMR_37_AI_STROY_2			"qtts/37.amr"		//37、我给你讲个故事吧！
#define AMR_38_AI_STROY_3			"qtts/38.amr"		//38、我给你讲讲百科知识吧。
#define AMR_39_AI_STROY_4			"qtts/39.amr"		//39、按栏目键然后再按左右键切换你想要听的本地内容吧！


#define AMR_40_NOT_USR				"qtts/40.amr"		//40、小朋友，你去哪里了，请跟我一起来玩吧！！
#define AMR_41_LISTEN_MUSIC			"qtts/41.amr"		//41、要不我们今天听一下歌曲！
#define AMR_42_LISTEN_GUOXUE		"qtts/42.amr"		//42、小朋友，要不我们今天学习一下国学内容吧，我的国学知识很丰富的!
#define AMR_43_LISTEN_CHENGYU		"qtts/43.amr"		//43、要不我们今天学习一下成语!
#define AMR_44_WEIXIN_WARN			"qtts/44.amr"		//44、小伙伴,在家想爸爸妈妈吗？快用我给爸爸妈妈发微聊吧!按住我的微聊键发语音然后松开，爸爸妈妈就能马上收到你的思念了！


#define AMR_45_SLEEP_1				"qtts/45.amr"		//45、亲我先去休息了，当你想我的时候，记得叫醒我喔!
#define AMR_46_SLEEP_2				"qtts/46.amr"		//46、今天就先陪你到这里，我们先休息一下再继续吧。
#define AMR_47_SLEEP_3				"qtts/47.amr"		//47、没人理我，我先去总部学习去了,当你想问我问题的时候再叫醒我吧。

#define AMR_48_NOT_SCARD			"qtts/48.amr"		//48、没有读到本地内容，请联系总部!

#define AMR_49_POWER_LOW			"qtts/49.amr"		//49、我饿了，请帮我充电吧!
#define AMR_50_POWER_OFF_1			"qtts/50.amr"		//50、电池电量过低，即将关机。
#define AMR_51_POWER_OFF_1			"qtts/51.amr"		//51、饿晕了，没能量了，即将关机。
#define AMR_52_POWER_AC				"qtts/52.amr"		//52、正在补充能量
#define AMR_53_POWER_FULL			"qtts/53.amr"		//53、能量补充完毕
#define AMR_54_POWER_DISCONNET		"qtts/54.amr"		//54、能量补充断开

#define AMR_55_NEW_VERSION			"qtts/55.amr"		//55、发现新程序版本，正在升级，请不要关机。
#define AMR_56_RESET				"qtts/56.amr"		//56、亲，我已经恢复到最初状态，正在重新启动。

#define AMR_57_recv_message			"qtts/57.amr"		//57、请稍等，正在接受总部信息。

#define AMR_58_WHO_NAME				"qtts/58.amr"		//58、我是小培老师开心又可爱的智能教育机器人。
#define AMR_59_WHO_NAME				"qtts/59.amr"		//59、我是小培老师，你的好朋友啊！
#define AMR_60_AGE					"qtts/60.amr"		//60、我比你大一点

#define AMR_61_DIR					"qtts/61.amr"		
#define AMR_62_DIR					"qtts/62.amr"		
#define AMR_63_DIR					"qtts/63.amr"		
#define AMR_64_DIR					"qtts/64.amr"
#define AMR_65_DIR					"qtts/65.amr"		
#define AMR_66_DIR					"qtts/66.amr"
#define AMR_67_DIR					"qtts/67.amr"		
#define AMR_68_DIR					"qtts/68.amr"
#define AMR_69_DIR					"qtts/69.amr"		
#define AMR_70_DIR					"qtts/70.amr"
#define AMR_71_DIR					"qtts/71.amr"
#define AMR_72_DIR					"qtts/72.amr"
#define AMR_73_DIR					"qtts/73.amr"
#define AMR_74_DIR					"qtts/74.amr"
#define AMR_75_DIR					"qtts/75.amr"


#define AMR_ADD_VOL					"qtts/add_vol.amr"		//64、加大音量
#define AMR_SUB_VOL					"qtts/sub_vol.amr"		//65、减小音量



#define AMR_WEIXIN_SEND_ERROR		"qtts/weixin_send_error.amr"	//消息弄丢了呜呜!
#define AMR_UPDATE_OK				"qtts/update_ok_8K.amr"			//更新固件结束
#define TIMEOUT_baike				"qtts/timeOut_baike.amr"		//上知天文，下知地理，我给你讲百科知识吧!
#define TIMEOUT_sleep				"qtts/timeOut_sleep.amr"		//小朋友今天已经很晚了，早睡早起做一个乖宝宝，我给你放睡眠曲吧!

#define TULING_KEYUP				"qtts/tuling_keyup.amr"		//
#define TULING_WINT					"qtts/TuLin_Wint_8K.amr"		//请稍等  
#define AMR_WEIXIN_SEND_OK			"qtts/weixin_send_ok.amr"
#define AMR_WEIXIN_RECV_OK			"qtts/weixin_recv_ok.amr"

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

