#ifndef _CONFIG_H_
#define _CONFIG_H_
//----------------------版本类-----------------------------------

#define VERSION "\n12.11-15:00\n"	//版本号

//#define TEST_SDK					//测试SDK

//#define TEST_PLAY_EQ_MUSIC			//������Ч		


#ifdef TEST_SDK						//使能命令输入行
	#define WORK_INTER
#endif

//#define CONFIG_ALL		//全功能
//#define DATOU_JIANG		//蒋总
#define QITUTU_SHI		//石总
//#define TANGTANG_LUO		//罗总

//----------------------功能类-----------------------------------

#define SYSTEMLOCK				//测试版本限制开机次数
#define CLOCESYSTEM				//超时退出
//#define CLOSE_VOICE				//不工作处于关闭音频状态
#define SELECT_UDP				//将udp添加到select 当中
#define SYSVOICE

#define PCM_DATA				//上传pcm数据
//#define AMR16K_DATA				//上传amr数据

#define MY_HTTP_REQ			//ʹ���Լ�д��http ����ӿ�
#define TULING_FILE_LOG		//����ͼ��д����־�ļ�


#ifdef DATOU_JIANG	//大头---蒋总
	#define SPEEK_VOICES	//开启对讲功能
	
	#define SPEEK_VOICES1	//按键切换会话
	
	#define PALY_URL_SD		//下载保存到本地

	#define LOCAL_MP3		//MP3本地播放

	#define	LED_LR			//LED左右灯以及多按键

	//#define VOICS_CH		//播音人选择功能添加
#endif
#ifdef QITUTU_SHI	//石总---好奇兔
	#define CLOCKTOALIYUN	//阿里云闹钟
	
	#define SPEEK_VOICES	//开启对讲功能

	#define SPEEK_VOICES1	//按键切换会话

	#define PALY_URL_SD		//下载保存到本地

	#define LOCAL_MP3		//MP3本地播放

	#define	LED_LR			//LED左右灯以及多按键

	//#define VOICS_CH		//播音人选择功能添加
#endif
#ifdef CONFIG_ALL	//全功能
	#define CLOCKTOALIYUN	//阿里云闹钟
	
	#define SPEEK_VOICES	//开启对讲功能
//=====================================================
	#define SPEEK_VOICES1	//按键切换会话
	
	#define PALY_URL_SD		//下载保存到本地

	#define LOCAL_MP3		//MP3本地播放

	#define	LED_LR			//LED左右灯以及多按键
//=====================================================
	//#define VOICS_CH		//播音人选择功能添加
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
#ifdef PALY_URL_SD
#define URL_SDPATH				"/home/cache.tmp"		//url缓存路径
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

#endif


