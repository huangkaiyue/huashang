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

extern void DelSdcardMp3file(char * sdpath);
extern int CheckFileNum(char * sdpath);

SysMessage sysMes;
/*
@ 没有网络的时候，播放本地系统固定录好台本
@
*/
static void TaiBenToNONetWork(void){
#if 0			//启动重连网络
	static int starttime_net=0;
	int endtime;
	time_t t;
	endtime=time(&t);
	if(((endtime-starttime_net)>1*MIN)&&(!checkconnetState())){
		starttime_net=time(&t);
		startServiceWifi();
	}
#endif
	//srand((unsigned)time(NULL));
	int i=(1+(int)(5.0*rand()/(RAND_MAX+1.0)));
	switch(i){
		case NETWORK_ERR_VOICES_1:
			play_sys_tices_voices(NETWORK_ERROR_1);
			break;
		case NETWORK_ERR_VOICES_2:
			play_sys_tices_voices(NETWORK_ERROR_2);
			break;
		case NETWORK_ERR_VOICES_3:
			play_sys_tices_voices(NETWORK_ERROR_3);
			break;
		case NETWORK_ERR_VOICES_4:
			play_sys_tices_voices(NETWORK_ERROR_4);
			break;
		case NETWORK_ERR_VOICES_5:
			play_sys_tices_voices(NETWORK_ERROR_5);
			break;
	}
}
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
@ 
@
*/
static int checkNetWorkLive(void){
	if(getNetWorkLive()==NETWORK_ER||getNetWorkLive()==NETWORK_UNKOWN){
		//播报台本
		if(getEventNum()>0){	//检查是否添加过事件
			return -1;
		}
		//添加系统音进去播放，提示用户进行联网
		Create_PlaySystemEventVoices(NETWORK_ERROT_PLAY);
		return -1;
	}else if(getNetWorkLive()==NETWORK_OK){
		return 0;
	}
	return -1;
}
/*******************************************************
函数功能: 添加网络URL地址到队列当中进行播放
参数: data 播放歌曲信息，已经malloc申请内存了	
返回值: 无
********************************************************/
static void __AddNetWork_UrlForPaly(const void *data){
	WritePlayUrl_Log("url start add \n");
	if(checkNetWorkLive()){	//检查网络
		return;
	}
	//防止添加过快
	if(getEventNum()>0||getplayEventNum()>0){
		DEBUG_EVENT("num =%d \n",getEventNum());
		return;
	}
	if(GetRecordeVoices_PthreadState() == PLAY_WAV_E){
		WritePlayUrl_Log("add failed ,reocde voices pthread is PLAY_WAV_E\n");
		//exitqttsPlay();
		return;
	}
	AddworkEvent((const char *)data,0,URL_VOICES_EVENT);
	WritePlayUrl_Log("add url ok\n");
}
#ifdef LOCAL_MP3
/*******************************************************
函数功能: 添加本地音乐到队列进行播放
参数: localpath 本地MP3播放地址	
返回值: 0添加成功 -1添加失败
********************************************************/
static int __AddLocalMp3ForPaly(const char *localpath){
	if(getEventNum()>0){	//事件任务过多，直接丢掉，防止添加过快，导致后面清理时间过长
		DEBUG_EVENT("num =%d \n",getEventNum());
		return -1;
	}
	if(GetRecordeVoices_PthreadState() == PLAY_WAV_E){	//处于播放qtts文件
		DEBUG_EVENT(" PLAY_WAV_E \n");
		exitqttsPlay();
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
		if(GetRecordeVoices_PthreadState()==PAUSE_E)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	if(menu==xiai){		//获取喜爱目录下的歌曲路径名
		PlayxiaiMusic((const char *)TF_SYS_PATH,path,filename, playMode);
	}
	else{				//获取客户自定义存放歌曲路径名
		if(GetSdcardMusic((const char *)TF_SYS_PATH,path,filename, playMode)){
			Create_PlaySystemEventVoices(PLAY_ERROT_PLAY);
			return ret;
		}
	}
	snprintf(buf,128,"%s%s",TF_SYS_PATH,path);
	if((menu==xiai)&&CheckFileNum(buf)){	 //喜爱目录为空,提示用户收藏歌曲
		Create_PlaySystemEventVoices(LIKE_ERROT_PLAY);
		return ret;
	}
	if(!strcmp(filename,"")){				//获取的路径名为空，直接退出
		//Create_PlaySystemEventVoices(PLAY_ERROT_PLAY);
		return ret;
	}
	snprintf(buf,128,"%s%s",buf,filename);
	printf("filepath = %s\n",buf);
	ret=__AddLocalMp3ForPaly((const char *)buf);			//添加歌曲到队列播放
	if(ret==0)
		sysMes.localplayname=menu;
	return ret;
}
#endif

#ifdef PALY_URL_SD
void CreateLikeMusic(void){
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeVoices_PthreadState()==PAUSE_E)	
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ;
	}
	if(GetRecordeVoices_PthreadState()!=PLAY_URL_E){	//检查播放状态
		if(GetRecordeVoices_PthreadState()==PAUSE_E){
			//Create_PlaySystemEventVoices(TF_ERROT_PLAY);	//Fix me 当前没有播放音乐
		}
		return ;
	}
	printf("CreateLikeMusic like music add \n");
	Save_like_music();
}
void DelLikeMusic(void){
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeVoices_PthreadState()==PAUSE_E)	
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ;
	}
	if(GetRecordeVoices_PthreadState()!=PLAY_URL_E){	//检查播放状态
		if(GetRecordeVoices_PthreadState()==PAUSE_E){
			//Create_PlaySystemEventVoices(TF_ERROT_PLAY);	//Fix me 当前没有播放音乐
		}
		return ;
	}
	printf("Del_like_music like music sub \n");
	Del_like_music();
}
#endif
#define VOLWAITTIME		300*1000	//音量加减时间间隔
#define VOLKEY_CHANG	2			//长按时间
static int volstart_time=0;
static unsigned char voltype=VOLKEYUP;
static unsigned char KeyNum=0;
static void handlevolandnext(void){
	time_t t;
	int volendtime=0;
	int ret;
	while(1){
		volendtime=time(&t);
		printf("volendtime-volstart_time = %d\n",volendtime-volstart_time);
		if((volendtime-volstart_time)<VOLKEY_CHANG){
			if(voltype==VOLKEYUP){
				//下一曲
				if(KeyNum==ADDVOL_KEY){
					createPlayEvent((const void *)"xiai",PLAY_NEXT);	//下一曲
				}else if(KeyNum==SUBVOL_KEY){
					createPlayEvent((const void *)"xiai",PLAY_PREV);	//上一曲
				}
				break;
			}
			usleep(10*1000);
			continue;
		}else if((volendtime-volstart_time)<5){
			if(voltype==VOLKEYUP){
				break;
			}
			if(KeyNum==ADDVOL_KEY){
				ret = Setwm8960Vol(VOL_ADD,0);	//音量加
			}else if(KeyNum==SUBVOL_KEY){
				ret = Setwm8960Vol(VOL_SUB,0);	//音量减
			}
			if(ret==1){	//音量加满了
				break;
			}
			//音量加
			usleep(VOLWAITTIME);
		}else{
			break;
		}
	}
	KeyNum=0;
}
/*
@ 复用按键接口，短按切换歌曲，长按设置音量加
@ 
@
*/
void VolAndNextKey(unsigned char state,unsigned char dir){
	time_t t;
	if(KeyNum==0||dir==KeyNum){
		if(state==VOLKEYDOWN){	//按下
			volstart_time=time(&t);
			voltype=VOLKEYDOWN;
			KeyNum=dir;
			pool_add_task(handlevolandnext,NULL);
		}else{			//弹起
			voltype=VOLKEYUP;
		}
	}
}
//设置自动播放模式下，记录的起始时间
void setAutoPlayMusicTime(void){
	time_t t;
	sysMes.Playlocaltime=time(&t);
}
/*******************************************************
函数功能: 播放MP3
参数: play 本地MP3播放命令 或 URL地址
返回值: 无
********************************************************/
int createPlayEvent(const void *play,unsigned char Mode){
	int ret=-1;
#ifdef LOCAL_MP3
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
	else if(!strcmp((const char *)play,"xiai")){
		ret=GetSdcardMusicNameforPlay(xiai,XIMALA_MUSIC_DIRNAME,Mode);
	}else{
		ret=0;
		__AddNetWork_UrlForPaly(play);
	}
	return ret;
#else
	__AddNetWork_UrlForPaly(play);
#endif	
}

/*******************************************************
函数功能: 清理URL事件
参数: 无
返回值: 无
********************************************************/
void CleanUrlEvent(void){
	sysMes.localplayname=0;
	AddworkEvent(NULL,0,SET_RATE_EVENT);
}
/*******************************************************
函数功能: 创建播放QTTS语音事件 --->文本转语音并播放
参数: txt QTTS文本 type :0---GBK 格式 1----UTF8 格式
返回值: 无
********************************************************/
void Create_PlayQttsEvent(const char *txt,int type){
	if (checkNetWorkLive()){	//检查网络
		return;
	}
	if (GetRecordeVoices_PthreadState() == PLAY_URL_E){
		CleanUrlEvent();
	}
	if (GetRecordeVoices_PthreadState() == PLAY_TULING_E){
		NetStreamExitFile();
	}
	char *TXT = (char *)calloc(1,strlen(txt)+1);
	if (TXT){
		sprintf(TXT,"%s",txt);
		AddworkEvent((const char *)TXT,type,QTTS_PLAY_EVENT);
	}
}

/*******************************************************
函数功能: 会话按键按下信号,启动录音 
参数: 无
返回值: 无
********************************************************/
void TulingKeyDownSingal(void){
	Write_Speekkeylog((const char *)"speekstart",0);
	if(GetRecordeVoices_PthreadState()==START_SPEEK_VOICES||GetRecordeVoices_PthreadState()==END_SPEEK_VOICES){		
		Write_Speekkeylog((const char *)"START_SPEEK_VOICES",GetRecordeVoices_PthreadState());
		return;
	}
	if (GetRecordeVoices_PthreadState() == PLAY_WAV_E){
		exitqttsPlay();
		if (GetPlaySystem_VoicesState() == 0){
			SetPlaySystem_VoiceState(EXIT_SYSTEM_PLAY);
		}
		Write_Speekkeylog((const char *)"PLAY_WAV_E",GetRecordeVoices_PthreadState());
	}else if (GetRecordeVoices_PthreadState() == PLAY_URL_E){
		CleanUrlEvent();
		Write_Speekkeylog((const char *)"PLAY_URL_E",GetRecordeVoices_PthreadState());
	}else{	
		if (checkNetWorkLive()){	//检查网络
			exitqttsPlay();		//暂时解决会话过程中，添加系统音导致问题
			Write_Speekkeylog((const char *)"exitqttsPlay",GetRecordeVoices_PthreadState());
			return;
		}	
		sysMes.localplayname=0;	
		NetStreamExitFile();
		if(SetWm8960Rate(RECODE_RATE)){
			return ;
		}
		StartTuling_RecordeVoices();
		Write_Speekkeylog((const char *)"StartTuling_RecordeVoices",GetRecordeVoices_PthreadState());
	}
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
void CheckNetServer(void){
	sleep(5);
	if(judge_pid_exist(get_pid_name)){
		remove(NET_SERVER_FILE_LOCK);
		remove(INTEN_NETWORK_FILE_LOCK);
		StartNetServer();
	}
}
/*******************************************************
函数功能: 配网事件
参数: 无
返回值: 无
********************************************************/
void NetKeyDown_ForConfigWifi(void){
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
	if(GetRecordeVoices_PthreadState()==PAUSE_E){//处于播放事件当中，不予许配网
		disable_gpio();
		startSmartConfig(Create_PlaySystemEventVoices,enable_gpio);	
	}else{		
		WiterSmartConifg_Log("startSmartConfig  failed ","is not PAUSE_E");
	}
}
//关机保存文件和清理工作
void Close_Mtk76xxSystem(void){
	char token[64]={0};
	GetTokenValue(token);
	Save_TulingToken_toRouteTable((const char *)token);
	SaveVol_toRouteTable(GetVol());		//设置声音到路由表
#ifdef CLOCKTOALIYUN
	CloseSystemSignToaliyun();			//发送关机信号给闹钟
#endif

#ifdef PALY_URL_SD	
	SaveSystemPlayNum();
	DelSdcardMp3file(MP3_SDPATH);
#endif	
}
//连接成功设置工作指示灯,更新muc时间
void Link_NetworkOk(void){
	Led_vigue_close();
#ifdef QITUTU_SHI
	led_lr_oc(openled);
#endif
#ifdef TANGTANG_LUO
	led_lr_oc(openled);
#endif
#ifdef DATOU_JIANG
	led_lr_oc(closeled);
#endif
	SocSendMenu(3,0);			//发送本地时间给mcu
	usleep(100*1000);
	setNetWorkLive(NETWORK_OK);		//设置联网状态
}
void Link_NetworkError(void){
	pool_add_task(Led_vigue_open,NULL);
#ifdef QITUTU_SHI
	led_lr_oc(closeled);
#endif
#ifdef TANGTANG_LUO
	led_lr_oc(closeled);
#endif
#ifdef DATOU_JIANG
	led_lr_oc(openled);
#endif
	setNetWorkLive(NETWORK_ER);
}
#define TLERNUM 34.0
static void TaiwanToTulingError(void){
	char buf[32]={0};
	int i=(1+(int)(TLERNUM*rand()/(RAND_MAX+1.0)));
	snprintf(buf,32,"qtts/TulingError%d_8k.amr",i);
	PlaySystemAmrVoices(buf);
}
/*******************************************************
函数功能: 创建一个播放系统声音事件，
参数: sys_voices 系统音标号	
返回值: 无
********************************************************/
void Create_PlaySystemEventVoices(int sys_voices){
	PlaySystemAmrVoicesLog("playsys_start\n");
	if((sys_voices==END_SYS_VOICES_PLAY)){	//结束音退出事件
#ifdef PALY_URL_SD
		pool_add_task(Close_Mtk76xxSystem,NULL);//关机删除，长时间不用的文件
#endif
		cleanQuequeEvent();	//清理队列
		if(GetRecordeVoices_PthreadState()==PLAY_WAV){
			exitqttsPlay();	//清理事件
		}
		if(GetRecordeVoices_PthreadState() ==PLAY_TULING_E){
			NetStreamExitFile();
		}
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	AddworkEvent(NULL,sys_voices,SYS_VOICES_EVENT);
	PlaySystemAmrVoicesLog("playsys voices end \n");
}
/*******************************************************
函数功能: 系统音事件处理函数
参数: sys_voices 系统音标号
返回值: 无
********************************************************/
void Handle_PlaySystemEventVoices(int sys_voices){
	PlaySystemAmrVoicesLog("playsys voices handle \n");
//----------------------系统有关-----------------------------------------------------
	switch(sys_voices){
		case END_SYS_VOICES_PLAY:					//结束音
			play_sys_tices_voices(END_SYS_VOICES);
			Led_vigue_close();
			Led_System_vigue_close();
			Mute_voices(MUTE);		//关闭功放
#ifdef QITUTU_SHI
			close_sys_led();
#endif
#ifdef DATOU_JIANG
			open_sys_led();
#endif
#ifdef TANGTANG_LUO
			close_sys_led();
#endif
			led_lr_oc(closeled);
			break;
		case TULING_WINT_PLAY:						//请稍等	
#ifdef QITUTU_SHI
			pool_add_task(Led_System_vigue_open,NULL);
#endif
#ifdef TANGTANG_LUO
			pool_add_task(Led_System_vigue_open,NULL);
#endif
			play_sys_tices_voices(TULING_WINT);
			break;
		case LOW_BATTERY_PLAY:						//低电关机
#ifdef PALY_URL_SD
			pool_add_task(Close_Mtk76xxSystem,NULL);//关机删除，长时间不用的文件
#endif
			play_sys_tices_voices(LOW_BATTERY);			
			break;
		case RESET_HOST_V_PLAY:						//恢复出厂设置
			play_sys_tices_voices(RESET_HOST_V);
			break;
//----------------------重连有关-----------------------------------------------------
		case REQUEST_FAILED_PLAY:					//重连，请求服务器数据失败
			PlaySystemAmrVoices(REQUEST_FAILED);
			break;
		case UPDATA_END_PLAY:						//更新固件结束
			play_sys_tices_voices(UPDATA_END);
			//system("sleep 8 && reboot &");
			break;
		case TIMEOUT_PLAY_LOCALFILE:				//请求服务器超时，播放本地已经录制好的音频
			TaiwanToTulingError();
			break;
		case 8:
			play_sys_tices_voices(REQUEST_FAILED);
			break;
//----------------------网络有关-----------------------------------------------------
		case CONNET_ING_PLAY:			//正在连接，请稍等
			play_sys_tices_voices(CHANGE_NETWORK);
			play_sys_tices_voices(CONNET_TIME);
			break;
		case START_SMARTCONFIG_PLAY:		//启动配网
			pool_add_task(Led_vigue_open,NULL);
			led_lr_oc(closeled);
			setNetWorkLive(NETWORK_ER);
			play_sys_tices_voices(START_INTERNET);
			break;
		case SMART_CONFIG_OK_PLAY:		//接受密码成功
			play_sys_tices_voices(YES_REAVWIFI);
			break;
		case CONNECT_OK_PLAY:			//连接成功
			play_sys_tices_voices(LINK_SUCCESS);
			Link_NetworkOk();		//连接成功关灯，开灯，状态设置
#if 0
			if(!access(TF_SYS_PATH, F_OK)){
				play_sys_tices_voices(WELCOME_PLAY);
				//PlayTuLingTaibenQtts("小朋友我们接着上次内容继续听吧，开始播放。",QTTS_GBK);
				createPlayEvent((const void *)"xiai",PLAY_NEXT);
			}
#endif
			enable_gpio();
			break;
		case NOT_FIND_WIFI_PLAY:			//没有扫描到wifi
			play_sys_tices_voices(NO_WIFI);
			enable_gpio();
			break;
		case SMART_CONFIG_FAILED_PLAY:	//没有收到用户发送的wifi
			play_sys_tices_voices(NOT_REAVWIFI);
			break;
		case NOT_NETWORK_PLAY:			//没有连接成功
			play_sys_tices_voices(NO_NETWORK_VOICES);
			Link_NetworkError();
			enable_gpio();
			break;
		case CONNET_CHECK_PLAY:			//检查网络是否可用
			play_sys_tices_voices(CHECK_INTERNET);
			break;
		case WIFI_CHECK_PLAY:			//检查网络
			play_sys_tices_voices(CHECK_WIFI);
			play_sys_tices_voices(CHECK_WIFI_WAIT);
			break;
		case WIFI_NO:			//检查网络NO
			play_sys_tices_voices(CHECK_WIFI_NO);
			break;
		case WIFI_YES:			//检查网络OK
			play_sys_tices_voices(CHECK_WIFI_YES);
			break;
//----------------------对讲有关-----------------------------------------------------
		case SEND_OK_PLAY:			//发送成功
			play_sys_tices_voices(SEND_OK);
			break;
		case SEND_ERROR_PLAY:			//发送失败
			play_sys_tices_voices(SEND_ERROR);
			break;
		case SEND_LINK_PLAY:			//正在发送
			play_sys_tices_voices(SEND_LINK);
			break;
		case KEY_DOWN_PLAY:			//按键按下	=---正在发送
			play_sys_tices_voices(KEY_VOICE_DOWN);
			break;
		case PLAY_ERROT_PLAY:			//播放失败
			play_sys_tices_voices(PLAY_ERROR);
			break;
		case LIKE_ERROT_PLAY:			//
			play_sys_tices_voices(LIKE_ERROR);
			break;
		case TF_ERROT_PLAY:				//TF加载失败
			play_sys_tices_voices(TF_ERROR);
			break;
//=====================================================================
		case BIND_SSID_PLAY:			//
			play_sys_tices_voices(BIND_SSID);
			break;
		case BIND_OK_PLAY:			//
			play_sys_tices_voices(BIND_OK);
			break;
		case SEND_LINK_ER_PLAY:			//
			play_sys_tices_voices(SEND_LINK_ER);
			break;
		case TALK_CONFIRM_PLAY:			//
			play_sys_tices_voices(TALK_CONFIRM);
			break;
		case TALK_CONFIRM_OK_PLAY:			//
			play_sys_tices_voices(TALK_CONFIRM_OK);
			break;
		case TALK_CONFIRM_ER_PLAY:			//
			play_sys_tices_voices(TALK_CONFIRM_ER);
			break;
		case DOWNLOAD_ING_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_ING);
			break;
		case DOWNLOAD_ERROE_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_ERROE);
			break;
		case DOWNLOAD_END_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_END);
			break;
		case DOWNLOAD_25_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_25);
			break;
		case DOWNLOAD_50_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_50);
			break;
		case DOWNLOAD_75_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_75);
			break;
		case UPDATA_NEW_PLAY:			//
			play_sys_tices_voices(UPDATA_NEW);
			break;
		case UPDATA_START_PLAY:			//
			play_sys_tices_voices(UPDATA_START);
			break;
		case UPDATA_ERROR_PLAY:			//
			play_sys_tices_voices(UPDATA_ERROR);
			break;
//==========================================================================
		case NETWORK_ERROT_PLAY:				//网络连接失败
			TaiBenToNONetWork();
			break;
	}
	PlaySystemAmrVoicesLog("playsys voices end \n");
}
#ifdef SPEEK_VOICES
void CreateSpeekEvent(const char *filename){
	if(GetRecordeVoices_PthreadState() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_TULING_E){
		NetStreamExitFile();
	}
	char *TXT= (char *)calloc(1,strlen(filename)+1);
	if(TXT){
		sprintf(TXT,"%s",filename);
		AddworkEvent(TXT,0,SPEEK_VOICES_EVENT);
	}
}
#endif

#ifdef SPEEK_VOICES

typedef struct{
	int file_len;
	FILE *savefilefp;
	int Starttime;	//录制微信对讲起始时间，用来检查文件录制长度，防止录制太短的音频
	pthread_mutex_t mutex;
}Speek_t;
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
	}else{
		pthread_mutex_unlock(&speek->mutex);
		return;
	}
	pthread_mutex_unlock(&speek->mutex);
	time_t t;
	t = time(NULL);
#ifdef CACHE_SDCARD
	sprintf(filepath,"%s%s%d%s",sysMes.localVoicesPath,CACHE_WAV_PATH,(unsigned int)t,".amr");
#else
	sprintf(filepath,"%s%d%s",CACHE_WAV_PATH,(unsigned int)t,".amr");
#endif
	if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath)){
		DEBUG_EVENT("enc_wav_amr_file failed");
		return;
	}
	DEBUG_EVENT("stop save file \n");
	Create_PlaySystemEventVoices(KEY_DOWN_PLAY);	//播放微信提示音，表示音频正在发送
	uploadVoicesToaliyun(filepath,pcmwavhdr.data_size/10+6);
	//remove(SAVE_WAV_VOICES_DATA);
}
/*******************************************************
函数功能:微信语音对讲按键按下和弹起事件
参数:gpioState 0 按下  1 弹起
********************************************************/
void Create_WeixinSpeekEvent(unsigned int gpioState){	
	if(checkNetWorkLive()){	//检查网络
		return;
	}
	if(gpioState==VOLKEYDOWN){	//按下
		
	}else{			//弹起
	
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_URL_E){		//打断播放音乐
		CleanUrlEvent();
		return;
	}else if(GetRecordeVoices_PthreadState()==PLAY_WAV_E){
		exitqttsPlay();
		return;
	}else if(GetRecordeVoices_PthreadState() ==PLAY_TULING_E){
		NetStreamExitFile();
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
	int endtime;
	time_t t;
	if(gpioState==VOLKEYDOWN&&GetRecordeVoices_PthreadState()==RECODE_PAUSE){	//按下
		DEBUG_EVENT("gpioState(%d)...\n",gpioState);
		start_speek_wait();
		if(CreateRecorderFile()){		//创建音频文件节点，将插入到链表当中
			pause_record_audio(6);
		}
		speek->Starttime=time(&t);
		start_event_talk_message();
	}else if(gpioState==VOLKEYUP){			//弹起
		DEBUG_EVENT("state(%d)\n",gpioState);
		start_speek_wait();
		endtime=time(&t);
		if((endtime - speek->Starttime)<2){		//时间太短
			pause_record_audio(7);
			shortVoicesClean();
			Create_PlaySystemEventVoices(SEND_ERROR_PLAY);
			return ;
		}else if((endtime - speek->Starttime)>10){		//时间太长
			pause_record_audio(7);
			shortVoicesClean();
			Create_PlaySystemEventVoices(SEND_ERROR_PLAY);
			return ;
		}
		/********************************************
		注 :需要写文件头信息 
		*********************************************/
		StopRecorder_AndSendFile();
		pause_record_audio(8);
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
			pause_record_audio(9);
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
		pause_record_audio(10);
	}
	pthread_mutex_unlock(&speek->mutex);
}
#endif

#ifdef LOCAL_MP3
static void *waitLoadMusicList(void *arg){
	int timeout=0;
	char dirBuf[128]={0};
	sleep(10);
	while(++timeout<20){
		if(!access(TF_SYS_PATH, F_OK)){		//检查tf卡
			break;
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

