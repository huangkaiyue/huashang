#ifndef _CONFIG_H_
#define _CONFIG_H_
//----------------------版本类-----------------------------------
//#define WORK_INTER

#define HUASHANG_JIAOYU
//----------------------功能类-----------------------------------

#define MY_HTTP_REQ			//ʹ���Լ�д��http ����ӿ�
#define TULING_FILE_LOG		//����ͼ��д����־�ļ�
#define LOCAL_MP3		//MP3本地播放

#define  HUASHANG_JIAOYU_PLAY_JSON_FILE		"huashang_play.json"

//----------------------测试类-----------------------------------

//#define TEST_MIC			//测试录音并直接播放出来


#define ENABLE_LOG			//使能写入文件log


//----------------------用户数据类-------------------------------
	
#define TF_SYS_PATH 		"/media/mmcblk0p1/"		//tf卡路径


#define URL_SDPATH			"/home/cache.tmp"		//url缓存路径
#define MP3_SDPATH			"/media/mmcblk0p1/"	//url保存路径
#define MP3_LIKEPATH		"/media/mmcblk0p1/ximalaya/"//url喜爱路径
//#define DEBUG_PLAY_SYSTEM_IP			//��������������������wifi ��IP ��ַ����
//#define TEST_PLAY_EQ_MUSIC			//������Ч		
//#define PCM_TEST						//���Ա���pcm�ļ�


#define TULING_PLAY_TEXT_WEIXIN_FAILED	"С���Ѱ�ʧ�ܣ���������΢�Ž�������Ҫ�󶨵��豸�š�"


#define NET_SERVER_FILE_LOCK			"/var/server.lock"		//�������������ļ���

#define INTEN_NETWORK_FILE_LOCK			"/var/internet.lock"	//�����ļ��������������̵��У�ɨ������ӹ��̵�����һ�������ļ������ã���ֹ��߽���������״̬���������̽���ɨ�������

#define SMART_CONFIG_FILE_LOCK			"/var/SmartConfig.lock"	//�����ļ���,��ֹ�ڿ��������Զ����������ͻ

#define LOCAL_SERVER_FILE_LOCK			"/var/localserver.lock"	//���ط������ļ���

#define ENABLE_RECV_NETWORK_FILE_LOCK	"/var/startNet.lock"	//ʹ�ܽ����ļ���

#define CLOSE_SYSTEM_LOCK_FILE			"/var/close_system.lock"
#endif

#define XIMALA_MUSIC_DIRNAME	"ximalaya/"	//ϲ�������ղصĸ���Ŀ¼��		
#define HUASHANG_GUOXUE_DIR		"huashangedu"	

#define DEFALUT_URL_JSON		"/home/defalutUrl.json"
#define DEFALUT_HUASHANG_JSON 	"/home/huashang_server.json"
#define DEFALUT_DIR_MENU_JSON 	"/home/huashang_dir.json"


#define PLAY_NEXT		1		//������һ�׸���
#define PLAY_PREV		2 		//������һ�׸���
#define PLAY_RANDOM		3		//�������


//#define TEST_ERROR_TULING

