#ifndef _CONFIG_H_
#define _CONFIG_H_
//----------------------�汾��-----------------------------------

#define VERSION "\n10.12-15:00\n"	//�汾��

//#define TEST_SDK					//����SDK

#ifdef TEST_SDK						//ʹ������������
	#define WORK_INTER
#endif

//----------------------������-----------------------------------

//#define SPEEK_VOICES	//�����Խ�����

//#define WAVTOAMR		//��wav��ʽ�ļ�ת����amr��ʽ

#define PALY_URL_SD		//���ر��浽����

#define LOCAL_MP3		//MP3���ز���

#define	LED_LR			//LED���ҵ��Լ��ఴ��

#define UART			//���ڿ���

#define CLOSE_VOICE		//���������ڹر���Ƶ״̬

#define MUTE_TX			//�ر���Ƶ���ùص�����

#define TULIN_WINT_MUSIC//ͼ�������λ����

#define SELECT_UDP		//��udp��ӵ�select ����

//#define VOICS_CH		//������ѡ�������

//----------------------������-----------------------------------

//#define TEXT_UP		//�����ϴ������ӿ�

//#define TEST_MIC		//����¼����ֱ�Ӳ��ų���

//#define TEST_SAVE_MP3	//�������ڱ�������ʶ��֮������������MP3�ı���Ϣ

#define UDP_LOG			//ʹ��udp��ӡ��Ϣ

#define TCP_LOG			//ʹ��tcp��ӡ��Ϣ

#define STRLOG			//ʹ�ܲ�������ӡ��Ϣ

//#define LOG_DELMP3	//ʹ��ɾ��MP3·����ӡ��Ϣ

#define LOG_MP3PLAY		//ʹ��URL��ӡ��Ϣ

//----------------------�û�������-------------------------------

#define UDP_BRO_PORT 		20001						// ���ع㲥�˿�

#define FRIST_SMART_LIST	"smart12345678"
#define FRIST_PASSWD		"12345678"

#define TF_SYS_PATH 		"/media/mmcblk0p1/"			//tf��·��
#define TF_TEST_PATH 		"testmp3/"					//���ز���·��
#ifdef LOCAL_MP3
#ifdef	LED_LR
#define TF_MP3_PATH 		"keji/"						//��������·��
#define TF_STORY_PATH 		"why/"						//���ع���·��
#define TF_ENGLISH_PATH		"english/"					//����Ӣ��·��
#else
#define TF_MP3_PATH 		"mp3/"						//��������·��
#define TF_STORY_PATH 		"story/"					//���ع���·��
#endif
#endif
#ifdef PALY_URL_SD
#define URL_SDPATH			"/home/cache.tmp"			//url����·��
#define MP3_SDPATH			"/media/mmcblk0p1/music/"	//url����·��
#define MP3_PATHLEN			sizeof(MP3_SDPATH)
#endif

#endif
