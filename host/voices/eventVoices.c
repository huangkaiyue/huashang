#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/pool.h"
#include "host/voices/callvoices.h"
#include "host/ap_sta.h"
#include "nvram.h"
#include "../../net/network.h"
#include "host/voices/WavAmrCon.h"
#include "systools.h"
#include "gpio_7620.h"

#include "../host/studyvoices/qtts_qisc.h"
#include "../sdcard/musicList.h"
#include "config.h"

SysMessage sysMes;
//------------------------config network and set system network state---------------------------------------------------------
/*
@ 设置网络状态
@ 
@
*/
static void setNetWorkLive(unsigned char state){
	sysMes.netstate=state;
}
static int getNetWorkLive(void){
	return sysMes.netstate;
}
/*
@ 检查网络状态 
@ enablePlayVoices :用于控制播放wifi 断网状态语音
@ 0: 连接网络正常  -1: 连接网络失败
*/
int checkNetWorkLive(unsigned char enablePlayVoices){
#if 1
	if(getNetWorkLive()==NETWORK_ER||getNetWorkLive()==NETWORK_UNKOWN){
		//播报台本
		if(getEventNum()>0){	//检查是否添加过事件
			return -1;
		}
		//添加系统音进去播放，提示用户进行联网
		if(enablePlayVoices==ENABLE_CHECK_VOICES_PLAY)
			Create_PlaySystemEventVoices(NETWORK_ERROT_PLAY);
		return -1;
	}else if(getNetWorkLive()==NETWORK_OK){
		return 0;
	}
	return -1;
#else
	return 0;
#endif
}
/*******************************************************
函数功能: 查看NetManger进程是否存在pid号
参数: pid_name	进程名字
返回值: 无
********************************************************/
static int get_pid_name(char *pid_name){
	if(!strcmp(pid_name,"NetManger")){
		return 0;
	}
	return -1;
}
static void StartNetServer(void){
	system("NetManger -t 5 -wifi on &");
	sleep(1);
}
static void CheckNetServer(void){
	sleep(5);
	if(judge_pid_exist(get_pid_name)){
		remove(NET_SERVER_FILE_LOCK);
		remove(INTEN_NETWORK_FILE_LOCK);
		StartNetServer();
	}
}
//短按键按下获取wifi 名字并播放
void ShortKeyDown_ForPlayWifiMessage(void){
	if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
		if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//检查网络
			return;
		}
		char buf[128]={0};
		char *wifi = nvram_bufget(RT2860_NVRAM, "ApCliSsid");
		if(strlen(wifi)<=0){
			printf("read wifi failed \n");
			return ;
		}
#ifdef DEBUG_SYSTEM_IP
		char IP[20]={0};
		GetNetworkcardIp((char * )"apcli0",IP);
		snprintf(buf,128,"已连接 wifi %s  IP地址是 %s",wifi,IP);
#else
		snprintf(buf,128,"已连接 wifi %s ",wifi);
#endif	
		Create_PlayQttsEvent(buf,QTTS_GBK);
	}
}

/*******************************************************
函数功能: 长按键触发配网事件
参数: 无
返回值: 无
********************************************************/
void LongNetKeyDown_ForConfigWifi(void){
	if(judge_pid_exist(get_pid_name)){
		remove(NET_SERVER_FILE_LOCK);
		remove(INTEN_NETWORK_FILE_LOCK);
		StartNetServer();
	}
	WiterSmartConifg_Log("Network key down","ok");
	if(!checkInternetFile()){
		WiterSmartConifg_Log("startSmartConfig checkInternetFile","failed");
		return;
	}
	if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){//处于播放事件当中，不予许配网
		disable_gpio();
		startSmartConfig(Create_PlaySystemEventVoices,enable_gpio);	
	}else{		
		WiterSmartConifg_Log("startSmartConfig  failed ","is not RECODE_PAUSE");
	}
}
//连接成功设置工作指示灯,更新muc时间
static void Link_NetworkOk(void){
	Led_vigue_close();
#if defined(TANGTANG_LUO)||defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)
	led_lr_oc(openled);
#elif defined(DATOU_JIANG)
	led_lr_oc(closeled);
#endif
	SocSendMenu(3,0);			//发送本地时间给mcu
	usleep(100*1000);
	setNetWorkLive(NETWORK_OK);		//设置联网状态
}
//联网失败,闪烁指示灯
static void Link_NetworkError(void){
	pool_add_task(Led_vigue_open,NULL);
#if defined(TANGTANG_LUO)||defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)
	led_lr_oc(closeled);

#elif defined(DATOU_JIANG)
	led_lr_oc(openled);
#endif
	setNetWorkLive(NETWORK_ER);
}
//--------------end config network and set system network state---------------------------------------------------------
/*******************************************************
函数功能: 添加网络URL地址到队列当中进行播放
参数: data 播放歌曲信息，已经malloc申请内存了	
返回值: 无
********************************************************/
int __AddNetWork_UrlForPaly(const void *data){
	WritePlayUrl_Log("url start add \n");
	if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//检查网络
		goto exit0;
	}
	//防止添加过快
	if(getEventNum()>0||getplayEventNum()>0){
		DEBUG_EVENT("num =%d \n",getEventNum());
		goto exit0;
	}
	if(GetplayNetwork_LockState()==PLAY_NETWORK_VOICES_LOCK){
		printf("is tuling play lock \n");
		goto exit0;
	}
	if(GetRecordeVoices_PthreadState() == PLAY_WAV){
		WritePlayUrl_Log("add failed ,reocde voices pthread is PLAY_WAV\n");
		goto exit0;
	}else if(GetRecordeVoices_PthreadState() == PLAY_DING_VOICES){
		goto exit0;
	}
	WritePlayUrl_Log("add url ok\n");
	return AddworkEvent((const char *)data,0,URL_VOICES_EVENT);
exit0:
	free(data);
	return -1;
}
#ifdef LOCAL_MP3
//设置自动播放模式下，记录的起始时间
void setAutoPlayMusicTime(void){
	time_t t;
	sysMes.Playlocaltime=time(&t);
}
/*******************************************************
函数功能: 添加本地音乐到队列进行播放
参数: localpath 本地MP3播放地址	
返回值: 0添加成功 -1添加失败
********************************************************/
int __AddLocalMp3ForPaly(const char *localpath){
	if(getEventNum()>0){	//事件任务过多，直接丢掉，防止添加过快，导致后面清理时间过长
		DEBUG_EVENT("num =%d \n",getEventNum());
		return -1;
	}
	if(GetplayNetwork_LockState()==PLAY_NETWORK_VOICES_LOCK){
		printf("is tuling play lock \n");
		return -1;
	}
	if(GetRecordeVoices_PthreadState() == PLAY_WAV){	//处于播放qtts文件
		DEBUG_EVENT(" PLAY_WAV \n");
		ExitPlay_WavVoices();
		return -1;
	}else if(GetRecordeVoices_PthreadState() == PLAY_DING_VOICES){
		return -1;
	}
	char *URL= (char *)calloc(1,strlen(localpath)+1);
	if(URL==NULL){
		perror("calloc error !!!");
		return -1;
	}
	sprintf(URL,"%s",localpath);
	return AddworkEvent(URL,0,LOCAL_MP3_EVENT);
}
/*
@ 根据目录菜单和路径获取本地sdcatd 歌曲名字进行播放
@ menu 目录菜单(英语/科技/国学/收藏等内容分类 ) path sdcard目录路径 playMode播放模式 下一首/上一首
@ 0添加成功 -1添加失败
*/
static int GetSdcardMusicNameforPlay(unsigned char menu,const char *path, unsigned char playMode){	
	char buf[128]={0};
	char filename[64]={0};
	int ret=-1;
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	int len =snprintf(buf,128,"%s%s",TF_SYS_PATH,path);
	if(menu==xiai){		//获取喜爱目录下的歌曲路径名
		if(PlayxiaiMusic((const char *)TF_SYS_PATH,path,filename, playMode) == -2){
			Create_PlaySystemEventVoices(LIKE_ERROT_PLAY);
			return ret;
		}
		if(!strcmp(filename,"")){//获取的路径名为空，直接退出
			//Create_PlaySystemEventVoices(LIKE_ERROT_PLAY);
			return ret;
		}
	}else{				//获取客户自定义存放歌曲路径名
		if(GetSdcardMusic((const char *)TF_SYS_PATH,path,filename, playMode)){
			Create_PlaySystemEventVoices(PLAY_ERROT_PLAY);
			return ret;
		}
		if(!strcmp(filename,"")){//获取的路径名为空，直接退出
			return ret;
		}
	}
	snprintf(buf+len,128-len,"%s",filename);
	ret=__AddLocalMp3ForPaly((const char *)buf);			//添加歌曲到队列播放
	if(ret==0)
		sysMes.localplayname=menu;
	return ret;
}
#endif


/*******************************************************
函数功能: 播放MP3
参数: play 本地MP3播放命令 或 URL地址
返回值: 无
********************************************************/
int Create_playMusicEvent(const void *play,unsigned char Mode){
#ifdef LOCAL_MP3
	int ret=-1;
	if(!strcmp((const char *)play,"mp3")){
		ret=GetSdcardMusicNameforPlay(mp3,TF_MP3_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"story")){
		ret=GetSdcardMusicNameforPlay(story,TF_STORY_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"english")){
		ret=GetSdcardMusicNameforPlay(english,TF_ENGLISH_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"guoxue")){
		ret=GetSdcardMusicNameforPlay(guoxue,TF_GUOXUE_PATH,Mode);
	}
	else if(!strcmp((const char *)play,XIAI_DIR)){
		ret=GetSdcardMusicNameforPlay(xiai,XIMALA_MUSIC_DIRNAME,Mode);
	}
#if defined(HUASHANG_JIAOYU)	
	else if(!strcmp((const char *)play,HUASHANG_GUOXUE_DIR)){
		GetScard_forPlayHuashang_Music(Mode,(const void *)play);
	}
#endif	
	else{
		ret=0;
		__AddNetWork_UrlForPaly(play);
	}
	return ret;
#else
	return __AddNetWork_UrlForPaly(play);
#endif	
}
/*******************************************************
函数功能: 清理URL事件
参数: 无
返回值: 无
********************************************************/
void Create_CleanUrlEvent(void){
	sysMes.localplayname=0;
	AddworkEvent(NULL,0,SET_RATE_EVENT);
}
/*******************************************************
函数功能: 创建播放QTTS语音事件 --->文本转语音并播放
参数: txt QTTS文本 type :0---GBK 格式 1----UTF8 格式
返回值: 无
********************************************************/
void Create_PlayQttsEvent(const char *txt,int type){
	if(GetRecordeVoices_PthreadState() == PLAY_WAV){	//解决在智能会话过程当中，添加播放系统语音、播放wifi 名字导致的死机现象  2017-3-26 
		printf("%s: current is play wav\n",__func__);
		return ;
	}

	if (GetRecordeVoices_PthreadState() == PLAY_DING_VOICES){
		return ;
	}
	if (checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//检查网络
		return;
	}	
	if (GetRecordeVoices_PthreadState() == PLAY_URL){	//当前播放歌曲
		Create_CleanUrlEvent();
	}
	char *TXT = (char *)calloc(1,strlen(txt)+1);
	if (TXT){
		sprintf(TXT,"%s",txt);
		printf("%s: add qtts ok \n",__func__);
		AddworkEvent((const char *)TXT,type,QTTS_PLAY_EVENT);
	}
}

/*******************************************************
函数功能: 会话按键按下信号,启动录音 
参数: 无
返回值: 无
********************************************************/
void TulingKeyDownSingal(void){
	Write_Speekkeylog((const char *)"TulingKeyDownSingal",0);
	//处于微信对讲状态，直接退出	
	if(GetRecordeVoices_PthreadState()==START_SPEEK_VOICES||GetRecordeVoices_PthreadState()==END_SPEEK_VOICES){		
		Write_Speekkeylog((const char *)"START_SPEEK_VOICES",GetRecordeVoices_PthreadState());
		return;
	}	
	if (GetRecordeVoices_PthreadState() == PLAY_DING_VOICES){
		return ;
	}
	if(GetplayNetwork_LockState()==PLAY_NETWORK_VOICES_LOCK){
		printf("is tuling play lock \n");
		ExitPlayNetworkState();
		return ;
	}else if (GetRecordeVoices_PthreadState() == PLAY_WAV){//处于播放wav原始数据状态
		ExitPlay_WavVoices();
		Write_Speekkeylog((const char *)"PLAY_WAV",GetRecordeVoices_PthreadState());
	}
	else if (GetRecordeVoices_PthreadState() == PLAY_URL){//处于播放歌曲状态	
		Create_CleanUrlEvent();
		Write_Speekkeylog((const char *)"PLAY_URL",GetRecordeVoices_PthreadState());
	}else{		
#if defined(HUASHANG_JIAOYU)	//华上教育有离线语音识别接口，需要采集音频进行离线识别
#ifndef XUN_FEI_OK		
#else
#endif
		if (checkNetWorkLive(DISABLE_CHECK_VOICES_PLAY)){	//检查网络,没有网络直接退出播放
			Write_Speekkeylog((const char *)"ExitPlay_WavVoices",GetRecordeVoices_PthreadState());
			return;
		}
#endif		
		sysMes.localplayname=0;	
		NetStreamExitFile();	//在微信端推送歌曲，没有进行播放下一首歌曲的时候，突然按下智能会话按键，需要切换采样率，才能进入智能会话状态
		if(SetWm8960Rate(RECODE_RATE)){	//切换采样率失败，退出(防止多线程当中切换，资源冲突问题)
			return ;
		}
		StartTuling_RecordeVoices();
		Write_Speekkeylog((const char *)"StartTuling_RecordeVoices",GetRecordeVoices_PthreadState());
	}
}
//关机保存文件和清理工作
void *Close_Mtk76xxSystem(void *arg){
	char token[64]={0};
	GetTokenValue(token);
	Save_TulingToken_toRouteTable((const char *)token);
	SaveVol_toRouteTable(GetVol());		//设置声音到路由表
#ifdef CLOCKTOALIYUN
	CloseSystemSignToaliyun();			//发送关机信号给闹钟
#endif

#ifdef PALY_URL_SD	
	SaveSystemPlayNum();
#endif	
#ifdef HUASHANG_JIAOYU
	closeSystemSave_huashangData();
#endif
	return NULL;
}
//----------------------播放系统声音有关的、事件的产生、消费处理-----------------------------------------------------
//播放智能会话按键误触发产生的声音
static void TaiBenToTulingNOVoices(void){
	int i=(1+(int) (10.0*rand()/(RAND_MAX+1.0)));	
	switch(i){
		case 1:
			PlaySystemAmrVoices(NO_VOICES);
			break;
		case 2:
			PlaySystemAmrVoices(NO_VOICES_1);
			break;
		case 3:
			PlaySystemAmrVoices(NO_VOICES_2);
			break;
		case 4:
			PlaySystemAmrVoices(NO_VOICES_3);
			break;
		case 5:
			PlaySystemAmrVoices(NO_VOICES_4);
			break;
		case 6:
			PlaySystemAmrVoices(NO_VOICES_5);
			break;
		case 7:
			PlaySystemAmrVoices(NO_VOICES_6);
			break;
		case 8:
			PlaySystemAmrVoices(NO_VOICES_7);
			break;
		case 9:
			PlaySystemAmrVoices(NO_VOICES_8);
			break;
		case 10:
			PlaySystemAmrVoices(NO_VOICES_9);
			break;
	}
}
#define TLERNUM 34.0
static void TaiwanToTulingError(void){
	char buf[32]={0};
	int i=(1+(int)(TLERNUM*rand()/(RAND_MAX+1.0)));
	snprintf(buf,32,"qtts/TulingError%d_8k.amr",i);
	PlaySystemAmrVoices(buf);
}
/*
@ 没有网络的时候，播放本地系统固定录好台本
@
*/
static void Handle_PlayTaiBenToNONetWork(void){
	char file[64]={0};
	int i=(1+(int)(5.0*rand()/(RAND_MAX+1.0)));
	snprintf(file,64,"qtts/network_error_8K_%d.amr",i);
	PlaySystemAmrVoices(file);
}
static void CreateCloseSystemLock(void){
	FILE *fp = fopen(CLOSE_SYSTEM_LOCK_FILE,"w+");
	if(fp){
		fclose(fp);
	}
}
/*******************************************************
函数功能: 创建一个播放系统声音事件，
参数: sys_voices 系统音标号	
返回值: 无
********************************************************/
void Create_PlaySystemEventVoices(int sys_voices){
	if(sys_voices==END_SYS_VOICES_PLAY){	//结束音退出事件
		CreateCloseSystemLock();
#ifdef PALY_URL_SD
		pool_add_task(Close_Mtk76xxSystem,NULL);//关机删除，长时间不用的文件
#endif
		cleanQuequeEvent();	//清理队列
		if(GetRecordeVoices_PthreadState()==PLAY_WAV){
			ExitPlay_WavVoices();	//清理事件
		}
		if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
			NetStreamExitFile();
		}
	}
	if(GetplayNetwork_LockState()==PLAY_NETWORK_VOICES_LOCK){
		printf("is tuling play lock \n");
		ExitPlayNetworkState();
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
		return;
	}
	else if(GetRecordeVoices_PthreadState() ==PLAY_URL){
		Create_CleanUrlEvent();
	}else if(GetRecordeVoices_PthreadState()==PLAY_WAV){
		ExitPlay_WavVoices();		
	}
	AddworkEvent(NULL,sys_voices,SYS_VOICES_EVENT);
}
//添加播放过渡音事件
void Create_PlayTulingWaitVoices(int sys_voices){
	AddworkEvent(NULL,sys_voices,SYS_VOICES_EVENT);
}
/*******************************************************
函数功能: 系统音事件处理函数
参数: sys_voices 系统音标号
返回值: 无
********************************************************/
void Handle_PlaySystemEventVoices(int sys_voices){
	switch(sys_voices){
		case END_SYS_VOICES_PLAY:					//结束音
			PlaySystemAmrVoices(END_SYS_VOICES);
			Led_vigue_close();
			Led_System_vigue_close();
			Mute_voices(MUTE);		//关闭功放
#if defined(TANGTANG_LUO) || defined(QITUTU_SHI) || defined(HUASHANG_JIAOYU)
			close_sys_led();
#endif
#ifdef DATOU_JIANG
			open_sys_led();
#endif
			led_lr_oc(closeled);
			break;
		case LOW_BATTERY_PLAY:						//低电关机
#ifdef PALY_URL_SD
			pool_add_task(Close_Mtk76xxSystem,NULL);//关机删除，长时间不用的文件
#endif
			PlaySystemAmrVoices(LOW_BATTERY);			
			break;
		case RESET_HOST_V_PLAY:						//恢复出厂设置
			PlaySystemAmrVoices(RESET_HOST_V);
			break;
//----------------------重连有关-----------------------------------------------------
		case REQUEST_FAILED_PLAY:					//重连，请求服务器数据失败
			PlaySystemAmrVoices(REQUEST_FAILED);
			break;
		case UPDATA_END_PLAY:						//更新固件结束
			PlaySystemAmrVoices(UPDATA_END);
			//system("sleep 8 && reboot &");
			break;
		case TIMEOUT_PLAY_LOCALFILE:				//请求服务器超时，播放本地已经录制好的音频
			TaiwanToTulingError();
			break;
		case 8:
			PlaySystemAmrVoices(REQUEST_FAILED);
			break;
//----------------------网络有关-----------------------------------------------------
		case CONNET_ING_PLAY:			//正在连接，请稍等
			PlaySystemAmrVoices(CHANGE_NETWORK);
			PlaySystemAmrVoices(CONNET_TIME);
			break;
		case START_SMARTCONFIG_PLAY:		//启动配网
			pool_add_task(Led_vigue_open,NULL);
			led_lr_oc(closeled);
			setNetWorkLive(NETWORK_ER);
			PlaySystemAmrVoices(START_INTERNET);
			break;
		case SMART_CONFIG_OK_PLAY:		//接受密码成功
			PlaySystemAmrVoices(YES_REAVWIFI);
			break;
		case CONNECT_OK_PLAY:			//连接成功
			PlaySystemAmrVoices(LINK_SUCCESS);
			Link_NetworkOk();		//连接成功关灯，开灯，状态设置
			enable_gpio();
			break;
		case NOT_FIND_WIFI_PLAY:			//没有扫描到wifi
			PlaySystemAmrVoices(NO_WIFI);
			enable_gpio();
			break;
		case SMART_CONFIG_FAILED_PLAY:	//没有收到用户发送的wifi
			PlaySystemAmrVoices(NOT_REAVWIFI);
			break;
		case NOT_NETWORK_PLAY:			//没有连接成功
			PlaySystemAmrVoices(NO_NETWORK_VOICES);
			Link_NetworkError();
			enable_gpio();
			break;
		case CONNET_CHECK_PLAY:			//检查网络是否可用
			PlaySystemAmrVoices(CHECK_INTERNET);
			break;
		case WIFI_CHECK_PLAY:			//检查网络
			PlaySystemAmrVoices(CHECK_WIFI);
			PlaySystemAmrVoices(CHECK_WIFI_WAIT);
			break;
		case WIFI_NO:			//检查网络NO
			PlaySystemAmrVoices(CHECK_WIFI_NO);
			break;
		case WIFI_YES:			//检查网络OK
			PlaySystemAmrVoices(CHECK_WIFI_YES);
			break;
//----------------------对讲有关-----------------------------------------------------
		case SEND_OK_PLAY:			//发送成功
			PlaySystemAmrVoices(SEND_OK);
			break;
		case SEND_ERROR_PLAY:			//发送失败
			PlaySystemAmrVoices(SEND_ERROR);
			break;
		case SEND_LINK_PLAY:			//正在发送
			PlaySystemAmrVoices(SEND_LINK);
			break;
		case KEY_DOWN_PLAY:			//按键按下	=---正在发送
			PlaySystemAmrVoices(KEY_VOICE_DOWN);
			break;
		case PLAY_ERROT_PLAY:			//播放失败
			PlaySystemAmrVoices(PLAY_ERROR);
			break;
		case LIKE_ERROT_PLAY:			//
			PlaySystemAmrVoices(LIKE_ERROR);
			break;
		case TF_ERROT_PLAY:				//TF加载失败
			PlaySystemAmrVoices(TF_ERROR);
			break;
//=====================================================================
		case BIND_SSID_PLAY:			//
			PlaySystemAmrVoices(BIND_SSID);
			break;
		case BIND_OK_PLAY:			//
			PlaySystemAmrVoices(BIND_OK);
			break;
		case SEND_LINK_ER_PLAY:			//
			PlaySystemAmrVoices(SEND_LINK_ER);
			break;
		case TALK_CONFIRM_PLAY:			//
			PlaySystemAmrVoices(TALK_CONFIRM);
			break;
		case TALK_CONFIRM_OK_PLAY:			//
			PlaySystemAmrVoices(TALK_CONFIRM_OK);
			break;
		case TALK_CONFIRM_ER_PLAY:			//
			PlaySystemAmrVoices(TALK_CONFIRM_ER);
			break;
		case DOWNLOAD_ING_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_ING);
			break;
		case DOWNLOAD_ERROE_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_ERROE);
			break;
		case DOWNLOAD_END_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_END);
			break;
		case DOWNLOAD_25_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_25);
			break;
		case DOWNLOAD_50_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_50);
			break;
		case DOWNLOAD_75_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_75);
			break;
		case UPDATA_NEW_PLAY:			//
			PlaySystemAmrVoices(UPDATA_NEW);
			break;
		case UPDATA_START_PLAY:			//
			PlaySystemAmrVoices(UPDATA_START);
			break;
		case UPDATA_ERROR_PLAY:			//
			PlaySystemAmrVoices(UPDATA_ERROR);
			break;
//==========================================================================
		case NETWORK_ERROT_PLAY:				//网络连接失败
			Handle_PlayTaiBenToNONetWork();
			break;
		case AI_KEY_TALK_ERROR: 
			TaiBenToTulingNOVoices();
			break;
		case MIN_10_NOT_USER_WARN: 
			PlaySystemAmrVoices(SPEEK_WARNING);
			break;
		case TULING_WAIT_VOICES:
			play_waitVoices(TULING_WINT);
			printf("%s: play wait voices ok\n",__func__);
			break;
	}
}
//-------end--------播放系统声音有关的、事件的产生、消费处理-----------------------------------------------------
#ifdef SPEEK_VOICES
//播放微信发送过来语音文件  filename 发送过来的微信语音文件
void CreatePlayWeixinVoicesSpeekEvent(const char *filename){
	if(GetplayNetwork_LockState()==PLAY_NETWORK_VOICES_LOCK){
		printf("is tuling play lock \n");
		ExitPlayNetworkState();
		goto exit0;
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
		goto exit0;
	}
	else if(GetRecordeVoices_PthreadState() == PLAY_WAV){	//解决在智能会话过程当中，添加微信发送过来的语音导致的死机现象  2017-4-26 
		goto exit0;
	}
	else if(GetRecordeVoices_PthreadState() ==PLAY_URL){
		Create_CleanUrlEvent();
	}
	char *TXT= (char *)calloc(1,strlen(filename)+1);
	if(TXT){
		sprintf(TXT,"%s",filename);
		if(AddworkEvent(TXT,0,SPEEK_VOICES_EVENT)){
			printf("add play amr voices failed ,and remove file \n");
			goto exit0;
		}
		return;
	}
	
exit0:
	remove(filename);
}
#endif

#ifdef SPEEK_VOICES
static Speek_t *speek=NULL;
//先挂载录音再退出
static void shortVoicesClean(void){
	//lock
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	}
	pthread_mutex_unlock(&speek->mutex);
	//unlock
}
/*******************************************************
函数功能: 创建待发送的文件，并保持到本地当中,写入wav头
参数:	无
返回值: 0 创建成果 -1 创建失败
********************************************************/
static int CreateRecorderFile(void){
	//lock
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	}
	if((speek->savefilefp= fopen(SAVE_WAV_VOICES_DATA,"w+"))==NULL){
		perror("open send file failed \n");
		pthread_mutex_unlock(&speek->mutex);
		//unlock
		return -1;
	}
	speek->file_len=0;
	fwrite((char *)&pcmwavhdr,WAV_HEAD,1,speek->savefilefp);
	DEBUG_EVENT("create save file \n");
	pthread_mutex_unlock(&speek->mutex);
	//unlock
	return 0;
}
/*******************************************************
函数功能: 停止录音，并将数据发送给app用户
参数:
返回值:
********************************************************/
static void StopRecorder_AndSendFile(void){
	char filepath[64]={0};
	pcmwavhdr.size_8 = (speek->file_len+36);
	pcmwavhdr.data_size = speek->file_len;
	DEBUG_EVENT("fseek file \n");
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fseek(speek->savefilefp,0,SEEK_SET);
		fwrite(&pcmwavhdr,1,WAV_HEAD,speek->savefilefp);
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	
		pthread_mutex_unlock(&speek->mutex);
		time_t t;
		t = time(NULL);
		sprintf(filepath,"%s%d%s",CACHE_WAV_PATH,(unsigned int)t,".amr");
		if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath)){
			DEBUG_EVENT("enc_wav_amr_file failed");
			return;
		}
		DEBUG_EVENT("stop save file \n");
		uploadVoicesToaliyun(filepath,pcmwavhdr.data_size/10+6);
		PlaySystemAmrVoices(KEY_VOICE_DOWN);	//播放微信提示音，表示音频正在发送	
	}else{
		pthread_mutex_unlock(&speek->mutex);
	}
	//remove(SAVE_WAV_VOICES_DATA);
}
/*******************************************************
函数功能:微信语音对讲按键按下和弹起事件
参数:gpioState 0 按下  1 弹起
********************************************************/
void Create_WeixinSpeekEvent(unsigned int gpioState){	
	if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//检查网络
		return;
	}
	if(GetplayNetwork_LockState()==PLAY_NETWORK_VOICES_LOCK){
		printf("is tuling play lock \n");
		ExitPlayNetworkState();
		return;
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_URL){		//打断播放音乐
		Create_CleanUrlEvent();
		return;
	}else if(GetRecordeVoices_PthreadState()==PLAY_WAV){
		ExitPlay_WavVoices();
		return;
	}else if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
		//NetStreamExitFile();
		return;
	}
	DEBUG_EVENT("state %d\n",gpioState);
	AddworkEvent(NULL,gpioState,TALK_EVENT_EVENT);
}
/*******************************************************
函数功能:处理微信语音按键事件 
参数: gpioState 0 按下  1 弹起
********************************************************/
void Handle_WeixinSpeekEvent(unsigned int gpioState){
	int endtime,voicesTime=0;
	time_t t;
	if(gpioState==VOLKEYDOWN&&GetRecordeVoices_PthreadState()==RECODE_PAUSE){	//按下
		DEBUG_EVENT("gpioState(%d)...\n",gpioState);
		start_speek_wait();
		if(CreateRecorderFile()){		//创建音频文件节点，将插入到链表当中
			pause_record_audio();
		}else{
			speek->Starttime=time(&t);
			start_event_talk_message();
		}
	}else if(gpioState==VOLKEYUP){			//弹起
		DEBUG_EVENT("state(%d)\n",gpioState);
		start_speek_wait();
		endtime=time(&t);
		voicesTime = endtime - speek->Starttime;
		if(voicesTime<2||voicesTime>10){//时间太短或太长
			pause_record_audio();
			shortVoicesClean();
			PlaySystemAmrVoices(SEND_ERROR);
			return ;
		}else{
			StopRecorder_AndSendFile();
			pause_record_audio();
		}
	}
}
/*******************************************************
函数功能:保存语音数据 (需要将音频数据压缩保存)
参数:  voices_data 原始音频数据  size 原始音频数据大小
返回值:
********************************************************/
void SaveRecorderVoices(const char *voices_data,int size){
	int i=0;
	int endtime;
	time_t t;
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		endtime=time(&t);
		if((endtime - speek->Starttime)>60){
			pause_record_audio();
		}
#if 0	//右声道
		for(i=2; i<size; i+=4)		//双声道数据转成单声道数据
		{
			fwrite(voices_data+i,2,1,speek->savefilefp);
		}
#else	//左声道
		for(i=0; i<size; i+=4){
			fwrite(voices_data+i,2,1,speek->savefilefp);
		}
#endif
		i /=2; 
		speek->file_len +=i;
	}else{
		pause_record_audio();
	}
	pthread_mutex_unlock(&speek->mutex);
}
#endif			//endif SPEEK_VOICES  :微信对讲声音 事件的产生和处理

#ifdef PALY_URL_SD	
//--------------------------------------sdcard 收藏喜爱歌曲---------------------------------------------------
//检查sdcard 是否挂载，没有挂载，添加系统音播放提示
static int checkSdcard_MountState(void){
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){	
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);	//提示用户，没有检查到sdcard
		}
		return -1;
	}	
	return 0;
}

//使能收藏歌曲，当处于在线播放音乐，才能收藏
void Enable_SaveLoveMusicFlag(void){
	if(!checkSdcard_MountState()){
		if(GetRecordeVoices_PthreadState()!=PLAY_URL){	//检查当前是否在播放url下载的歌曲状态
			printf("%s: this is error save event current not online play music ,cannot save love music to sdcard \n",__func__);
			return ;
		}
		printf("%s: like music add \n",__func__);
		Save_like_music();
	}
}
//删除喜爱内容 delete
void Delete_LoveMusic(void){
	if(!checkSdcard_MountState()){
		if(GetRecordeVoices_PthreadState()!=PLAY_URL){	//检查播放状态
			printf("%s: this is error save event current not online play music ,cannot save love music to sdcard \n",__func__);
			return ;
		}
		printf("Del_like_music like music sub \n");
		Del_like_music();
	}
}
//创建保存微信下载mp3 事件到主线程队列当中
void Create_SaveWeixinDownMp3_EventToMainQueue(const char *saveFileName){
	int len = strlen(saveFileName);
	char *filename=(char *)calloc(1,len+1);
	if(filename){
		snprintf(filename,len+1,"%s",saveFileName);
		SetMainQueueLock(MAIN_QUEUE_UNLOCK);
		AddDownEvent((const char *)filename,WEIXIN_DOWN_MP3_EVENT);
	}
}
#endif
//----------------------end sdcard 收藏喜爱歌曲--------------------------------------------------------------

#ifdef LOCAL_MP3
//开机加载sdcard 当中数据库信息
static void *waitLoadMusicList(void *arg){
	int timeout=0;
	char dirBuf[128]={0};
	sleep(10);
	while(++timeout<20){
		if(!access(TF_SYS_PATH, F_OK)){		//检查tf卡
			break;
		}
		//检测到关键文件锁，直接退出，不执行下面操作，防止在关机过程读写sdcard
		if(access(CLOSE_SYSTEM_LOCK_FILE, F_OK)==F_OK){
			return ;
		}
		sleep(1);
	}
	snprintf(dirBuf,128,"%s",MP3_LIKEPATH);
	if(access(MP3_LIKEPATH, F_OK)){			//创建喜马拉雅目录
		mkdir(MP3_LIKEPATH,0777);	
	}
	SysOnloadMusicList((const char *)TF_SYS_PATH,(const char *)TF_MP3_PATH,(const char *)TF_STORY_PATH,(const char *)TF_ENGLISH_PATH,(const char *)TF_GUOXUE_PATH);
	CheckNetServer();	//检查网络服务
	if(sysMes.netstate==NETWORK_UNKOWN){
		sysMes.netstate=NETWORK_ER;
		enable_gpio();
	}
#ifdef HUASHANG_JIAOYU
	openSystemload_huashangData();
#endif
	return;
} 
#endif

/******************************************************************
初始化8960音频芯片，开启8K录音和播放双工模式,初始化gpio，播放开机启动音
*******************************************************************/
void InitMtkPlatfrom76xx(void){
#ifdef SPEEK_VOICES
	speek = (Speek_t *)calloc(1,sizeof(Speek_t));
	if(speek==NULL){
		return ;
	}
	speek->savefilefp=NULL;
	speek->file_len=0;
	pthread_mutex_init(&speek->mutex, NULL);
#endif

	InitMtk76xx_gpio();
	InitWm8960Voices();
#ifndef TEST_SDK
	PlaySystemAmrVoices(START_SYS_VOICES);//开机启动音
#endif
	initStream(ack_playCtr,WritePcmData,SetWm8960Rate,GetVol);

	InitEventMsgPthread();
	InitRecord_VoicesPthread();
#ifdef TEST_SDK
	enable_gpio();
#endif

#ifdef LOCAL_MP3
	InitMusicList();
	pool_add_task(waitLoadMusicList, NULL);	//防止T卡加载慢
#endif
}
/*
@ 
@ 销毁平台相关申请的资源
*/
void CleanMtkPlatfrom76xx(void){
	ExitRecord_Voicespthread();
	DestoryWm8960Voices();
	CleanMtk76xx_gpio();
	CleanEventMsgPthread();
	cleanStream();
}
