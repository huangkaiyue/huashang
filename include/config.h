#ifndef _CONFIG_H_
#define _CONFIG_H_
//----------------------版本类-----------------------------------

//#define TEST_SDK					//测试SDK

//#define WORK_INTER

#ifdef TEST_SDK						//使能命令输入行
#endif

//#define DATOU_JIANG		//蒋总
//#define QITUTU_SHI		//石总
//#define TANGTANG_LUO		//罗总
#define HUASHANG_JIAOYU
//----------------------功能类-----------------------------------

#define SYSTEMLOCK				//测试版本限制开机次数
#define CLOCESYSTEM				//超时退出
//#define CLOSE_VOICE				//不工作处于关闭音频状态
#define SELECT_UDP				//将udp添加到select 当中

#define MY_HTTP_REQ			//ʹ���Լ�д��http ����ӿ�
#define TULING_FILE_LOG		//����ͼ��д����־�ļ�

#if defined(QITUTU_SHI)||defined(DATOU_JIANG)||defined(HUASHANG_JIAOYU)
	#define SPEEK_VOICES	//开启对讲功能
	#define SPEEK_VOICES1	//按键切换会话
	#define PALY_URL_SD		//下载保存到本地
	#define LOCAL_MP3		//MP3本地播放
	#define	LED_LR			//LED左右灯以及多按键
#endif

#ifdef DATOU_JIANG	//大头---蒋总
#endif
#if defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)	//石总---好奇兔
	#define CLOCKTOALIYUN	//阿里云闹钟
#endif

#if defined(HUASHANG_JIAOYU)
	#define  HUASHANG_JIAOYU_PLAY_JSON_FILE		"huashang_play.json"
#endif

//----------------------测试类-----------------------------------

//#define TEST_MIC			//测试录音并直接播放出来

//#define TEST_SAVE_MP3		//测试用于保存语音识别之后，下载下来的MP3文本信息

#define ENABLE_LOG			//使能写入文件log


//----------------------用户数据类-------------------------------

#define UDP_BRO_PORT 		20001						// 本地广播端口

#ifndef TEST_SDK	
	#define TF_SYS_PATH 		"/media/mmcblk0p1/"		//tf卡路径
#else
	#define TF_SYS_PATH 		"/mnt/neirong/"			//tf卡路径
#endif
#ifdef LOCAL_MP3
#ifdef	LED_LR
#if 0
#define TF_MP3_PATH 		"keji/"						//本地音乐路径
#define TF_STORY_PATH 		"why/"						//本地故事路径
#define TF_ENGLISH_PATH		"english/"					//本地英语路径
#else
#define TF_MP3_PATH 		"故事/"						//本地音乐路径（儿歌）
#define TF_STORY_PATH 		"素质/"						//本地故事路径
#define TF_ENGLISH_PATH		"英语/"						//本地英语路径
#define TF_GUOXUE_PATH		"国学/"						//本地国学路径（科技）
#endif
#else
#define TF_MP3_PATH 		"mp3/"						//本地音乐路径
#define TF_STORY_PATH 		"story/"					//本地故事路径
#define TF_ENGLISH_PATH		"english/"					//本地英语路径
#endif
#endif

#define URL_SDPATH				"/home/cache.tmp"		//url缓存路径
#ifdef PALY_URL_SD
#ifdef TEST_SDK	
	#define MP3_SDPATH			"/mnt/neirong/music/"			//url保存路径
	#define MP3_LIKEPATH		"/mnt/neirong/ximalaya/"		//url喜爱路径
#else
	#define MP3_SDPATH			"/media/mmcblk0p1/music/"	//url保存路径
	#define MP3_LIKEPATH		"/media/mmcblk0p1/ximalaya/"//url喜爱路径
#endif
#define MP3_PATHLEN			sizeof(MP3_SDPATH)
#define MP3_LIKEPATHLEN		sizeof(MP3_LIKEPATH)
#endif

#define SYSTEMLOCKNUM	500		//限制次数


//#define DEBUG_SYSTEM_IP					//��������������������wifi ��IP ��ַ����
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


#define XIAI_DIR 				"xiai"
#define HUASHANG_GUOXUE_DIR		"huashangedu"	
//#define TEST_ERROR_TULING

