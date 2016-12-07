#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/message_wav.h"
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

#define XIMALA_MUSIC_E	"ximalaya/"	//喜马拉雅收藏的歌曲		

#define START_SPEEK_E	START_SPEEK_VOICES 		  	//录音、语音识别
#define PAUSE_E			RECODE_PAUSE 				//录音挂起
#define PLAY_WAV_E		PLAY_WAV					//播放wav原始数据音
#define END_SPEEK_E		END_SPEEK_VOICES			//结束录音
#define PLAY_URL_E		PLAY_URL					//播放url数据

#define DBG_EVENT
#ifdef DBG_EVENT  
#define DEBUG_EVENT(fmt, args...) printf("event : " fmt, ## args)  
#else   
#define DEBUG_EVENT(fmt, args...) { }  
#endif //end DBG_EVENT

extern void DelSdcardMp3file(char * sdpath);

SysMessage sysMes;
/*******************************************************
函数功能: 播放网络URL地址
参数: url URL地址	
返回值: 无
********************************************************/
static void CreateUrlEvent(const void *data){
	playurlLog("url_start\n");
	playurlLog(VERSION);	//版本时间
	if(getEventNum()>0||getplayEventNum()>0){
		DEBUG_EVENT("CreateUrlEvent num =%d \n",getEventNum());
		return;
	}
	if(GetRecordeLive() == PLAY_WAV_E){
		exitqttsPlay();
		return;
	}
	add_event_msg(data,0,URL_VOICES_EVENT);
	playurlLog("add ok\n");
}
#ifdef LOCAL_MP3
/*******************************************************
函数功能: 播放本地地址
参数: localpath 本地MP3播放地址	
返回值: 无
********************************************************/
static void CreateLocalMp3(char *localpath){
	if(getEventNum()>0){
		DEBUG_EVENT("CreateUrlEvent num =%d \n",getEventNum());
		return;
	}
	if(GetRecordeLive() == PLAY_WAV_E){
		exitqttsPlay();
		return;
	}
	char *URL= (char *)calloc(1,strlen(localpath)+1);
	if(URL==NULL){
		perror("calloc error !!!");
		return;
	}
	sprintf(URL,"%s",localpath);
	add_event_msg(URL,0,LOCAL_MP3_EVENT);
}
static void PlayLocal(unsigned char menu,const char *path, unsigned char Mode){	
	char buf[128]={0};
	char filename[64]={0};
	if(GetSdcardMusic((const char *)TF_SYS_PATH,path,filename, Mode)){
		return;
	}
	snprintf(buf,128,"%s%s%s",TF_SYS_PATH,path,filename);
	printf("filepath = %s\n",buf);
	CreateLocalMp3(buf);
	sysMes.localplayname=menu;
}
#endif
/*******************************************************
函数功能: 播放MP3
参数: play 本地MP3播放命令 或 URL地址
返回值: 无
********************************************************/
void createPlayEvent(const void *play,unsigned char Mode){
#ifdef LOCAL_MP3
	if(Mode==PLAY_NEXT_AUTO){
		Mode=PLAY_NEXT;
	}else{
		time_t t;
		sysMes.Playlocaltime=time(&t);
	}
	if(!strcmp((const char *)play,"mp3")){
		PlayLocal(mp3,TF_MP3_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"story")){
		PlayLocal(story,TF_STORY_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"english")){
		PlayLocal(english,TF_ENGLISH_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"guoxue")){
		PlayLocal(guoxue,TF_GUOXUE_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"xiai")){
		PlayLocal(xiai,XIMALA_MUSIC_E,Mode);
	}else{
		CreateUrlEvent(play);
	}
#else
	CreateUrlEvent(play);
#endif	
}
/*******************************************************
函数功能: 清理URL事件
参数: 无
返回值: 无
********************************************************/
void CleanUrlEvent(void){
	sysMes.localplayname=0;
	add_event_msg(NULL,0,SET_RATE_EVENT);
}
/*******************************************************
函数功能: QTTS事件
参数: txt QTTS文本 type :0---GBK 1----UTF8
返回值: 无
********************************************************/
void QttsPlayEvent(const char *txt,int type){
	if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	char *TXT= (char *)calloc(1,strlen(txt)+1);
	if(TXT){
		sprintf(TXT,"%s",txt);
		add_event_msg((const char *)TXT,type,QTTS_PLAY_EVENT);
	}
}

/*******************************************************
函数功能: 会话事件
参数: 无
返回值: 无
********************************************************/
void down_voices_sign(void){
	if(GetRecordeLive() ==START_SPEEK_VOICES||GetRecordeLive() ==END_SPEEK_VOICES){
		DEBUG_EVENT("check_recorde_state ...\n");
		return;
	}
	if(GetRecordeLive()==PLAY_WAV_E){
		exitqttsPlay();
	}else if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}else{
		sysMes.localplayname=0;	
		NetStreamExitFile();
		start_event_std();
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
void CacheNetServer(void){
	sleep(5);
	if(judge_pid_exist(get_pid_name)){
		remove("/var/server.lock");
		remove("/var/internet.lock");
		StartNetServer();
	}
}
/*******************************************************
函数功能: 配网事件
参数: 无
返回值: 无
********************************************************/
void Net_work(void){
	if(judge_pid_exist(get_pid_name)){
		remove("/var/server.lock");
		remove("/var/internet.lock");
		StartNetServer();
	}
	smartConifgLog("smart_start\n");
	if(!checkInternetFile()){
		smartConifgLog("startSmartConfig checkInternetFile failed\n");
		return;
	}
	if(GetRecordeLive()==PAUSE_E){
		disable_gpio();
		startSmartConfig(create_event_system_voices,enable_gpio);
	}
	smartConifgLog("startSmartConfig Net_work ok\n");
}
/*******************************************************
函数功能: 创建一个系统声音事件，播放系统声音
参数: sys_voices 系统音标号	
返回值: 无
********************************************************/
void create_event_system_voices(int sys_voices)
{
	playsysvoicesLog("playsys_start\n");
	if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	add_event_msg(NULL,sys_voices,SYS_VOICES_EVENT);
	playsysvoicesLog("playsys voices end \n");
}
//关机保存文件和清理工作
void CloseSystemWork(void){
	SaveSystemPlayNum();
	DelSdcardMp3file(MP3_SDPATH);
}
/*******************************************************
函数功能: 系统音事件处理函数
参数: sys_voices 系统音标号
返回值: 无
********************************************************/
void handle_event_system_voices(int sys_voices){
	playsysvoicesLog("playsys voices handle \n");
//----------------------系统有关-----------------------------------------------------
	switch(sys_voices){
		case END_SYS_VOICES_PLAY:					//结束音
#ifdef PALY_URL_SD
			pool_add_task(CloseSystemWork,NULL);//关机删除，长时间不用的文件
#endif
			CloseSystemSignToaliyun();			//发送关机信号给闹钟
			play_sys_tices_voices(END_SYS_VOICES);			
			break;
		case TULING_WINT_PLAY:						//请稍等	
#ifdef QITUTU_SHI
			pool_add_task(Led_System_vigue_open,NULL);
#endif
			play_sys_tices_voices(TULING_WINT);
			break;
		case LOW_BATTERY_PLAY:						//低电关机
#ifdef PALY_URL_SD
			pool_add_task(CloseSystemWork,NULL);//关机删除，长时间不用的文件
#endif
			play_sys_tices_voices(LOW_BATTERY);			
			break;
		case RESET_HOST_V_PLAY:						//恢复出厂设置
			play_sys_tices_voices(RESET_HOST_V);
			break;
//----------------------重连有关-----------------------------------------------------
		case REQUEST_FAILED_PLAY:					//重连，请求服务器数据失败
			play_sys_tices_voices(REQUEST_FAILED);
			break;
		case UPDATA_END_PLAY:						//更新固件结束
			play_sys_tices_voices(UPDATA_END);
			//system("sleep 8 && reboot &");
			//clean_resources();
			break;
		case 7:
			play_sys_tices_voices(REQUEST_FAILED);
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
			led_left_right(left,closeled);
			led_left_right(right,closeled);
			play_sys_tices_voices(START_INTERNET);
			break;
		case SMART_CONFIG_OK_PLAY:		//接受密码成功
			play_sys_tices_voices(YES_REAVWIFI);
			break;
		case CONNECT_OK_PLAY:			//连接成功
			play_sys_tices_voices(LINK_SUCCESS);
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
			enable_gpio();
			break;
		case CONNET_CHECK_PLAY:			//检查网络是否可用
			play_sys_tices_voices(CHECK_INTERNET);
			break;
//----------------------对讲有关-----------------------------------------------------
		case SEND_OK_PLAY:			//检查网络是否可用
			play_sys_tices_voices(SEND_OK);
			break;
		case SEND_ERROR_PLAY:			//检查网络是否可用
			play_sys_tices_voices(SEND_ERROR);
			break;
	}
	playsysvoicesLog("playsys voices end \n");
}
#ifdef SPEEK_VOICES
void CreateSpeekEvent(const char *filename){
	if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	char *TXT= (char *)calloc(1,strlen(filename)+1);
	if(TXT){
		sprintf(TXT,"%s",filename);
		add_event_msg(TXT,0,SPEEK_VOICES_EVENT);
	}
}
#endif

#ifdef SPEEK_VOICES
static int file_len=0;
static FILE *savefilefp=NULL;
static void shortVoicesClean(void)
{
	if(savefilefp!=NULL){
		fclose(savefilefp);
		savefilefp=NULL;
	}
}
/*******************************************************
函数功能: 创建待发送的文件，并保持到本地当中
参数:	无
返回值: 0 创建成果 -1 创建失败
********************************************************/
static int create_recorder_file(void)
{
	if((savefilefp= fopen(SAVE_WAV_VOICES_DATA,"w+"))==NULL){
		perror("open send file failed \n");
		return -1;
	}
	file_len=0;
	fwrite((char *)&pcmwavhdr,WAV_HEAD,1,savefilefp);
	DEBUG_EVENT("create save file \n");
	return 0;
}
/*******************************************************
函数功能: 停止录音，并将数据发送给app用户
参数:
返回值:
********************************************************/
static void stop_recorder_tosend_file(void)
{
	char filepath[64];
	pcmwavhdr.size_8 = (file_len+36);
	pcmwavhdr.data_size = file_len;
	if(savefilefp==NULL){
		return;
	}
	DEBUG_EVENT("stop_recorder_tosend_file : fseek file \n");
	fseek(savefilefp,0,SEEK_SET);
	fwrite(&pcmwavhdr,1,WAV_HEAD,savefilefp);
	DEBUG_EVENT("stop_recorder_tosend_file : shortVoicesClean \n");
	shortVoicesClean();
	memset(filepath,0,64);
	time_t t;
	t = time(NULL);
	DEBUG_EVENT("stop_recorder_tosend_file : sprintf \n");
#ifdef CACHE_SDCARD
	sprintf(filepath,"%s%s%d%s",sysMes.sd_path,CACHE_WAV_PATH,(unsigned int)t,".amr");
#else
	sprintf(filepath,"%s%d%s",CACHE_WAV_PATH,(unsigned int)t,".amr");
#endif
	DEBUG_EVENT("stop_recorder_tosend_file : WavToAmr8kFile \n");
	if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath))
	{
		DEBUG_EVENT("enc_wav_amr_file failed");
		return;
	}
	DEBUG_EVENT("stop save file \n");
	uploadVoicesToaliyun(filepath);
	create_event_system_voices(SEND_OK_PLAY);
	//remove(SAVE_WAV_VOICES_DATA);
}
/*******************************************************
函数功能:语音对讲按键按下和弹起事件
参数:state 0 按下  1 弹起
返回值:
********************************************************/
void create_event_voices_key(unsigned int state)
{	
	int endtime;
	time_t t;
	if(state==0){	//按下
		sysMes.Starttime=time(&t);
	}else{			//弹起
		endtime=time(&t);
		if((endtime - sysMes.Starttime)<2){		//时间太短
			pause_record_audio();
			shortVoicesClean();
			return ;
		}else if((endtime - sysMes.Starttime)>10){		//时间太长
			pause_record_audio();
			shortVoicesClean();
			return ;
		}
	}
	if(GetRecordeLive() ==PLAY_URL_E){		//打断播放音乐
		CleanUrlEvent();
		return;
	}
	DEBUG_EVENT("create_event_voices_key ...\n");
	add_event_msg(NULL,state,TALK_EVENT_EVENT);
}
/*******************************************************
函数功能:处理按键事件
参数:
返回值:
********************************************************/
void handle_voices_key_event(unsigned int state)
{
	int i;
	if(state==0&&GetRecordeLive() ==PAUSE_E){	//按下
		DEBUG_EVENT("handle_voices_key_event : state(%d)...\n",state);
		start_event_play_wav();
		if(create_recorder_file())		//创建音频文件节点，将插入到链表当中
		{
			pause_record_audio();
		}
		start_event_talk_message();
	}else if(state==1){			//弹起
		DEBUG_EVENT("handle_voices_key_event : state(%d)\n",state);
		pause_record_audio();
		usleep(100);
		/********************************************
		注 :需要写文件头信息 
		*********************************************/
		stop_recorder_tosend_file();
	}
}
/*******************************************************
函数功能:保存语音数据 (需要将音频数据压缩保存)
参数:  voices_data 原始音频数据  size 原始音频数据大小
返回值:
********************************************************/
void save_recorder_voices(const char *voices_data,int size)
{
	int i=0;
	int endtime;
	time_t t;
	if(savefilefp!=NULL)
	{
		endtime=time(&t);
		if((endtime - sysMes.Starttime)>60)//
		{
			pause_record_audio();
		}
#if 0	//右声道
		for(i=2; i<size; i+=4)		//双声道数据转成单声道数据
		{
			fwrite(voices_data+i,2,1,savefilefp);
		}
#else	//左声道
		for(i=0; i<size; i+=4){
			fwrite(voices_data+i,2,1,savefilefp);
		}
#endif
		i /=2; 
		file_len +=i;
	}
	else{
		pause_record_audio();
	}
}
#endif

#ifdef LOCAL_MP3
static void *waitLoadMusicList(void *arg){
	sleep(15);
	SysOnloadMusicList((const char *)TF_SYS_PATH,(const char *)TF_MP3_PATH,(const char *)TF_STORY_PATH,(const char *)TF_ENGLISH_PATH,(const char *)TF_GUOXUE_PATH);
	CacheNetServer();	//检查网络服务
	return;
} 
#endif
/******************************************************************
初始化8960音频芯片，开启8K录音和播放双工模式
*******************************************************************/
void init_wm8960_voices(void)
{
	init_7620_gpio();
	__init_wm8960_voices();
	//disable_gpio();
#ifndef TEST_SDK
	playsysvoices(START_SYS_VOICES);//开机启动音
#endif
	initStream(ack_playCtr,WritePcmData,i2s_start_play,GetVol);
	init_stdvoices_pthread();
	init_record_pthread();
#ifdef TEST_SDK
	enable_gpio();
#endif

#ifdef LOCAL_MP3
	InitMusicList();
	pool_add_task(waitLoadMusicList, NULL);	//防止T卡加载慢
#endif
}
void clean_main_voices(void)
{
	exit_record_pthread();
	i2s_destory_voices();
	clean_7620_gpio();
	clean_stdvoices_pthread();
	cleanStream();
}

