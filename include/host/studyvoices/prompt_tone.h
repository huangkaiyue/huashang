#ifndef	_PROMPT_TONE_H
#define _PROMPT_TONE_H

/*******************************************************************************
�̶��ϳɵ���Ƶ�ļ�
*******************************************************************************/
#if 0
#define ERROR_40002			"qtts/40002_8k.wav"				//����һ��
#define ERROR_40003			"qtts/40003_8k.wav"				//�˺�����
#define ERROR_40004			"qtts/40004_8k.wav"				//�Ի���������
#define ERROR_40005			"qtts/40005_8k.wav"				//��֧������
#define ERROR_40006			"qtts/40006_8k.wav"				//��������������
#define ERROR_40007			"qtts/40007_8k.wav"				//��һЩ

#define NO_NETWORK_VOICES 	"qtts/no_network_8k.wav"		//�������������
#define START_SYS_VOICES	"qtts/start_haha_talk_8k.wav"	//������
#define END_SYS_VOICES		"qtts/end_haha_talk_8k.wav"		//������

#define EXIT_TALK_VOICES	"qtts/exit_haha_talk_8k.wav"	//�˳����밴�ҵı���
#define OPEN_TALK_VOICES	"qtts/open_std_8k.wav"			//������ʲô�Ը�
#define CLOSE_TALK_VOICES	"qtts/close_std_8k.wav"			//����ȥ��Ϣ�ˣ��ݰ�
#define CONNECT_NETWORK_V	"qtts/connect_network_8k.wav"	//�ף�����������·��������ϵ�
#define RESET_HOST_V		"qtts/reset_host_8k.wav"		//�ף��ָ��������õ��У�����ϵ�
#define REQUEST_FAILED		"qtts/request_failed_8k.wav"	//�������������ʧ��

#define CHANGE_NETWORK		"qtts/change_network_8k.wav"	//�������ӣ����Ե�
#define START_INTERNET		"qtts/start_internet_8K.wav"	//��ʼ�������뷢��wifi���Լ�����
#define CHECK_INTERNET		"qtts/check_internet_8K.wav"	//���ڼ�������Ƿ���ã��ɵȴ�������������
#define LINK_SUCCESS		"qtts/link_conect_8k.wav"		//���ӳɹ�
#define ERROR_INTER			"qtts/error_internet_8k.wav"	//�������������������粢��������
#define ERROR_PASSWORD		"qtts/error_password_8k.wav"	//�������������������������
#define NO_WIFI		        "qtts/NO_WIFI_8k.wav"			//�޷�ɨ�赽����wifi,������������
#define NOT_REAVWIFI		"qtts/not_reavwifi_8K.wav"		//û���յ��㷢�͵�wifi�������·���һ��
#define YES_REAVWIFI		"qtts/yes_reavwifi_8K.wav"		//�ɹ��յ��㷢�͵�wifi

#define NO_BADY	      		"qtts/Nobady_8K.wav"			//û���û�����
#define MSG_VOICES			"qtts/MSG_VOICES_8k.wav"		//����

#define TULING_DIDI			"qtts/TuLin_Di_8K.wav"			//��
#define TULING_WINT			"qtts/TuLin_Wint_8K.wav"		//���Ե�
#define TULING_WINTXUNFEI	"qtts/TuLin_WintXunfei_8K.wav"	//��������е��ѣ��������ҵ��ǻ�үү
#define TULING_HAHAXIONG	"qtts/TuLin_Hahaxiong_8K.wav"	//�ҽй����ܣ������ֿɰ��Ĺ�����
#define NO_MUSIC			"qtts/no_music_8K.wav"			//��֧�����ֲ���
#define NO_VOICES			"qtts/no_voices_8K.wav"			//�ϴ�����С��0.5��
#define LOW_BATTERY			"qtts/low_battery_8K.wav"

#define UPDATA_END			"qtts/end_updata_8K.wav"		//���¹̼�����

#else
#define ERROR_40002			"qtts/40002_8k.amr"				//����һ��
#define ERROR_40003			"qtts/40003_8k.amr"				//�˺�����
#define ERROR_40004			"qtts/40004_8k.amr"				//�Ի���������
#define ERROR_40005			"qtts/40005_8k.amr"				//��֧������
#define ERROR_40006			"qtts/40006_8k.amr"				//��������������
#define ERROR_40007			"qtts/40007_8k.amr"				//��һЩ

#define NO_NETWORK_VOICES 	"qtts/no_network_8k.amr"		//�������������
#define START_SYS_VOICES	"qtts/start_haha_talk_8k.amr"	//������
#define END_SYS_VOICES		"qtts/end_haha_talk_8k.amr"		//������

#define EXIT_TALK_VOICES	"qtts/exit_haha_talk_8k.amr"	//�˳����밴�ҵı���
#define OPEN_TALK_VOICES	"qtts/open_std_8k.amr"			//������ʲô�Ը�
#define CLOSE_TALK_VOICES	"qtts/close_std_8k.amr"			//����ȥ��Ϣ�ˣ��ݰ�
#define CONNECT_NETWORK_V	"qtts/connect_network_8k.amr"	//�ף�����������·��������ϵ�
#define RESET_HOST_V		"qtts/reset_host_8k.amr"		//�ף��ָ��������õ��У�����ϵ�
#define REQUEST_FAILED		"qtts/request_failed_8k.amr"	//�������������ʧ��

#define CHANGE_NETWORK		"qtts/change_network_8k.amr"	//�������ӣ����Ե�
#define CONNET_TIME			"qtts/waittime_network_8k.amr"	//����������
#define START_INTERNET		"qtts/start_internet_8K.amr"	//��ʼ�������뷢��wifi���Լ�����
#define CHECK_INTERNET		"qtts/check_internet_8K.amr"	//���ڼ�������Ƿ���ã��ɵȴ�������������
#define CHECK_WIFI			"qtts/check_wifi_8K.amr"		//���WiFi
#define CHECK_WIFI_WAIT		"qtts/check_wifi_wait_8K.amr"	//���WiFi������
#define CHECK_WIFI_NO		"qtts/check_wifi_no_8K.amr"		//���WiFi
#define CHECK_WIFI_YES		"qtts/check_wifi_yes_8K.amr"	//���WiFi
#define LINK_SUCCESS		"qtts/link_conect_8k.amr"		//���ӳɹ�
#define ERROR_INTER			"qtts/error_internet_8k.amr"	//�������������������粢��������
#define ERROR_PASSWORD		"qtts/error_password_8k.amr"	//�������������������������
#define NO_WIFI		        "qtts/NO_WIFI_8k.amr"			//�޷�ɨ�赽����wifi,������������
#define NOT_REAVWIFI		"qtts/not_reavwifi_8K.amr"		//û���յ��㷢�͵�wifi�������·���һ��
#define YES_REAVWIFI		"qtts/yes_reavwifi_8K.amr"		//�ɹ��յ��㷢�͵�wifi

#define NO_BADY	      		"qtts/Nobady_8K.amr"			//û���û�����
#define MSG_VOICES			"qtts/MSG_VOICES_8k.amr"		//����

#define TULING_DIDI			"qtts/TuLin_Di_8K.amr"			//��
#define TULING_WINT			"qtts/TuLin_Wint_8K.amr"		//���Ե�
#define TULING_WINTXUNFEI	"qtts/TuLin_WintXunfei_8K.amr"	//��������е��ѣ��������ҵ��ǻ�үү
#define TULING_HAHAXIONG	"qtts/TuLin_Hahaxiong_8K.amr"	//�ҽй����ܣ������ֿɰ��Ĺ�����
#define LOW_BATTERY			"qtts/low_battery_8K.amr"

#define NO_VOICES			"qtts/no_voices_8K.amr"			//�ϴ�����С��0.5��
#define NO_VOICES_1			"qtts/no_voices_8K_1.amr"		//�ϴ�����С��0.5��
#define NO_VOICES_2			"qtts/no_voices_8K_2.amr"		//�ϴ�����С��0.5��
#define NO_VOICES_3			"qtts/no_voices_8K_3.amr"		//�ϴ�����С��0.5��
#define NO_VOICES_4			"qtts/no_voices_8K_4.amr"		//�ϴ�����С��0.5��
#define NO_VOICES_5			"qtts/no_voices_8K_5.amr"		//�ϴ�����С��0.5��
#define NO_VOICES_6			"qtts/no_voices_8K_6.amr"		//�ϴ�����С��0.5��
#define NO_VOICES_7			"qtts/no_voices_8K_7.amr"		//�ϴ�����С��0.5��
#define NO_VOICES_8			"qtts/no_voices_8K_8.amr"		//�ϴ�����С��0.5��
#define NO_VOICES_9			"qtts/no_voices_8K_9.amr"		//�ϴ�����С��0.5��

#define UPDATA_END			"qtts/end_updata_8K.amr"		//���¹̼�����

#define SEND_OK				"qtts/send_ok_8K.amr"			//���ͳɹ�
#define SEND_ERROR			"qtts/send_error_8K.amr"		//����ʧ��
#define SEND_LINK			"qtts/send_link_8K.amr"			//���ڷ���

#define KEY_VOICE_DOWN		"qtts/key_down_8K.amr"			//
#define KEY_VOICE_UP		"qtts/key_up_8K.amr"			//

#define PLAY_ERROR			"qtts/play_error_8K.amr"		//����ʧ��
#define LIKE_ERROR			"qtts/like_error_8K.amr"		//��ǰû��ϲ�����ݣ���ȥ�ղ�ϲ�����ݰ�
#define TF_ERROR			"qtts/tf_error_8K.amr"			//tf������ʧ��

#define NETWORK_ERROR_1		"qtts/network_error_8K_1.amr"		//��������ʧ��
#define NETWORK_ERROR_2		"qtts/network_error_8K_2.amr"		//��������ʧ��
#define NETWORK_ERROR_3		"qtts/network_error_8K_3.amr"		//��������ʧ��
#define NETWORK_ERROR_4		"qtts/network_error_8K_4.amr"		//��������ʧ��
#define NETWORK_ERROR_5		"qtts/network_error_8K_5.amr"		//��������ʧ��
//=============================================================================
#define NO_STORY			"qtts/no_story_8K.amr"				//С���ѣ��һ�û�й��£��Ͻ��ղع��£����ҽ����°ɡ�
#define NO_MUSIC			"qtts/no_music_8K.amr"				//С���ѣ��һ����ᳪ�裬�Ͻ��ղظ��������ҳ���ɡ�
#define VOICE_ADD			"qtts/voice_add_8K.amr"				//���������óɹ���
#define VOICE_SUB			"qtts/voice_sub_8K.amr"				//���������óɹ���
#define SPEEK_WARNING		"qtts/speek_warning_8K.amr"			//С���ѿ��������棬����˵������ɡ�
#define WELCOME_PLAY		"qtts/welcome_play_8K.amr"			//С�������ǽ����ϴ����ݼ������ɣ���ʼ���š�

#define BIND_SSID			"qtts/bind_ssid_8K.amr"				//�ɹ��յ�С���İ�����
#define BIND_OK				"qtts/bind_ok_8K.amr"				//�ɹ�����С���İ�����
#define SEND_LINK_ER		"qtts/send_linkerror_8K.amr"		//��ǰ���绷�����������ʧ�ܣ��������硣
#define TALK_CONFIRM		"qtts/talk_confirm_8K.amr"			//�ڼ�ô���ڼ�ô�������ڼ�ô������Ҫ��Ϣ֪ͨ��Ӵ���밴�����ظ��ҡ�
#define TALK_CONFIRM_OK		"qtts/talk_confirm_ok_8K.amr"		//ȷ����Ϣ�ظ��ɹ����뷢�ϴ�������
#define TALK_CONFIRM_ER		"qtts/talk_confirm_er_8K.amr"		//��ǰ��û���˺����㡣
#define DOWNLOAD_ING		"qtts/download_ing_8K.amr"			//�������ع̼���
#define DOWNLOAD_ERROE		"qtts/download_error_8K.amr"		//���ع̼�����
#define DOWNLOAD_END		"qtts/download_end_8K.amr"			//���ع̼�������
#define DOWNLOAD_25			"qtts/download_25_8K.amr"			//���ص��ٷ�֮��ʮ�塣
#define DOWNLOAD_50			"qtts/download_50_8K.amr"			//���ص��ٷ�֮��ʮ��
#define DOWNLOAD_75			"qtts/download_75_8K.amr"			//���ص��ٷ�֮��ʮ�塣
#define UPDATA_NEW			"qtts/updata_new_8K.amr"			//���°汾����Ҫ���¡�
#define UPDATA_START		"qtts/updata_start_8K.amr"			//��ʼ���¹̼���
#define UPDATA_ERROR		"qtts/updata_error_8K.amr"			//���¹̼�����
#endif
/*******************************************************************************
��ʱ��Ƶ�ļ�·��
*******************************************************************************/
#define CACHE_CHAR_TO_VOICES		"/home/char.wav"				//app�˷��͹�����Ҫ��ʱת���������·��
#define VOICES_QTTS_PATH_SINGLE		"/home/tts_voices_single.pcm"  	//��ȡqtts����������
#define QTTS_TULING_TEXT_PATH		"/home/qtts.wav"				//qtts ����ת������ʱ����ļ�				
#define QTTS_LONG_TEXT_PATH			"/home/qlist"					//qtts ����ת������ʱ����ļ�				
#define SAVE_WAV_VOICES_DATA		"/home/send_file.wav"
#define AMR_WAV_FILE				"/home/amr_to_wav.wav"
#define SINGLE_TO_DOUBLE			"/home/double.wav"

/*******************************************************************************
����app�˷��͹�������Ƶ�ļ� ·��
*******************************************************************************/
//#define CACHE_WAV_PATH		"/cache/wav/"
#define CACHE_WAV_PATH		"/upload/"

#endif

