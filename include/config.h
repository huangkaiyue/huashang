#ifndef _CONFIG_H_
#define _CONFIG_H_
//----------------------ç‰ˆæœ¬ç±»-----------------------------------

//#define TEST_SDK					//æµ‹è¯•SDK

//#define WORK_INTER

#ifdef TEST_SDK						//ä½¿èƒ½å‘½ä»¤è¾“å…¥è¡Œ
#endif

//#define DATOU_JIANG		//è’‹æ€»
//#define QITUTU_SHI		//çŸ³æ€»
//#define TANGTANG_LUO		//ç½—æ€»
#define HUASHANG_JIAOYU
//----------------------åŠŸèƒ½ç±»-----------------------------------

#define SYSTEMLOCK				//æµ‹è¯•ç‰ˆæœ¬é™åˆ¶å¼€æœºæ¬¡æ•°
#define CLOCESYSTEM				//è¶…æ—¶é€€å‡º
//#define CLOSE_VOICE				//ä¸å·¥ä½œå¤„äºå…³é—­éŸ³é¢‘çŠ¶æ€
#define SELECT_UDP				//å°†udpæ·»åŠ åˆ°select å½“ä¸­

#define MY_HTTP_REQ			//Ê¹ÓÃ×Ô¼ºĞ´µÄhttp ÇëÇó½Ó¿Ú
#define TULING_FILE_LOG		//¿ªÆôÍ¼ÁéĞ´ÈëÈÕÖ¾ÎÄ¼ş

#if defined(QITUTU_SHI)||defined(DATOU_JIANG)||defined(HUASHANG_JIAOYU)
	#define SPEEK_VOICES	//å¼€å¯å¯¹è®²åŠŸèƒ½
	#define SPEEK_VOICES1	//æŒ‰é”®åˆ‡æ¢ä¼šè¯
	#define PALY_URL_SD		//ä¸‹è½½ä¿å­˜åˆ°æœ¬åœ°
	#define LOCAL_MP3		//MP3æœ¬åœ°æ’­æ”¾
	#define	LED_LR			//LEDå·¦å³ç¯ä»¥åŠå¤šæŒ‰é”®
#endif

#ifdef DATOU_JIANG	//å¤§å¤´---è’‹æ€»
#endif
#if defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)	//çŸ³æ€»---å¥½å¥‡å…”
	#define CLOCKTOALIYUN	//é˜¿é‡Œäº‘é—¹é’Ÿ
#endif

#if defined(HUASHANG_JIAOYU)
	#define  HUASHANG_JIAOYU_PLAY_JSON_FILE		"huashang_play.json"
#endif

//----------------------æµ‹è¯•ç±»-----------------------------------

//#define TEST_MIC			//æµ‹è¯•å½•éŸ³å¹¶ç›´æ¥æ’­æ”¾å‡ºæ¥

//#define TEST_SAVE_MP3		//æµ‹è¯•ç”¨äºä¿å­˜è¯­éŸ³è¯†åˆ«ä¹‹åï¼Œä¸‹è½½ä¸‹æ¥çš„MP3æ–‡æœ¬ä¿¡æ¯

#define ENABLE_LOG			//ä½¿èƒ½å†™å…¥æ–‡ä»¶log


//----------------------ç”¨æˆ·æ•°æ®ç±»-------------------------------

#define UDP_BRO_PORT 		20001						// æœ¬åœ°å¹¿æ’­ç«¯å£

#ifndef TEST_SDK	
	#define TF_SYS_PATH 		"/media/mmcblk0p1/"		//tfå¡è·¯å¾„
#else
	#define TF_SYS_PATH 		"/mnt/neirong/"			//tfå¡è·¯å¾„
#endif
#ifdef LOCAL_MP3
#ifdef	LED_LR
#if 0
#define TF_MP3_PATH 		"keji/"						//æœ¬åœ°éŸ³ä¹è·¯å¾„
#define TF_STORY_PATH 		"why/"						//æœ¬åœ°æ•…äº‹è·¯å¾„
#define TF_ENGLISH_PATH		"english/"					//æœ¬åœ°è‹±è¯­è·¯å¾„
#else
#define TF_MP3_PATH 		"æ•…äº‹/"						//æœ¬åœ°éŸ³ä¹è·¯å¾„ï¼ˆå„¿æ­Œï¼‰
#define TF_STORY_PATH 		"ç´ è´¨/"						//æœ¬åœ°æ•…äº‹è·¯å¾„
#define TF_ENGLISH_PATH		"è‹±è¯­/"						//æœ¬åœ°è‹±è¯­è·¯å¾„
#define TF_GUOXUE_PATH		"å›½å­¦/"						//æœ¬åœ°å›½å­¦è·¯å¾„ï¼ˆç§‘æŠ€ï¼‰
#endif
#else
#define TF_MP3_PATH 		"mp3/"						//æœ¬åœ°éŸ³ä¹è·¯å¾„
#define TF_STORY_PATH 		"story/"					//æœ¬åœ°æ•…äº‹è·¯å¾„
#define TF_ENGLISH_PATH		"english/"					//æœ¬åœ°è‹±è¯­è·¯å¾„
#endif
#endif

#define URL_SDPATH				"/home/cache.tmp"		//urlç¼“å­˜è·¯å¾„
#ifdef PALY_URL_SD
#ifdef TEST_SDK	
	#define MP3_SDPATH			"/mnt/neirong/music/"			//urlä¿å­˜è·¯å¾„
	#define MP3_LIKEPATH		"/mnt/neirong/ximalaya/"		//urlå–œçˆ±è·¯å¾„
#else
	#define MP3_SDPATH			"/media/mmcblk0p1/music/"	//urlä¿å­˜è·¯å¾„
	#define MP3_LIKEPATH		"/media/mmcblk0p1/ximalaya/"//urlå–œçˆ±è·¯å¾„
#endif
#define MP3_PATHLEN			sizeof(MP3_SDPATH)
#define MP3_LIKEPATHLEN		sizeof(MP3_LIKEPATH)
#endif

#define SYSTEMLOCKNUM	500		//é™åˆ¶æ¬¡æ•°


//#define DEBUG_SYSTEM_IP					//¿ªÆô°´¼ü°´ÏÂÓïÒô²¥·Åwifi ºÍIP µØÖ·¹¦ÄÜ
//#define TEST_PLAY_EQ_MUSIC			//²âÊÔÒôĞ§		
//#define PCM_TEST						//²âÊÔ±£´æpcmÎÄ¼ş


#define TULING_PLAY_TEXT_WEIXIN_FAILED	"Ğ¡ÅóÓÑ°ó¶¨Ê§°Ü£¬ÇëÖØĞÂÔÚÎ¢ĞÅ½çÃæÊäÈëÒª°ó¶¨µÄÉè±¸ºÅ¡£"


#define NET_SERVER_FILE_LOCK			"/var/server.lock"		//¿ª»úÁªÍø½ø³ÌÎÄ¼şËø

#define INTEN_NETWORK_FILE_LOCK			"/var/internet.lock"	//Á¬½ÓÎÄ¼şËø£¬ºÍÁªÍø½ø³Ìµ±ÖĞ£¬É¨ÃèºÍÁ¬½Ó¹ı³Ìµ±ÖĞÆğµ½Ò»¸ö»¥³âÎÄ¼şËø×÷ÓÃ£¬·ÀÖ¹Õâ±ß½ø³ÌÔÚÅäÍø×´Ì¬£¬ÁªÍø½ø³Ì½øĞĞÉ¨ÃèºÍÁ¬½Ó

#define SMART_CONFIG_FILE_LOCK			"/var/SmartConfig.lock"	//ÅäÍøÎÄ¼şËø,·ÀÖ¹µÚ¿ª»ú¹ı³Ì×Ô¶¯Á¬½ÓÍøÂç³åÍ»

#define LOCAL_SERVER_FILE_LOCK			"/var/localserver.lock"	//±¾µØ·şÎñÆ÷ÎÄ¼şËø

#define ENABLE_RECV_NETWORK_FILE_LOCK	"/var/startNet.lock"	//Ê¹ÄÜ½ÓÊÕÎÄ¼şËø

#define CLOSE_SYSTEM_LOCK_FILE			"/var/close_system.lock"
#endif


#define XIAI_DIR 				"xiai"
#define HUASHANG_GUOXUE_DIR		"huashangedu"	
//#define TEST_ERROR_TULING

