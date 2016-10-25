#ifndef _CALLVOICES_H
#define _CALLVOICES_H

#include "base/pool.h"

#define START_SPEEK_VOICES 		1  	//¼��������ʶ��
#define START_TAIK_MESSAGE		2	//����Ϣ,���͸������豸(app/��������)
#define RECODE_PAUSE 			3 	//¼������
#define PLAY_STD_VOICES			4	//����ʶ����
#define PLAY_MP3				5	//����mp3�ļ���
#define PLAY_WAV				6	//����wavԭʼ������
#define END_SPEEK_VOICES		7	//����¼��
#define PLAY_URL				8	//����url����

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

#define STUDY_WAV_EVENT 		1		//ѧϰ��
#define SYS_VOICES_EVENT		2		//ϵͳ�����¼�
#define URL_VOICES_EVENT		3		//url�����¼�
#define SET_RATE_EVENT			4		//�޸Ĳ������¼�
#define QTTS_PLAY_EVENT			5		//qtts�¼�
#define LOCAL_MP3_EVENT			7
#define SPEEK_VOICES_EVENT		8		//���յ�������Ϣ	
#define TALK_EVENT_EVENT		9		//�Խ��¼�

typedef struct sys_message
{
	unsigned char networkstate:1,
		network_timeout:7;			//����״̬  0:û������ 1 ������
	unsigned char recorde_live:4,
		oldrecorde_live:4;
	unsigned char error_400002;	
	char Play_sign:4;//ֹͣ��־λ
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
