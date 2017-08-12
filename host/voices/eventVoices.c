#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/pool.h"
#include "base/cJSON.h"
#include "host/voices/callvoices.h"
#include "host/ap_sta.h"
#include "nvram.h"
#include "../../net/network.h"
#include "host/voices/WavAmrCon.h"
#include "systools.h"
#include "gpio_7620.h"

#include "host/studyvoices/qtts_qisc.h"
#include "huashangMusic.h"
#include "uart/uart.h"
#include "config.h"
#include "log.h"

SysMessage sysMes;
static Speek_t *speek=NULL;

//------------------------config network and set system network state---------------------------------------------------------
static void CloseWifi(void){
	if(sysMes.wifiState==0){
		sysMes.wifiState=1;
		system("ifconfig ra0 down &");
	}
}
void OpenWifi(void){
	if(sysMes.wifiState==1){
		sysMes.wifiState=0;
		system("ifconfig ra0 up &");
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
@ enablePlayVoices :用于控制播放wifi 断网状态语音
@ 0: 连接网络正常  -1: 连接网络失败
*/
int checkNetWorkLive(unsigned char enablePlayVoices){
	if(getNetWorkLive()==NETWORK_ER||getNetWorkLive()==NETWORK_UNKOWN||getNetWorkLive()==NETWORK_RESTART){
		//播报台本
		if(getEventNum()>0){	//检查是否添加过事件
			return -1;
		}
		//添加系统音进去播放，提示用户进行联网
		if(enablePlayVoices==ENABLE_CHECK_VOICES_PLAY)
			Create_PlaySystemEventVoices(CMD_12_NOT_NETWORK);
		return -1;
	}else if(getNetWorkLive()==NETWORK_OK){
		return 0;
	}
	return -1;
}
/*******************************************************
函数功能: 查看NetManger进程是否存在pid号
参数: pid_name	进程名字
返回值: 无
********************************************************/
static int Compare_PIDName(char *pid_name){
	if(!strcmp(pid_name,"NetManger")){
		return 0;
	}
	return -1;
}
//重新运行NetManger 这个进程
static void Restart_RunNetManger(void){
	system("NetManger -t 2 -wifi on &");
	sleep(1);
}
//检查后台联网进程(NetManger)运行状态
void CheckNetManger_PidRunState(void){
	if(judge_pid_exist(Compare_PIDName)){
		remove(NET_SERVER_FILE_LOCK);
		remove(INTEN_NETWORK_FILE_LOCK);	
		Restart_RunNetManger();
	}
}
//短按键按下获取wifi 名字并播放
void ShortKeyDown_ForPlayWifiMessage(void){
	if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
		if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//检查网络
			return;
		}
		char wifiMessage[128]={0};
		char *wifi = nvram_bufget(RT2860_NVRAM, "ApCliSsid");
		if(strlen(wifi)<=0){
			printf("read wifi failed \n");
			return ;
		}
#ifdef DEBUG_PLAY_SYSTEM_IP
		char IP[20]={0};
		GetNetworkcardIp((char * )"apcli0",IP);
		snprintf(wifiMessage,128,"已连接 wifi %s  IP地址是 %s",wifi,IP);
#else
		snprintf(wifiMessage,128,"已连接 wifi %s ",wifi);
#endif	
		Create_PlayQttsEvent(wifiMessage,QTTS_UTF8);
	}
}

/*******************************************************
函数功能: 长按键触发配网事件
参数: 无
返回值: 无
********************************************************/
int LongNetKeyDown_ForConfigWifi(void){
	CheckNetManger_PidRunState();
	WiterSmartConifg_Log("Network key down","ok");
	if(!checkInternetFile()){
		WiterSmartConifg_Log("startSmartConfig checkInternetFile","failed");
		return -1;
	}
	if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){//处于播放事件当中，不予许配网
		if(access(INTEN_NETWORK_FILE_LOCK,F_OK)<0){
			disable_gpio();
			Create_PlaySystemEventVoices(CMD_15_START_CONFIG);
			sysMes.lockRestartNetwork=RESTART_NETWORK_UNLOCK;
			//startSmartConfig(Create_PlaySystemEventVoices,enable_gpio);	
			system("smartconfig start &");
			return 0;
		}
	}	
	printf("startSmartConfig  failed is not RECODE_PAUSE\n");
	WiterSmartConifg_Log("startSmartConfig  failed ","is not RECODE_PAUSE");
	return -1;
}
//连接成功设置工作指示灯,更新muc时间
static void Link_NetworkOk(void){
	Stop_light_500Hz();
	led_lr_oc(openled);
	SocSendMenu(3,0);			//发送本地时间给mcu
	usleep(100*1000);
	setNetWorkLive(NETWORK_OK);		//设置联网状态
}
//联网失败,闪烁指示灯
static void Link_NetworkError(void){
	Running_light_500Hz();
	setNetWorkLive(NETWORK_ER);
}
static void WarnIng_notRecvConetNetwork(void){
	sleep(20);
	unlockRecoderPthread_TimeoutCheck();
}
static void NetWorkConnetIngPlayVoices(int playEventNums){
	char file[64]={0};
	PlayImportVoices(AMR_16_CONNET_ING,playEventNums);
	sleep(1);
	PlayImportVoices(AMR_17_NETWORK_1,playEventNums);
	sleep(1);
	int i=(18+(int)(2.0*rand()/(RAND_MAX+1.0)));
	snprintf(file,64,"qtts/%d.amr",i);
	PlayImportVoices(file,playEventNums);
	if(getlockRecoderPthread_TimeoutCheck()==TIME_OUT_CHECK_LOCK){
		pthread_create_attr(WarnIng_notRecvConetNetwork,NULL);
	}
}

//--------------end config network and set system network state---------------------------------------------------------
/*******************************************************
函数功能: 添加网络URL地址到队列当中进行播放
参数: data 播放歌曲信息，已经malloc申请内存了	
返回值: 无
********************************************************/
int __AddNetWork_UrlForPaly(Player_t *player){
	if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//检查网络
		goto exit0;
	}
	if(getEventNum()>0||getplayEventNum()>0){//防止添加过快
		DEBUG_EVENT("num =%d getplayEventNum() =%d\n",getEventNum(),getplayEventNum());
		WritePlayUrl_Log("__AddNetWork_UrlForPaly"," getEventNum ");
		goto exit0;
	}
	if(GetRecordeVoices_PthreadState() == START_SPEEK_VOICES||GetRecordeVoices_PthreadState() == START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() == END_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==PLAY_WAV||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){	
		WritePlayUrl_Log("__AddNetWork_UrlForPaly"," GetRecordeVoices_PthreadState failed ");
		return -1;
	}
	int ret=-1;
	HandlerText_t *handEvent = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handEvent){
		WritePlayUrl_Log("url"," add ok");
		handEvent->data = (char *)player;
		handEvent->event=URL_VOICES_EVENT;
		handEvent->EventNums=updateCurrentEventNums();
		ret = AddworkEvent(handEvent,sizeof(HandlerText_t));
	}else{
		goto exit0;
	}
	return ret;
exit0:
	free(player);
	return -1;
}
/*******************************************************
函数功能: 添加本地音乐到队列进行播放
参数: localpath 本地MP3播放路径
EventSource:事件来源 (外部事件触发/自身触发)
返回值: 0添加成功 -1添加失败
********************************************************/
int __AddLocalMp3ForPaly(const char *localpath,unsigned char EventSource){
	if(getEventNum()>0){	//事件任务过多，直接丢掉，防止添加过快，导致后面清理时间过长
		DEBUG_EVENT("num =%d \n",getEventNum());
		return -1;
	}
	if(GetRecordeVoices_PthreadState() == START_SPEEK_VOICES||GetRecordeVoices_PthreadState() == START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() == END_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==PLAY_WAV||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){	
		return -1;
	}
	int ret=-1;
	HandlerText_t *handEvent = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));	
	if(handEvent){
		Player_t *player= (char *)calloc(1,sizeof(Player_t));
		if(player==NULL){
			perror("calloc error !!!");
			goto exit0;
		}
		player->playListState=EventSource;
		sprintf(player->playfilename,"%s",localpath);	
		handEvent->EventNums=updateCurrentEventNums();
		handEvent->data = (char *)player;
		handEvent->event=LOCAL_MP3_EVENT;	
		ret = AddworkEvent(handEvent,sizeof(HandlerText_t));
	}
	return ret;	
exit0:
	free(handEvent);
	return -1;
}
int __AddLocalMp3_autoPlay(Player_t *player){
	if(getEventNum()>0||getplayEventNum()>0){//防止添加过快
		DEBUG_EVENT("num =%d getplayEventNum() =%d\n",getEventNum(),getplayEventNum());
		WritePlayUrl_Log("__AddLocalMp3_autoPlay"," getEventNum ");
		goto exit0;
	}
	if(GetRecordeVoices_PthreadState() == START_SPEEK_VOICES||GetRecordeVoices_PthreadState() == START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() == END_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==PLAY_WAV||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){ 
		WritePlayUrl_Log("__AddNetWork_UrlForPaly"," GetRecordeVoices_PthreadState failed ");
		return -1;
	}
	int ret=-1;
	HandlerText_t *handEvent = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handEvent){
		WritePlayUrl_Log("__AddLocalMp3_autoPlay"," add ok");
		handEvent->data = (char *)player;
		handEvent->event=URL_VOICES_EVENT;
		handEvent->EventNums=updateCurrentEventNums();
		ret = AddworkEvent(handEvent,sizeof(HandlerText_t));
	}else{
		goto exit0;
	}
	return ret;
exit0:
	free(player);
	return -1;

}
void CreateDirMenuPlay(int MenuIndex,const char *playName){
	if(getEventNum()>0){	//事件任务过多，直接丢掉，防止添加过快，导致后面清理时间过长
		DEBUG_EVENT("num =%d \n",getEventNum());
		return -1;
	}
	if(GetRecordeVoices_PthreadState() == START_SPEEK_VOICES||GetRecordeVoices_PthreadState() == START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() == END_SPEEK_VOICES||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){	
		return -1;
	}
	int ret=-1;
	HandlerText_t *handEvent = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));	
	if(handEvent){
		Player_t *player= (char *)calloc(1,sizeof(Player_t));
		if(player==NULL){
			perror("calloc error !!!");
			goto exit0;
		}
		player->playListState=EXTERN_PLAY_EVENT;
		sprintf(player->playfilename,"%s",playName);
		handEvent->playLocalVoicesIndex=MenuIndex;
		handEvent->EventNums=updateCurrentEventNums();
		handEvent->data = (char *)player;
		handEvent->event=DIR_MENU_PLAY_EVENT;	
		ret = AddworkEvent(handEvent,sizeof(HandlerText_t));
	}
	return ret; 
exit0:
	free(handEvent);
	return -1;
}
/**
处理按下播放暂停键
**/
void KeydownEventPlayPause(void){
	keydown_flashingLED();	
	//updateCurrentEventNums();
	if(GetRecordeVoices_PthreadState()==PLAY_MP3_MUSIC){
		keyStreamPlay();
	}
}
//按下目录按键切换菜单，并播放歌曲
void SelectDirMenu(void){
	//printf("%s: start get music \n",__func__);
	updateCurrentEventNums();
	Huahang_SelectDirMenu();
}

/*******************************************************
函数功能: 清理URL事件
参数: 无
返回值: 无
********************************************************/
void Create_CleanUrlEvent(void){
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->event=SET_RATE_EVENT;
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}
}
/*******************************************************
函数功能: 创建播放QTTS语音事件 --->文本转语音并播放
参数: txt QTTS文本 type :0---GBK 格式 1----UTF8 格式
返回值: 无
********************************************************/
void Create_PlayQttsEvent(const char *txt,int type){
	unsigned char mixMode =NORMAL_PLAY_PCM;
	if (checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//检查网络
		return;
	}	
	if(GetRecordeVoices_PthreadState() ==START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==END_SPEEK_VOICES){
		return;
	}else if (GetRecordeVoices_PthreadState() == PLAY_MP3_MUSIC||GetRecordeVoices_PthreadState() ==SOUND_MIX_PLAY){	//当前播放歌曲
		mixMode =MIX_PLAY_PCM;
	}
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->EventNums= updateCurrentEventNums();
		handtext->mixMode=mixMode;
		handtext->data= (char *)calloc(1,strlen(txt)+4);
		if (handtext->data){
			sprintf(handtext->data,"%s%s",txt,",");	//文本尾部添加",",保证文本播报出来
			handtext->event =QTTS_PLAY_EVENT;
			handtext->playLocalVoicesIndex=type;
			AddworkEvent((const char *)handtext,sizeof(HandlerText_t));
		}
	}
}
//播放QTTS 事件
void Handler_PlayQttsEvent(HandlerText_t *handText){
	char xunPlayname[24]={0};
	int playSpeed=50;	//默认播放速度为5
#if defined(HUASHANG_JIAOYU)
	Huashang_GetPlayVoicesName(xunPlayname,&playSpeed);	
	if(!strncmp(xunPlayname,"tuling",6)){
		memset(xunPlayname,0,24);
		snprintf(xunPlayname,24,"%s","vinn");
	}
#else
	snprintf(xunPlayname,24,"%s","vinn");
#endif
	if(handText->mixMode==NORMAL_PLAY_PCM){
		start_event_play_wav();
		if(!PlayQttsText(handText->data,handText->playLocalVoicesIndex,(const char *)xunPlayname,handText->EventNums,playSpeed)){
			pause_record_audio();
		}
	}else{
		char outFile[]="qtts.pcm";//混音处理
		if(!QttsTextVoicesFile(handText->data,handText->playLocalVoicesIndex,(const char *)xunPlayname,handText->EventNums,playSpeed,outFile)){
			__playResamplePlayPcmFile((const char *)outFile,handText->EventNums);
			remove(outFile);
		}
	}
	free((void *)handText->data);
	free((void *)handText);
}
/*******************************************************
函数功能: 会话按键按下信号,启动录音 
参数: 无
返回值: 无
********************************************************/
void TulingKeyDownSingal(void){
	updateCurrentEventNums();
	printf("---updateCurrentEventNums start \n");
	Write_Speekkeylog((const char *)"TulingKeyDownSingal",0);
	//处于微信对讲状态，直接退出	
	if(GetRecordeVoices_PthreadState()==START_SPEEK_VOICES||GetRecordeVoices_PthreadState()==END_SPEEK_VOICES||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){		
		Write_Speekkeylog((const char *)"START_SPEEK_VOICES",GetRecordeVoices_PthreadState());
		return;
	}	
	Lock_EventQueue();
	Write_Speekkeylog((const char *)"PLAY_MP3_MUSIC",GetRecordeVoices_PthreadState());
	printf("---NetStreamExitFile start \n");
	int lock =0;
	while(GetLockRate()){
		printf("%s: GetWm8960Rate =%d\n",__func__,GetWm8960Rate());
		usleep(100000);
		lock=1;
		if(GetWm8960Rate()==RECODE_RATE){
			printf("error exit rate ......\n");
			break;
		}
	}
	if(lock){
		goto exit0;
	}
	if(getLockNetwork()){
		//printf("..........\nerror lock network \n ...........\n ");
		printf("..........\n error exit ok \n ...........\n ");
		goto exit0;
	}
	NetStreamExitFile();//退出歌曲播放,并切换采样率	
	printf("---SetWm8960Rate start \n");
	if(SetWm8960Rate(RECODE_RATE,(const char *)"TulingKeyDownSingal set rate")){	//切换采样率失败，退出(防止多线程当中切换，资源冲突问题)
		goto exit0;
	}
	printf("---Unlock_EventQueue start \n");
	Unlock_EventQueue();
	if (checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//检查网络,没有网络直接退出播放
		return;
	}
	Show_KeyDownPicture();
	StartTuling_RecordeVoices();	
	keyDown_AndSetGpioFor_play();
	Write_Speekkeylog((const char *)"StartTuling_RecordeVoices",GetRecordeVoices_PthreadState());
	return ;
exit0:
	Unlock_EventQueue();
}
//关机保存文件和清理工作
void Close_Mtk76xxSystem(void){
	char token[64]={0};
	GetTokenValue(token);
	Save_TulingToken_toRouteTable((const char *)token);
	SaveVol_toRouteTable(GetVol());		//设置声音到路由表
	CloseSystemSignToaliyun();			//发送关机信号给闹钟
	Huashang_closeSystemSavedata();
	system("sync");	//同步数据到sdcard 当中
}
//请求图灵服务器失败，播放本地内容
static void TaiwanToTulingError(unsigned int playEventNums){
	char buf[32]={0};
	char musictype[12]={0};
	int i=(29+(int)(3*rand()/(RAND_MAX+1.0)));
	snprintf(buf,32,"qtts/%d.amr",i);
	PlaySystemAmrVoices(buf,playEventNums);
}
/*
@ 没有网络的时候，播放本地系统固定录好台本(按键触发播放)
@
*/
int Handle_PlayTaiBenToNONetWork(unsigned int playEventNums){
	char file[64]={0};
	int i=(12+(int)(3.0*rand()/(RAND_MAX+1.0)));
	snprintf(file,64,"qtts/%d.amr",i);
	return PlaySystemAmrVoices(file,playEventNums);
}
//进入睡眠状态，播放语音提醒
static void System_EntrySleeping(unsigned int playEventNums){
	char file[64]={0};
	int i=(45+(int)(3.0*rand()/(RAND_MAX+1.0)));
	snprintf(file,64,"qtts/%d.amr",i);
	if(!PlaySystemAmrVoices(file,playEventNums)){
		usleep(100000);
		showFacePicture(WAIT_CTRL_NUM1);
		SleepRecoder_Phthread();			
	}	
}
static void Handle_LowBattery(unsigned int playEventNums){
	char file[64]={0};
	int i=(49+(int)(3.0*rand()/(RAND_MAX+1.0)));
	snprintf(file,64,"qtts/%d.amr",i);
	//PlaySystemAmrVoices(file,playEventNums);
	PlayImportVoices(file,playEventNums);
}

/*******************************************************
函数功能: 创建一个播放系统声音事件，
参数: sys_voices 系统音标号	
返回值: 无
********************************************************/
void Create_PlaySystemEventVoices(int sys_voices){
	if(GetRecordeVoices_PthreadState() ==PLAY_MP3_MUSIC){
		Create_CleanUrlEvent();
		usleep(3000);
	}else if(GetRecordeVoices_PthreadState() ==START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==END_SPEEK_VOICES||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){
		return;
	}
	char addsys_voices[128]={0};
	sprintf(addsys_voices,"Create_PlaySystemEventVoices  %d",sys_voices);
	printf("\naddsys_voices =%s\n",addsys_voices);
	Write_huashangTextLog(addsys_voices);
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->EventNums =updateCurrentEventNums();
		handtext->playLocalVoicesIndex =sys_voices;
		handtext->event =SYS_VOICES_EVENT;
		printf("\n  add sys_voices ok=%d\n",sys_voices);
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}	
}
//添加播放过渡音、关机音等重要声音
void Create_PlayImportVoices(int sys_voices){
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->event =SYS_VOICES_EVENT;
		handtext->EventNums=GetCurrentEventNums();
		handtext->playLocalVoicesIndex =sys_voices;
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}
}
void CreateSystemPlay_ProtectMusic(const char *filename){
	char playTicesVoices[48]={0};
	snprintf(playTicesVoices,48,"%s%s",sysMes.localVoicesPath,filename);
	CreatePlayWeixinVoicesSpeekEvent(playTicesVoices);
}
void ReSetSystem(void){
	Mute_voices(MUTE);
	__ReSetSystem();
}
//串口事件回调函数
void UartEventcallFuntion(int event){
	updateCurrentEventNums();
	switch(event){
		case UART_EVENT_CLOSE_SYSTEM:		//串口发送关机事件
			printf("\n   uart close system v1.1 for fix no delay colse system \n");
			return;
			Mute_voices(MUTE);				//关闭功放
			//Close_Mtk76xxSystem();			//关机处理和保存后台数据
			disable_gpio();					//关闭gpio中断功能,防止关机过程再产生按键事件
			cleanQuequeEvent();				//清理事件队列，保证能够播放语音
			if(GetRecordeVoices_PthreadState() ==PLAY_MP3_MUSIC){
				printf("Create Clean url event \n");
				Create_CleanUrlEvent();
			}
			Create_PlaySystemEventVoices(CMD_26_BIND_PLAY);	
			Lock_EventQueue();
			showFacePicture(CLOSE_SYSTEM_PICTURE);	
			Stop_light_500Hz();
			close_sys_led();
			led_lr_oc(closeled);
			break;
		case UART_EVENT_LOW_BASTERRY:		//电量低提醒
			CreateSystemPlay_ProtectMusic(AMR_49_POWER_LOW);
			break;
		case  AC_BATTERRY:	//正在充电
			CreateSystemPlay_ProtectMusic(AMR_52_POWER_AC);	
			break;
		case POWER_FULL:
			CreateSystemPlay_ProtectMusic(AMR_53_POWER_FULL);
			break;			
		case BATTERRY:		//电池供电
			//Create_PlaySystemEventVoices(CMD_54_POWER_INTERRUPT);	
			CreateSystemPlay_ProtectMusic(AMR_54_POWER_DISCONNET);
			break;
	}
} 
//--------------------------------------------------------------------------------------------------------
//获取本地默认json 歌曲url地址
static int GetDefaultMusicJson_forPlay(char *getUrlBuf,const char *musicType){
	int ret =-1;
	char *readBuf = readFileBuf((const char * )DEFALUT_URL_JSON);
	if(readBuf==NULL){
		return -1;
	}
	cJSON * pJson = cJSON_Parse(readBuf);
	if(NULL == pJson){
		goto exit0;
	}
	cJSON * pArray =cJSON_GetObjectItem(pJson, musicType);
	if(NULL == pArray){
		printf("cJSON_Parse DEFALUT_URL_JSON failed \n");
		goto exit1;
	}
	int iCount = cJSON_GetArraySize(pArray);
	//printf("name	 iCount == %d \n",iCount);
	int randMax=(float)iCount-1;
	time_t ti = time(NULL);
	int randNums = ((int)ti)%randMax;
#if 0
	int i=0;
	for (i=0; i < iCount; ++i) {
		cJSON* pItem = cJSON_GetArrayItem(pArray, i);
		if (NULL == pItem){
			continue;
		}
		char *name = cJSON_GetObjectItem(pItem,"name")->valuestring;
		printf("name[%d] = %s\n",i,name);
	}
#endif
	cJSON* pItem = cJSON_GetArrayItem(pArray, randNums);
	if(pItem){
		cJSON *cjson = cJSON_GetObjectItem(pItem,"name");
		if(cjson){
			snprintf(getUrlBuf,128,"%s",cjson->valuestring);
			ret =0;
		}
	}
exit1:
	cJSON_Delete(pJson);
exit0:
	free(readBuf);
	return ret;
}
//创建播放默认url歌曲
void CreatePlayDefaultMusic_forPlay(const char* musicType){
	Player_t *player = (Player_t *)calloc(1,sizeof(Player_t));
	if(player==NULL){
		perror("calloc error !!!");
		return;
	}	
	player->musicTime=100;
	player->playListState=AUTO_PLAY_EVENT;
	snprintf(player->musicname,64,"%s",musicType);//musicname 暂时定义采用这个结构成语变量存放播放类型		
	if(Huashang_CreatePlayDefaultMusic_forPlay(player->playfilename,musicType)){
		return ;
	}
	__AddLocalMp3_autoPlay(player);
}
static int playLongNotuserVoices(int enablePlayVoices,unsigned int playEventNums){
	int ret =0;
	if(enablePlayVoices){
		if(checkNetWorkLive(DISABLE_CHECK_VOICES_PLAY)){
			ret = Handle_PlayTaiBenToNONetWork(playEventNums);
		}else{
			ret = PlaySystemAmrVoices(AMR_40_NOT_USR,playEventNums);
		}
	}else{
		
	}
	return ret;
}
//客户后台定制推送的内容
void Custom_Interface_RunPlayVoices(int enablePlayVoices,int playVoicesType,unsigned int playEventNums){
	int ret =-1;
	char musictype[12]={0};
	int randNums=0;
	if(playVoicesType){	
		int state= GetWeixinMessageFlag();
		switch(state){
			case NOT_MESSAGE:
				break;
			case WEIXIN_MESSAGE:
				PlaySystemAmrVoices(AMR_24_NEW_MESSAGE, playEventNums);
				goto exit0;
			case WEIXIN_PUSH_MESSAGE:
				PlaySystemAmrVoices(AMR_25_NEW_STROY, playEventNums);
				goto exit0;
		}
		time_t timep;
		struct tm *p;
		time(&timep);
		p=localtime(&timep);
		int times =0;
		if(p->tm_hour>16){
			times = p->tm_hour-16;
		}else{
			times=p->tm_hour+8;
		}
		if(times>=21){
			ret =PlaySystemAmrVoices(TIMEOUT_sleep,playEventNums);
			snprintf(musictype,12,"%s","sleep");	//播放音乐内容
			goto exit1;
		}
		randNums=(1+(int)(13.0*rand()/(RAND_MAX+1.0)));
	}else{
		randNums=(1+(int)(11.0*rand()/(RAND_MAX+1.0)));
	}
	
	start_event_play_wav();
	switch(randNums){
		case 1:
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_41_LISTEN_MUSIC,playEventNums);
			snprintf(musictype,12,"%s","music");	//播放音乐内容
			break;
		case 2:
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_42_LISTEN_GUOXUE,playEventNums);
			snprintf(musictype,12,"%s","guoxue");	//播放国学内容
			break;
		case 3:
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_43_LISTEN_CHENGYU,playEventNums);
			snprintf(musictype,12,"%s","gushi");	//播放成语故事
			break;
		case 4:
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_38_AI_STROY_3,playEventNums);
			snprintf(musictype,12,"%s","baike");	//播放百科知识
			break;
		case 5:
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_35_AI_STROY_0,playEventNums);
			snprintf(musictype,12,"%s","tangshi");	//播放百科知识
			break;
		case 6:
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_88_KEXUE,playEventNums);
			snprintf(musictype,12,"%s","kexue");	//播放百科知识
			break;
		case 7:			
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_89_SHANGXIA,playEventNums);
			snprintf(musictype,12,"%s","shangxia");	//播放百科知识
			break;
		case 8:			
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_90_HISTORY,playEventNums);
			snprintf(musictype,12,"%s","history");	//播放百科知识
			break;
		case 9:			
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_91_TONGYAO,playEventNums);
			snprintf(musictype,12,"%s","tongyao");	//播放百科知识
			break;	
		case 10:			
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_92_EN_MUSIC,playEventNums);
			snprintf(musictype,12,"%s","enmusic");	//播放百科知识
			break;
		case 11:			
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;	//异常打断退出
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_93_GUDIAN,playEventNums);
			snprintf(musictype,12,"%s","gudian");	//播放百科知识
			break;			
		case 12:
			if(checkNetWorkLive(DISABLE_CHECK_VOICES_PLAY)){
				ret = Handle_PlayTaiBenToNONetWork(playEventNums);
			}else{
				ret =PlaySystemAmrVoices(AMR_26_BIND,playEventNums);
			}
			goto exit0;
		case 13:
			if(checkNetWorkLive(DISABLE_CHECK_VOICES_PLAY)){
				ret = Handle_PlayTaiBenToNONetWork(playEventNums);
			}else{
				ret =PlaySystemAmrVoices(AMR_44_WEIXIN_WARN,playEventNums);
			}
			goto exit0;
		default:
			if(playLongNotuserVoices(enablePlayVoices,playEventNums)){
				goto exit0;
			}
			start_event_play_wav();
			ret =PlaySystemAmrVoices(AMR_41_LISTEN_MUSIC,playEventNums);
			snprintf(musictype,12,"%s","music");	//播放音乐内容
			break;
	}
exit1:	
	if(!ret){
		//printf("%s: CreatePlayDefaultMusic_forPlay musictype=%s\n",__func__,musictype);
		CreatePlayDefaultMusic_forPlay(musictype);	//musictype 
	}
exit0:
	return;
}

/***
播放列表歌曲，按键控制单曲还是列表播放 
data:播放的数据
musicType:音乐类型  网络歌曲/本地歌曲
***/
void CreatePlayListMuisc(const void *data,int musicType){
	Player_t * player =NULL;
	if(PLAY_MUSIC_NETWORK==musicType){
		__AddNetWork_UrlForPaly((Player_t *)data);
	}else if(PLAY_MUSIC_SDCARD==musicType){
		player= (Player_t *)data;
		__AddLocalMp3ForPaly((const char *)player->playfilename,player->playListState);
	}
}
//--------------------------------------------------------------------------------------------------------
void *updateHuashangFacePthread(void *arg){
	int eventNums = *(int *)arg;
	unlockRecoderPthread_TimeoutCheck();
	int i=0;
	for(i=0;i<3;i++){
		if(eventNums!=GetCurrentEventNums()){
			break;
		}
		sleep(1);
	}
	if(eventNums==GetCurrentEventNums()){
		showFacePicture(PLAY_MUSIC_NUM4);	
	}
	Link_NetworkOk();						//连接成功关灯，开灯，状态设置
	enable_gpio();
}
//smartconfig not recv wifi message, restart network
static void *RunTask_restartNetwork(void *arg){
	sleep(20);
	if(sysMes.lockRestartNetwork==RESTART_NETWORK_UNLOCK){
		return ;
	}
	disable_gpio();
	sysMes.startCheckNetworkFlag=1;
	Create_PlaySystemEventVoices(CMD_83_SMART_NOT_WIFI);
	system("smartconfig restart &");
	sysMes.lockRestartNetwork=RESTART_NETWORK_UNLOCK;
}
/*******************************************************
函数功能: 系统音事件处理函数
参数: sys_voices 系统音标号
返回值: 无
********************************************************/
void Handle_PlaySystemEventVoices(int sys_voices,unsigned int playEventNums){
	int vol=0;
	switch(sys_voices){
		case CMD_11_START:
			PlayImportVoices(AMR_11_START_SYSTEM_OK,playEventNums);
			break;
		case CMD_12_NOT_NETWORK:				//12、小朋友你可以让爸爸妈妈帮我连接网络，我才会更聪明哦！
			Handle_PlayTaiBenToNONetWork(playEventNums);
			enable_gpio();
			break;
		case CMD_15_START_CONFIG:				//15、开始配网，请发送wifi名以及密码！
			Running_light_500Hz();
			lockRecoderPthread_TimeoutCheck();
			if(getNetWorkLive()==NETWORK_OK){
				setNetWorkLive(NETWORK_RESTART);
			}else{
				setNetWorkLive(NETWORK_ER);
			}
			PlaySystemAmrVoices(AMR_15_START_CONFIG,playEventNums);
			break;
		case CMD_16_CONNET_ING:					//16、正在尝试连接网络，请稍等！
			NetWorkConnetIngPlayVoices(playEventNums);
			break;
		case CMD_20_CONNET_OK:					//20、(8634代号)小培老师与总部课堂连接成功，我们来聊天吧！（每次连接成功的语音，包括唤醒）
			pool_add_task(updateHuashangFacePthread,&playEventNums);	
			showFacePicture(CONNECT_WIFI_OK_PICTURE);	
			sleep(1);
			PlaySystemAmrVoices(AMR_20_CONNET_OK,playEventNums);
			break;
		case CMD_21_NOT_SCAN_WIFI:				//21、无法扫描到您的wifi,请检查您的网络
			unlockRecoderPthread_TimeoutCheck();
			PlaySystemAmrVoices(AMR_21_NOT_SCAN_WIFI,playEventNums);
			enable_gpio();
			break;	
		case CMD_22_NOT_RECV_WIFI:				//22、没有收到你发送的wifi,请重新发送一遍
			unlockRecoderPthread_TimeoutCheck();
			PlaySystemAmrVoices(AMR_22_NOT_RECV_WIFI,playEventNums);
			if(sysMes.lockRestartNetwork==RESTART_NETWORK_LOCK)
				break;
			if(getNetWorkLive()==NETWORK_RESTART){
				sysMes.lockRestartNetwork=RESTART_NETWORK_LOCK;
				pthread_create_attr(RunTask_restartNetwork,NULL);
			}
			break;
		case CMD_23_NOT_WIFI:
			unlockRecoderPthread_TimeoutCheck();
			PlaySystemAmrVoices(AMR_23_CHECK_NETWORk,playEventNums);			
			Link_NetworkError();
			enable_gpio();
			break;
		case CMD_24_WAKEUP_RECV_MSG:			//24、你有新消息，请按信息键听取吧！（唤醒之后播放，播放网络成功之后）
			PlaySystemAmrVoices(AMR_24_NEW_MESSAGE,playEventNums);
			break;		
		case CMD_25_WAKEUP_RECV_MSG:			//25、你有新故事未听取,按信息键开始听吧！（唤醒之后播放，播放网络成功之后）
			PlaySystemAmrVoices(AMR_25_NEW_STROY,playEventNums);
			break;		
		case CMD_26_BIND_PLAY:					//26、小朋友请让爸爸在微信界面当中邀请小伙伴一起来聊天吧！
			PlaySystemAmrVoices(AMR_26_BIND,playEventNums);
			break;
		case CMD_27_RECV_BIND:					//27、成功收到小伙伴的绑定请求。
			PlaySystemAmrVoices(AMR_27_RECV_BIND,playEventNums);
			break;	
		case CMD_28_HANDLE_BIND:				//28、成功处理小伙伴的绑定请求。
			PlaySystemAmrVoices(AMR_28_BIND_OK,playEventNums);
			break;
		case CMD_29_NETWORK_FAILED:				//29、当前网络环境差，语音发送失败，请检查网络！
			PlaySystemAmrVoices(AMR_29_NETWORK_FAILED,playEventNums);
			break;		
		case CMD_35_39_REQUEST_FAILED:					//请求服务器超时，播放本地已经录制好的音频
			TaiwanToTulingError(playEventNums);
			break;
		case CMD_39_REQUEST_FAILED:
			PlaySystemAmrVoices(AMR_39_AI_STROY_4,playEventNums);
			break;
		case CMD_40_NOT_USER_WARN: 				//40、小朋友，你去哪里了，请跟我一起来玩吧！！
			Custom_Interface_RunPlayVoices(1,1,playEventNums);
			break;
		case CMD_44_WEIXIN_WARN:
			PlaySystemAmrVoices(AMR_44_WEIXIN_WARN,playEventNums);
			break;
		case CMD_4547_SLEEP:					//45、亲我先去休息了，当你想我的时候，记得叫醒我喔!
			System_EntrySleeping(playEventNums);	
			break;	
		case CMD_48_TF_ERROT_PLAY:				//48、没有读到本地内容，请联系总部!
			PlaySystemAmrVoices(AMR_48_NOT_SCARD,playEventNums);
			break;
		case CMD_4951_POWER_LOW:					//49、我饿了，请帮我充电吧!
			Handle_LowBattery(playEventNums);
			break;
		case CMD_52_POWER_AC:					//52、正在补充能量
			PlaySystemAmrVoices(AMR_52_POWER_AC,playEventNums);
			break;
		case CMD_53_POWER_FULL:					//53、能量补充完毕
			PlaySystemAmrVoices(AMR_53_POWER_FULL,playEventNums);
			break;
		case CMD_54_POWER_INTERRUPT:			//54、能量补充断开
			PlaySystemAmrVoices(AMR_54_POWER_DISCONNET,playEventNums);
			break;
		case CMD_55_NEW_VERSION:				//55、发现新程序版本，正在升级，请不要关机。
			PlaySystemAmrVoices(AMR_55_NEW_VERSION,playEventNums);
			break;
		case CMD_56_RESET_SYSTEM:				//59、亲，我已经恢复到最初状态，正在重新启动。
			PlaySystemAmrVoices(AMR_56_RESET,playEventNums);
			usleep(500000);
			ReSetSystem();
			break;
		case CMD_6175_DIR_MENU:
			PlaySystemAmrVoices(AMR_61_DIR,playEventNums);
			break;
		case CMD_62_DIR_MENU:
			PlaySystemAmrVoices(AMR_62_DIR,playEventNums);
			break;
		case CMD_63_DIR_MENU:
			PlaySystemAmrVoices(AMR_63_DIR,playEventNums);
			break;
		case CMD_64_DIR_MENU:
			PlaySystemAmrVoices(AMR_64_DIR,playEventNums);
			break;
		case CMD_65_DIR_MENU:
			PlaySystemAmrVoices(AMR_65_DIR,playEventNums);
			break;
		case CMD_66_DIR_MENU:
			PlaySystemAmrVoices(AMR_66_DIR,playEventNums);
			break;
		case CMD_67_DIR_MENU:
			PlaySystemAmrVoices(AMR_67_DIR,playEventNums);
			break;
		case CMD_68_DIR_MENU:
			PlaySystemAmrVoices(AMR_68_DIR,playEventNums);
			break;
		case CMD_69_DIR_MENU:
			PlaySystemAmrVoices(AMR_69_DIR,playEventNums);
			break;
		case CMD_70_DIR_MENU:
			PlaySystemAmrVoices(AMR_70_DIR,playEventNums);
			break;
		case CMD_71_DIR_MENU:
			PlaySystemAmrVoices(AMR_71_DIR,playEventNums);
			break;
		case CMD_72_DIR_MENU:
			PlaySystemAmrVoices(AMR_72_DIR,playEventNums);
			break;
		case CMD_73_DIR_MENU:
			PlaySystemAmrVoices(AMR_73_DIR,playEventNums);
			break;
		case CMD_74_DIR_MENU:
			PlaySystemAmrVoices(AMR_74_DIR,playEventNums);
			break;	
		case CMD_75_DIR_MENU:
			PlaySystemAmrVoices(AMR_75_DIR,playEventNums);
			break;			
		case TULING_WAIT_VOICES:
			vol =GetVol();
			Setwm8960Vol(VOL_APP_SET,PLAY_PASUSE_VOICES_VOL);
			playVoicesNotFace(TULING_KEYUP,playEventNums);
			usleep(2000);
			playVoicesNotFace(TULING_WINT,playEventNums);
			Setwm8960Vol(VOL_SET_VAULE,vol);
			break;
		case CONTINUE_PLAY_MUSIC_VOICES:
			PlaySystemAmrVoices(PLAY_CONTINUE_MUSIC,playEventNums);
			break;
		case CMD_83_SMART_NOT_WIFI:
			unlockRecoderPthread_TimeoutCheck();
			if(!PlaySystemAmrVoices(AMR_83_SMART_NOT_WIFI,playEventNums)){
				sleep(2);
				Custom_Interface_RunPlayVoices(0,0,playEventNums);
			}
			break;
		case CMD_90_UPDATE_OK:						//更新固件结束
			PlaySystemAmrVoices(AMR_UPDATE_OK,playEventNums);
			usleep(500000);
			ReSetSystem();
			break;
		case CMD_110_NOT_NETWORK:			
			Handle_PlayTaiBenToNONetWork(playEventNums);
			PlaySystemAmrVoices(AMR_39_AI_STROY_4,playEventNums);
			enable_gpio();
			break;
		case CMD_111_NOTWIFI_PLAYMUSIC:
			Custom_Interface_RunPlayVoices(0,0,playEventNums);
			unlockRecoderPthread_TimeoutCheck();
			break;
		default:
			pause_record_audio();
			break;
	}
}
//-------end--------播放系统声音有关的、事件的产生、消费处理-----------------------------------------------------
//播放微信发送过来语音文件  filename 发送过来的微信语音文件
void CreatePlayWeixinVoicesSpeekEvent(const char *filename){
	unsigned char mixMode =NORMAL_PLAY_PCM;
	if(GetRecordeVoices_PthreadState() ==START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==END_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==SOUND_MIX_PLAY){
		return;
	}
	else if(GetRecordeVoices_PthreadState() ==PLAY_MP3_MUSIC){
		mixMode =MIX_PLAY_PCM;
	}
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->data= (char *)calloc(1,strlen(filename)+1);
		if(handtext->data==NULL){
			goto exit1;
		}
		sprintf(handtext->data,"%s",filename);
		handtext->event = SPEEK_VOICES_EVENT;
		handtext->mixMode=mixMode;
		if(handtext->mixMode==MIX_PLAY_PCM){	// for play music mode  don't interrupt auto play music 
			handtext->EventNums=GetCurrentEventNums();
		}else{
			handtext->EventNums=updateCurrentEventNums();
		}
		if(AddworkEvent(handtext,sizeof(HandlerText_t))){
			printf("add play amr voices failed ,and remove file \n");
			goto exit2;
		}
	}
	return;
exit2:
	free(handtext->data);
exit1:
	free(handtext);
exit0:
	remove(filename);
}
//先挂载录音再退出
static void shortVoicesClean(void){
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	}
	pthread_mutex_unlock(&speek->mutex);
}
/*******************************************************
函数功能: 创建待发送的文件，并保持到本地当中,写入wav头
参数:	无
返回值: 0 创建成果 -1 创建失败
********************************************************/
static int CreateRecorderFile(void){
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	}
	if((speek->savefilefp= fopen(SAVE_WAV_VOICES_DATA,"w+"))==NULL){
		perror("open send file failed \n");
		pthread_mutex_unlock(&speek->mutex);
		return -1;
	}
	speek->file_len=0;
	fwrite((char *)&pcmwavhdr,WAV_HEAD,1,speek->savefilefp);
	DEBUG_EVENT("create save file \n");
	pthread_mutex_unlock(&speek->mutex);
	return 0;
}
/*******************************************************
函数功能: 停止录音，并将数据发送给app用户
参数:
返回值:
********************************************************/
static void StopRecorder_AndSendFile(unsigned int playEventNums){
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
		showFacePicture(CONNECT_WIFI_OK_PICTURE);
		if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath)){
			DEBUG_EVENT("enc_wav_amr_file failed");
			usleep(100000);
			showFacePicture(WAIT_CTRL_NUM4);
			return;
		}
		DEBUG_EVENT("stop save file \n");
		uploadVoicesToaliyun(filepath,pcmwavhdr.data_size/10+6);
		playVoicesNotFace(AMR_WEIXIN_SEND_OK,playEventNums);	//播放微信提示音，表示音频正在发送	
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
	if(GetRecordeVoices_PthreadState() ==PLAY_MP3_MUSIC){		//打断播放音乐
		Create_CleanUrlEvent();
		return;
	}else if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
		return;
	}else if(GetRecordeVoices_PthreadState() ==PLAY_WAV||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY||GetRecordeVoices_PthreadState()==START_SPEEK_VOICES){
		return;
	}
	CloseWifi();
	DEBUG_EVENT("state %d\n",gpioState);
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){	
		handtext->EventNums = updateCurrentEventNums();
		handtext->event=TALK_EVENT_EVENT;
		handtext->playLocalVoicesIndex=gpioState;
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}
}
/*******************************************************
函数功能:处理微信语音按键事件 
参数: gpioState 0 按下  1 弹起
********************************************************/
void Handle_WeixinSpeekEvent(unsigned int gpioState,unsigned int playEventNums){
	int endtime,voicesTime=0;
	time_t t;
	if(gpioState==KEYDOWN&&GetRecordeVoices_PthreadState()==RECODE_PAUSE){	//按下
		DEBUG_EVENT("gpioState(%d)...\n",gpioState);
		start_speek_wait();
		if(CreateRecorderFile()){		//创建音频文件节点，将插入到链表当中
			pause_record_audio();
		}else{
			speek->Starttime=time(&t);
			start_event_talk_message();
			speek->freeVoiceNums=5;
		}	
		showFacePicture(WAIT_CTRL_NUM2);
	}else if(gpioState==KEYUP){			//弹起
		DEBUG_EVENT("state(%d)\n",gpioState);
		start_speek_wait();
		endtime=time(&t);
		voicesTime = endtime - speek->Starttime;
		start_event_play_wav();
		if(voicesTime<1||voicesTime>10){//时间太短或太长
			shortVoicesClean();
			//PlaySystemAmrVoices(AMR_WEIXIN_SEND_ERROR,playEventNums);
			pause_record_audio();
			return ;
		}else{
			playVoicesNotFace(AMR_WEIXIN_KEY_UP,playEventNums);	// play key up voices
			StopRecorder_AndSendFile(playEventNums);
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
	if(speek->freeVoiceNums>0){
		speek->freeVoiceNums--;
		return;
	}
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
//--------------------------------------sdcard 收藏喜爱歌曲---------------------------------------------------
//检查sdcard 是否挂载，没有挂载，添加系统音播放提示
static int checkSdcard_MountState(void){
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){	
			Create_PlaySystemEventVoices(CMD_48_TF_ERROT_PLAY);	//提示用户，没有检查到sdcard
		}
		return -1;
	}	
	return 0;
}

//----------------------end sdcard 收藏喜爱歌曲--------------------------------------------------------------
//开机加载sdcard 当中数据库信息
static void *waitLoadMusicList(void *arg){
	int timeout=0;
	while(++timeout<18){
		CheckNetManger_PidRunState();	//检查网络服务
		sleep(1);
		if(sysMes.netstate==NETWORK_OK){
			Write_StartLog("connet ok",timeout);
			break;
		}else if(sysMes.netstate==NETWORK_ER){
			enable_gpio();
			Write_StartLog("restart network failed",timeout);
			break;
		}
	}
	if(sysMes.netstate==NETWORK_UNKOWN){	//默认是未知状态，长时间未收到联网进程发送过来的状态，直接使能gpio
		sysMes.netstate=NETWORK_ER;	
		sysMes.startCheckNetworkFlag=1;
		Write_StartLog("unkown network ",timeout);
		Create_PlaySystemEventVoices(CMD_110_NOT_NETWORK);
		unsigned int currentEvent= GetCurrentEventNums();
		sleep(25);
		if(currentEvent==GetCurrentEventNums()){
			Create_PlaySystemEventVoices(CMD_111_NOTWIFI_PLAYMUSIC);
		}
	}
	//Huashang_loadSystemdata();
	return NULL;
} 

/******************************************************************
初始化8960音频芯片，开启8K录音和播放双工模式,初始化gpio，播放开机启动音
*******************************************************************/
void InitMtkPlatfrom76xx(void){
	speek = (Speek_t *)calloc(1,sizeof(Speek_t));
	if(speek==NULL){
		return ;
	}
	speek->savefilefp=NULL;
	speek->file_len=0;
	pthread_mutex_init(&speek->mutex, NULL);

	InitMtk76xx_gpio();
	InitWm8960Voices();
	InitRecord_VoicesPthread();
	initStream(ack_playCtr,WritePcmData,SetWm8960Rate,GetVol,GetWm8960Rate);
	InitEventMsgPthread();
	Huashang_Init();
	Create_PlaySystemEventVoices(CMD_11_START);
	pool_add_task(waitLoadMusicList, NULL);	//防止T卡加载慢
}
/*
@ 
@ 销毁平台相关申请的资源
*/
void CleanMtkPlatfrom76xx(void){
	ExitRecord_Voicespthread();
	System_StateLog("ExitRecord_Voicespthread ok\n");
	CleanEventMsgPthread();
	System_StateLog("CleanEventMsgPthread ok \n");
	DestoryWm8960Voices();
	System_StateLog("DestoryWm8960Voices ok \n");
	CleanMtk76xx_gpio();
	System_StateLog("CleanMtk76xx_gpio ok \n");
	cleanStream();
	System_StateLog("cleanStream ok \n");
}
                 
