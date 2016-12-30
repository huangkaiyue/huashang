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

//----------------------ϵͳ��---------------------------------------
#define END_SYS_VOICES_PLAY			1	//������
#define TULING_WINT_PLAY			2	//���Ե�
#define LOW_BATTERY_PLAY			3	//�͵�ػ�
#define RESET_HOST_V_PLAY			4	//�ָ���������
#define REQUEST_FAILED_PLAY			5	//�������������������ʧ��
#define UPDATA_END_PLAY				6	//���¹̼�����
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

#define NETWORK_OK 0	//�����ɹ�
#define NETWORK_ER 1	//����ʧ��
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
typedef struct sys_message{
	unsigned char recorde_live;
	unsigned char oldrecorde_live;
	char Play_sign:4;//ֹͣ��־λ
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
