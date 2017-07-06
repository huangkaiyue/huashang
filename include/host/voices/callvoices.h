#ifndef _CALLVOICES_H
#define _CALLVOICES_H

#include "base/pool.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/WavAmrCon.h"
#include "config.h"
#define KB	1024


//----------------------��Ƶ�¼�---------------------------------------
#define START_SPEEK_VOICES 		1  	//¼��������ʶ��
#define START_TAIK_MESSAGE		2	//����Ϣ,���͸������豸(app/��������)
#define RECODE_PAUSE 			3 	//¼������
#define PLAY_WAV				4	//����wavԭʼ������
#define END_SPEEK_VOICES		5	//����¼��
#define PLAY_MP3_MUSIC			6	//������������
#define TIME_SIGN				8	//��ʱ�����¼�
#define SPEEK_WAIT				9	//�Խ��¼�
#define PLAY_DING_VOICES		10	//���Ź�����
#define RECODE_STOP 			11  //¼��ֹͣ,�˳�����¼���߳�
#define RECODE_EXIT_FINNISH		12	//¼�������˳�
#define SOUND_MIX_PLAY			13	//��������
#define HUASHANG_SLEEP			14	//����˯��״̬
#define HUASHANG_SLEEP_OK		15	//����˯��״̬�ɹ�

//#define DBG_VOICES
#ifdef DBG_VOICES
#define DEBUG_VOICES(fmt, args...) printf("Call voices: " fmt, ## args) 
#else
#define DEBUG_VOICES(fmt, args...) {} 
#endif

#define DEBUG_VOICES_ERROR(fmt, args...) printf("Call voices: " fmt, ## args) 

#define TIME_RECODE_S	8		

#define STD_RECODE_SIZE	((TIME_RECODE_S*RECODE_RATE*16*1/8)+WAV_HEAD)

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
#define WEIXIN_DOWN_MP3_EVENT	13		//΢�Ŷ����ظ����¼�

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

#define LIKE_ERROT_PLAY				30	//��ǰû��ϲ�����ݣ���ȥ�ղ�ϲ�����ݰ�

#define BIND_SSID_PLAY				31	//�ɹ��յ�С���İ�����
#define BIND_OK_PLAY				32	//�ɹ�����С���İ�����
#define SEND_LINK_ER_PLAY			33	//��ǰ���绷�����������ʧ�ܣ��������硣
#define TALK_CONFIRM_PLAY			34	//�ڼ�ô���ڼ�ô�������ڼ�ô������Ҫ��Ϣ֪ͨ��Ӵ���밴�����ظ��ҡ�
#define TALK_CONFIRM_OK_PLAY		35	//ȷ����Ϣ�ظ��ɹ����뷢�ϴ�������
#define TALK_CONFIRM_ER_PLAY		36	//��ǰ��û���˺����㡣

#ifdef DOWN_IMAGE
#define DOWNLOAD_ING_PLAY			37	//�������ع̼���
#define DOWNLOAD_ERROE_PLAY			38	//���ع̼�����
#define DOWNLOAD_END_PLAY			39	//���ع̼�������
#define DOWNLOAD_25_PLAY			40	//���ص��ٷ�֮��ʮ�塣
#define DOWNLOAD_50_PLAY			41	//���ص��ٷ�֮��ʮ��
#define DOWNLOAD_75_PLAY			42	//���ص��ٷ�֮��ʮ�塣
#define UPDATA_NEW_PLAY				43	//���°汾����Ҫ���¡�
#define UPDATA_START_PLAY			44	//��ʼ���¹̼���
#define UPDATA_ERROR_PLAY			45	//���¹̼�����
#endif

#define AI_KEY_TALK_ERROR			46	//���ܻỰ�����󴥷�������ϵͳ��
#define MIN_10_NOT_USER_WARN		47	//10���Ӳ�����ʾ�û�
#define TULING_WAIT_VOICES			48	//����ͼ��ϵͳ�ȴ���
#define CONTINUE_PLAY_MUSIC_VOICES	49	//������㲥��

#define HUASHANG_SLEEP_VOICES 		50	//����˯����ʾ��
#define HUASHANG_START_10_VOICES 	51	//���Ͽ�����ʾ����

//---------------------------------------------------------
#define VOICES_MIN	13200	//�Ƿ��Ǵ���0.5�����Ƶ��������16000������λ16λ
#define VOICES_ERR	2000	//�󴥷�

#define SEC								1
#define MIN								60*SEC
#define SYSTEMOUTSIGN					1*MIN	//60s
#define TIME_OUT_NOT_USER_FOR_CLOSE		6*MIN	//8���Ӳ��ã����Զ��ػ�
#define ERRORTIME						30*24*60*MIN

#define LONG_TIME_NOT_USER_MUTE_VOICES	15		//15s���� mute��Ƶ

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

#define NETWORK_OK 0		//���������ɹ�
#define NETWORK_ER 1		//����ʧ��
#define NETWORK_UNKOWN	2	//δ֪����״̬

#define ENABLE_CHECK_VOICES_PLAY	1	//ʹ�ܼ����̵��в���--->������Ӳ���ϵͳ�����¼�
#define DISABLE_CHECK_VOICES_PLAY	0	//�رռ������״̬����--->������Ӳ���ϵͳ�����¼�

#define PLAY_MUSIC_NETWORK			1	//���Ÿ���Ϊ���粥������	
#define PLAY_MUSIC_SDCARD			2	//���Ÿ���Ϊsdcard �洢����

typedef struct{
	unsigned char wifiState;
	unsigned char localplayname;		//��ǰ���ű��ظ���Ŀ¼
	unsigned char netstate;				//���������ⲿ����״̬
	char localVoicesPath[20];			//����ϵͳ�����·��
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
extern unsigned int updateCurrentEventNums(void);	//���µ�ǰ�¼���ţ��������¼����ֵ
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
	int Starttime;	//¼��΢�ŶԽ���ʼʱ�䣬��������ļ�¼�Ƴ��ȣ���ֹ¼��̫�̵���Ƶ
	pthread_mutex_t mutex;
}Speek_t;

extern void ShortKeyDown_ForPlayWifiMessage(void);
extern void LongNetKeyDown_ForConfigWifi(void);

#define	EXTERN_PLAY_EVENT	1	//�ⲿ�����岥���¼�
#define AUTO_PLAY_EVENT		2	//�ڲ�������������¼�

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

#define PLAY_IS_COMPLETE		1		//��������   ---->���������ܻỰ���������У���������
#define PLAY_IS_INTERRUPT		2		//���Դ�ϲ���

extern int __playResamplePlayPcmFile(const char *pcmFile,unsigned int playEventNums);
extern int PlayWeixin_SpeekAmrFileVoices(const char *filename,unsigned int playEventNums,unsigned char mixMode);
extern int PlaySystemAmrVoices(const char *filePath,unsigned int playEventNums);
extern void PlayImportVoices(const char *filepath,unsigned int playEventNums);

#endif
