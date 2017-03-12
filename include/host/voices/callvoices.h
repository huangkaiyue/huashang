#ifndef _CALLVOICES_H
#define _CALLVOICES_H

#include "base/pool.h"
//----------------------��Ƶ�¼�---------------------------------------
#define START_SPEEK_VOICES 		1  	//¼��������ʶ��
#define START_TAIK_MESSAGE		2	//����Ϣ,���͸������豸(app/��������)
#define RECODE_PAUSE 			3 	//¼������
//#define PLAY_STD_VOICES			4	//����ʶ����
//#define PLAY_MP3				5	//����mp3�ļ���
#define PLAY_WAV				6	//����wavԭʼ������
#define END_SPEEK_VOICES		7	//����¼��
#define PLAY_URL				8	//����url����
#define TIME_OUT				9	//�ر�ϵͳ
#define TIME_SIGN				12	//��ʱ�����¼�
#define PLAY_OUT				13	//�ر�ϵͳ
#define SPEEK_WAIT				14	//�Խ��¼�
#define PLAY_TULING				15	//����ͼ��

#define RECODE_STOP 			10  //¼��ֹͣ,�˳�����¼���߳�
#define RECODE_EXIT_FINNISH		11	//¼�������˳�

//#define DBG_VOICES
#ifdef DBG_VOICES
#define DEBUG_VOICES(fmt, args...) printf("Call voices: " fmt, ## args) 
#else
#define DEBUG_VOICES(fmt, args...) {} 
#endif

#define DEBUG_VOICES_ERROR(fmt, args...) printf("Call voices: " fmt, ## args) 

#define STD_RECODE_SIZE	((5*RECODE_RATE*16*1/8)+WAV_HEAD)
#define STD_RECODE_SIZE_16K	2*(5*RECODE_RATE*16*1/8)+WAV_HEAD
//----------------------�¼�����---------------------------------------
#define STUDY_WAV_EVENT 		1		//ѧϰ��
#define SYS_VOICES_EVENT		2		//ϵͳ�����¼�
#define URL_VOICES_EVENT		3		//url�����¼�
#define SET_RATE_EVENT			4		//�޸Ĳ������¼�
#define QTTS_PLAY_EVENT			5		//qtts�¼�
#define LOCAL_MP3_EVENT			7
#define SPEEK_VOICES_EVENT		8		//���յ�������Ϣ	
#define TALK_EVENT_EVENT		9		//�Խ��¼�
#define QUIT_MAIN				10		//�˳�main����
#define TULING_URL_MAIN			11		//ͼ��URL�¼�
#define TULING_URL_VOICES		12		//ͼ��mp3�¼�
#define TEST_PLAY_EQ_WAV		13		//���Բ���wav��Ƶ�ļ�

//----------------------ϵͳ��---------------------------------------
#define END_SYS_VOICES_PLAY			1	//������
#define TULING_WINT_PLAY			2	//���Ե�
#define LOW_BATTERY_PLAY			3	//�͵�ػ�
#define RESET_HOST_V_PLAY			4	//�ָ���������
#define REQUEST_FAILED_PLAY			5	//�������������������ʧ��
#define UPDATA_END_PLAY				6	//���¹̼�����
#define TIMEOUT_PLAY_LOCALFILE		7	//�����������ʱ�����ű����Ѿ�¼�ƺõ���Ƶ
#define CONNET_ING_PLAY				9	//��������
#define CONNECT_OK_PLAY				10	//���ӳɹ�
#define START_SMARTCONFIG_PLAY		11	//��������
#define SMART_CONFIG_OK_PLAY		12	//��������ɹ�
#define NOT_FIND_WIFI_PLAY			13	//û��ɨ�赽wifi
#define SMART_CONFIG_FAILED_PLAY	14	//û���յ��û����͵�wifi
#define NOT_NETWORK_PLAY			18	//����û������������
#define CONNET_CHECK_PLAY			19	//���ڼ�������Ƿ����
#define SEND_OK_PLAY				20	//���ͳɹ�
#define SEND_ERROR_PLAY				21	//����ʧ��
#define SEND_LINK_PLAY				22	//���ڷ���
#define KEY_DOWN_PLAY				23	//���°�����
#define PLAY_ERROT_PLAY				24	//����ʧ��
#define TF_ERROT_PLAY				25	//TF����ʧ��
#define NETWORK_ERROT_PLAY			26	//��������ʧ��
#define WIFI_CHECK_PLAY				27	//���WiFi
#define WIFI_NO						28	//������绷��ʧ��
#define WIFI_YES					29	//������绷���ɹ�
#define LIKE_ERROT_PLAY				30	//��ǰû��ϲ�����ݣ���ȥ�ղ�ϲ�����ݰ�

#define BIND_SSID_PLAY				31	//�ɹ��յ�С���İ�����
#define BIND_OK_PLAY				32	//�ɹ�����С���İ�����
#define SEND_LINK_ER_PLAY			33	//��ǰ���绷�����������ʧ�ܣ��������硣
#define TALK_CONFIRM_PLAY			34	//�ڼ�ô���ڼ�ô�������ڼ�ô������Ҫ��Ϣ֪ͨ��Ӵ���밴�����ظ��ҡ�
#define TALK_CONFIRM_OK_PLAY		35	//ȷ����Ϣ�ظ��ɹ����뷢�ϴ�������
#define TALK_CONFIRM_ER_PLAY		36	//��ǰ��û���˺����㡣
#define DOWNLOAD_ING_PLAY			37	//�������ع̼���
#define DOWNLOAD_ERROE_PLAY			38	//���ع̼�����
#define DOWNLOAD_END_PLAY			39	//���ع̼�������
#define DOWNLOAD_25_PLAY			40	//���ص��ٷ�֮��ʮ�塣
#define DOWNLOAD_50_PLAY			41	//���ص��ٷ�֮��ʮ��
#define DOWNLOAD_75_PLAY			42	//���ص��ٷ�֮��ʮ�塣
#define UPDATA_NEW_PLAY				43	//���°汾����Ҫ���¡�
#define UPDATA_START_PLAY			44	//��ʼ���¹̼���
#define UPDATA_ERROR_PLAY			45	//���¹̼�����



//---------------------------------------------------------
#define VOICES_MIN	13200	//�Ƿ��Ǵ���0.5�����Ƶ��������16000������λ16λ
#define VOICES_ERR	1000	//�󴥷�

#define SEC				1
#define MIN				60*SEC
#define SYSTEMOUTSIGN	5*MIN
#define SYSTEMOUTTIME	15*MIN
#define PLAYOUTTIME		60*MIN
#define ERRORTIME		30*24*60*MIN
//--------------------------------------------------------

#define NETWORK_OK 0		//���������ɹ�
#define NETWORK_ER 1		//����ʧ��
#define NETWORK_UNKOWN	2	//δ֪����״̬


#define NETWORK_ERR_VOICES_1	1		//�������Ӵ��󣬲�������̨����ǩֵ
#define NETWORK_ERR_VOICES_2	2
#define NETWORK_ERR_VOICES_3	3
#define NETWORK_ERR_VOICES_4	4
#define NETWORK_ERR_VOICES_5	5


typedef struct sys_message{
	unsigned char recorde_live;
	unsigned char oldrecorde_live;
	char sd_path[20];
	int Playlocaltime;
	unsigned char localplayname;
	unsigned char netstate;		//���������ⲿ����״̬
}SysMessage;
extern SysMessage sysMes;
//--------------------eventVoices.c----------------------------------------

#define XIMALA_MUSIC_DIRNAME	"ximalaya/"	//ϲ�������ղصĸ���Ŀ¼��		

#define START_SPEEK_E	START_SPEEK_VOICES 		  	//¼��������ʶ��
#define PAUSE_E			RECODE_PAUSE 				//¼������
#define PLAY_WAV_E		PLAY_WAV					//����wavԭʼ������
#define END_SPEEK_E		END_SPEEK_VOICES			//����¼��
#define PLAY_URL_E		PLAY_URL					//����url����
#define START_TAIK_MESSAGE_E START_TAIK_MESSAGE
#define PLAY_TULING_E 	PLAY_TULING

#define DBG_EVENT
#ifdef DBG_EVENT  
#define DEBUG_EVENT(fmt, args...) printf("%s:" ,__func__,fmt, ## args)  
#else   
#define DEBUG_EVENT(fmt, args...) { }  
#endif //end DBG_EVENT

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
extern void start_event_play_wav(int i);
extern void start_event_play_url(void);
extern void pause_record_audio(int i);
extern int GetRecordeLive(void);
#ifdef TIMEOUT_CHECK
extern void exit_handle_event(void);
extern void start_event_talk_message(void);
extern void keep_recorde_live(int change);
#endif
extern int SetSystemTime(unsigned char outtime);

extern void InitRecord_VoicesPthread(void);
extern void ExitRecord_Voicespthread(void);
//--------------------eventVoices.c-----------------------------------------------
extern int createPlayEvent(const void *play,unsigned char Mode);
extern void CleanUrlEvent(void);
extern void Create_PlayQttsEvent(const char *txt,int type);
extern void down_voices_sign(void);
extern void Net_work(void);
extern void Create_PlaySystemEventVoices(int sys_voices);
extern void Handle_PlaySystemEventVoices(int sys_voices);
extern void InitMtkPlatfrom76xx(void);
extern void CleanMtkPlatfrom76xx(void);
extern void Create_WeixinSpeekEvent(unsigned int gpioState);
extern void Handle_WeixinSpeekEvent(unsigned int gpioState);
extern void SaveRecorderVoices(const char *voices_data,int size);

#endif
