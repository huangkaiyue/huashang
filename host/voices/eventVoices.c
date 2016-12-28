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
#define START_TAIK_MESSAGE_E START_TAIK_MESSAGE

#define DBG_EVENT
#ifdef DBG_EVENT  
#define DEBUG_EVENT(fmt, args...) printf("event : " fmt, ## args)  
#else   
#define DEBUG_EVENT(fmt, args...) { }  
#endif //end DBG_EVENT

extern void DelSdcardMp3file(char * sdpath);

SysMessage sysMes;

#ifdef CHECKNETWORK
#define N_VOICES_1	1
#define N_VOICES_2	2
#define N_VOICES_3	3
#define N_VOICES_4	4
#define N_VOICES_5	5
static int starttime_net=0;
static void TaiBenToNONetWork(void){
#if 0			//重连网络
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
		case N_VOICES_1:
			play_sys_tices_voices(NETWORK_ERROR_1);
			break;
		case N_VOICES_2:
			play_sys_tices_voices(NETWORK_ERROR_2);
			break;
		case N_VOICES_3:
			play_sys_tices_voices(NETWORK_ERROR_3);
			break;
		case N_VOICES_4:
			play_sys_tices_voices(NETWORK_ERROR_4);
			break;
		case N_VOICES_5:
			play_sys_tices_voices(NETWORK_ERROR_5);
			break;
	}
}
void setNetWorkLive(unsigned char live){
	sysMes.network_live=live;
}
static int getNetWorkLive(void){
	return sysMes.network_live;
}
int checkNetWorkLive(void){
	if(getNetWorkLive()==NETWORK_ER){
		//播报台本
		if(getEventNum()>0){	//检查是否添加过事件
			return;
		}
		create_event_system_voices(NETWORK_ERROT_PLAY);
		return -1;
	}else if(getNetWorkLive()==NETWORK_OK){
		return 0;
	}
}
#endif
static int check_add_event_permission(void)
{
	while(1){
		switch(GetRecordeLive()){
			case PAUSE_E:	//暂停状态，运行任何事件添加
				return;
			case START_TAIK_MESSAGE_E:
				usleep(1000);
				continue;
			default:
				return;
		}
	}
}
/*******************************************************
函数功能: 播放网络URL地址
参数: url URL地址	
返回值: 无
********************************************************/
static void CreateUrlEvent(const void *data){
	playurlLog("url_start\n");
	playurlLog(VERSION);	//版本时间
#ifdef CHECKNETWORK
	if(checkNetWorkLive()){	//检查网络
		return;
	}
#endif
	if(getEventNum()>0||getplayEventNum()>0){
		DEBUG_EVENT("CreateUrlEvent num =%d \n",getEventNum());
		return;
	}
	if(GetRecordeLive() == PLAY_WAV_E){
		//exitqttsPlay();
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
static int CreateLocalMp3(char *localpath){
	if(getEventNum()>0){
		DEBUG_EVENT("CreateUrlEvent num =%d \n",getEventNum());
		return -1;
	}
	if(GetRecordeLive() == PLAY_WAV_E){
		DEBUG_EVENT("CreateUrlEvent PLAY_WAV_E \n");
		exitqttsPlay();
		return -1;
	}
	char *URL= (char *)calloc(1,strlen(localpath)+1);
	if(URL==NULL){
		perror("calloc error !!!");
		return -1;
	}
	sprintf(URL,"%s",localpath);
	DEBUG_EVENT("CreateUrlEvent URL : %s \n",URL);
	add_event_msg(URL,0,LOCAL_MP3_EVENT);
	return 0;
}
static int PlayLocal(unsigned char menu,const char *path, unsigned char Mode){	
	char buf[128]={0};
	char filename[64]={0};
	int ret=-1;
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		create_event_system_voices(TF_ERROT_PLAY);
		return ret;
	}
	if(GetSdcardMusic((const char *)TF_SYS_PATH,path,filename, Mode)){
		create_event_system_voices(PLAY_ERROT_PLAY);
		return ret;
	}
	snprintf(buf,128,"%s%s%s",TF_SYS_PATH,path,filename);
	printf("filepath = %s\n",buf);
	ret=CreateLocalMp3(buf);
	if(ret==0)
		sysMes.localplayname=menu;
	return ret;
}
#endif
#ifdef TESTMP3
static unsigned char Testmp3sign =0;
void TestPlay(void){
	char buf[128]={0};
	Testmp3sign++;
	if(Testmp3sign>4){
		Testmp3sign=0;
	}
	snprintf(buf,128,"%stest/test%d.mp3",TF_SYS_PATH,Testmp3sign);
	printf("filepath = %s\n",buf);
	CreateLocalMp3(buf);
	return;
}
#endif
void CreateLikeMusic(void){
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeLive()!=PAUSE_E)	
			create_event_system_voices(TF_ERROT_PLAY);
		return ;
	}
	if(GetRecordeLive()!=PLAY_URL_E){	//检查播放状态
		if(GetRecordeLive()!=PAUSE_E){
			//create_event_system_voices(TF_ERROT_PLAY);	//Fix me 当前没有播放音乐
		}
		return ;
	}
	Save_like_music();
}
void DelLikeMusic(void){
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeLive()!=PAUSE_E)	
			create_event_system_voices(TF_ERROT_PLAY);
		return ;
	}
	if(GetRecordeLive()!=PLAY_URL_E){	//检查播放状态
		if(GetRecordeLive()!=PAUSE_E){
			//create_event_system_voices(TF_ERROT_PLAY);	//Fix me 当前没有播放音乐
		}
		return ;
	}
	Del_like_music();
}
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
				ret = SetVol(VOL_ADD,0);	//音量加
			}else if(KeyNum==SUBVOL_KEY){
				ret = SetVol(VOL_SUB,0);	//音量减
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
/*******************************************************
函数功能: 播放MP3
参数: play 本地MP3播放命令 或 URL地址
返回值: 无
********************************************************/
int createPlayEvent(const void *play,unsigned char Mode){
	int ret=-1;
#ifdef LOCAL_MP3
	if(Mode==PLAY_NEXT_AUTO){
		Mode=PLAY_NEXT;
	}else{
		time_t t;
		sysMes.Playlocaltime=time(&t);
	}
	if(!strcmp((const char *)play,"mp3")){
		ret=PlayLocal(mp3,TF_MP3_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"story")){
		ret=PlayLocal(story,TF_STORY_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"english")){
		ret=PlayLocal(english,TF_ENGLISH_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"guoxue")){
		ret=PlayLocal(guoxue,TF_GUOXUE_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"xiai")){
		ret=PlayLocal(xiai,XIMALA_MUSIC_E,Mode);
	}else{
		ret=0;
		CreateUrlEvent(play);
	}
	return ret;
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
#ifdef CHECKNETWORK
	if(checkNetWorkLive()){	//检查网络
		return;
	}
#endif
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
#ifdef CHECKNETWORK
	if(checkNetWorkLive()){	//检查网络
		exitqttsPlay();		//暂时解决会话过程中，添加系统音导致问题
		return;
	}
#endif
		sysMes.localplayname=0;	
		NetStreamExitFile();
#if 0
		create_event_system_voices(KEY_DOWN_PLAY);
#else
		start_event_std();
#endif
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
//关机保存文件和清理工作
#ifdef PALY_URL_SD
void CloseSystemWork(void){
	SaveSystemPlayNum();
#ifdef CLOCKTOALIYUN
	CloseSystemSignToaliyun();			//发送关机信号给闹钟
#endif
	set_vol_size(GetVol());				//设置声音到路由表
	DelSdcardMp3file(MP3_SDPATH);
}
#endif
void Link_Ok_Work(void){
	Led_vigue_close();
	led_left_right(left,openled);
	led_left_right(right,openled);
	SocSendMenu(3,0);			//发送本地时间给mcu
	usleep(100*1000);
#ifdef CHECKNETWORK
	setNetWorkLive(NETWORK_OK);		//设置联网状态
#endif
}
void Link_Error_Work(void){
	pool_add_task(Led_vigue_open,NULL);
	led_left_right(left,closeled);
	led_left_right(right,closeled);
#ifdef CHECKNETWORK
	setNetWorkLive(NETWORK_ER);
#endif
}
/*******************************************************
函数功能: 创建一个系统声音事件，播放系统声音
参数: sys_voices 系统音标号	
返回值: 无
********************************************************/
void create_event_system_voices(int sys_voices)
{
	playsysvoicesLog("playsys_start\n");
	if((sys_voices==END_SYS_VOICES_PLAY)){	//结束音退出事件
		cleanEvent();	//清理队列
		if(GetRecordeLive()==PLAY_WAV){
			exitqttsPlay();	//清理事件
		}
	}
	if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	add_event_msg(NULL,sys_voices,SYS_VOICES_EVENT);
	playsysvoicesLog("playsys voices end \n");
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
			playsysvoices(REQUEST_FAILED);
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
#ifdef CHECKNETWORK
			setNetWorkLive(NETWORK_ER);
#endif
			play_sys_tices_voices(START_INTERNET);
			break;
		case SMART_CONFIG_OK_PLAY:		//接受密码成功
			play_sys_tices_voices(YES_REAVWIFI);
			break;
		case CONNECT_OK_PLAY:			//连接成功
			play_sys_tices_voices(LINK_SUCCESS);
			Link_Ok_Work();		//连接成功关灯，开灯，状态设置
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
			Link_Error_Work();
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
			//start_event_std();
			break;
		case PLAY_ERROT_PLAY:			//播放失败
			play_sys_tices_voices(PLAY_ERROR);
			//start_event_std();
			break;
		case TF_ERROT_PLAY:				//TF加载失败
			play_sys_tices_voices(TF_ERROR);
			break;			
#ifdef CHECKNETWORK
		case NETWORK_ERROT_PLAY:				//网络连接失败
			TaiBenToNONetWork();
			break;
#endif
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

typedef struct{
	int file_len;
	FILE *savefilefp;
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
函数功能: 创建待发送的文件，并保持到本地当中
参数:	无
返回值: 0 创建成果 -1 创建失败
********************************************************/
static int create_recorder_file(void)
{
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
static void stop_recorder_tosend_file(void){
	char filepath[64]={0};
	pcmwavhdr.size_8 = (speek->file_len+36);
	pcmwavhdr.data_size = speek->file_len;
	DEBUG_EVENT("stop_recorder_tosend_file : fseek file \n");
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
	sprintf(filepath,"%s%s%d%s",sysMes.sd_path,CACHE_WAV_PATH,(unsigned int)t,".amr");
#else
	sprintf(filepath,"%s%d%s",CACHE_WAV_PATH,(unsigned int)t,".amr");
#endif
	if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath)){
		DEBUG_EVENT("enc_wav_amr_file failed");
		return;
	}
	DEBUG_EVENT("stop save file \n");
	//create_event_system_voices(SEND_LINK_PLAY);
	create_event_system_voices(KEY_DOWN_PLAY);
	uploadVoicesToaliyun(filepath,pcmwavhdr.data_size/10+6);
	//remove(SAVE_WAV_VOICES_DATA);
}
/*******************************************************
函数功能:语音对讲按键按下和弹起事件
参数:state 0 按下  1 弹起
返回值:
********************************************************/
void create_event_voices_key(unsigned int state)
{	
#ifdef CHECKNETWORK
	if(checkNetWorkLive()){	//检查网络
		return;
	}
#endif
	if(state==VOLKEYDOWN){	//按下
		
	}else{			//弹起
	
	}
	if(GetRecordeLive() ==PLAY_URL_E){		//打断播放音乐
		CleanUrlEvent();
		return;
	}else if(GetRecordeLive()==PLAY_WAV_E){
		exitqttsPlay();
		return;
	}
	DEBUG_EVENT("create_event_voices_key(%d) ...\n",state);
	add_event_msg(NULL,state,TALK_EVENT_EVENT);
}
/*******************************************************
函数功能:处理按键事件
参数:
返回值:
********************************************************/
void handle_voices_key_event(unsigned int state){
	int endtime;
	time_t t;
	if(state==VOLKEYDOWN&&GetRecordeLive()==RECODE_PAUSE){	//按下
		DEBUG_EVENT("handle_voices_key_event : state(%d)...\n",state);
		start_speek_wait();
		if(create_recorder_file())		//创建音频文件节点，将插入到链表当中
		{
			pause_record_audio();
		}
		sysMes.Starttime=time(&t);
		start_event_talk_message();
	}else if(state==VOLKEYUP){			//弹起
		DEBUG_EVENT("handle_voices_key_event : state(%d)\n",state);
		start_speek_wait();
		endtime=time(&t);
		if((endtime - sysMes.Starttime)<2){		//时间太短
			pause_record_audio();
			shortVoicesClean();
			create_event_system_voices(SEND_ERROR_PLAY);
			return ;
		}else if((endtime - sysMes.Starttime)>10){		//时间太长
			pause_record_audio();
			shortVoicesClean();
			create_event_system_voices(SEND_ERROR_PLAY);
			return ;
		}
		/********************************************
		注 :需要写文件头信息 
		*********************************************/
		stop_recorder_tosend_file();
		pause_record_audio();
	}
}
/*******************************************************
函数功能:保存语音数据 (需要将音频数据压缩保存)
参数:  voices_data 原始音频数据  size 原始音频数据大小
返回值:
********************************************************/
void save_recorder_voices(const char *voices_data,int size){
	int i=0;
	int endtime;
	time_t t;
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		endtime=time(&t);
		if((endtime - sysMes.Starttime)>60){
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
#ifdef SPEEK_VOICES
	speek = (Speek_t *)calloc(1,sizeof(Speek_t));
	if(speek==NULL){
		return ;
	}
	speek->savefilefp=NULL;
	speek->file_len=0;
	pthread_mutex_init(&speek->mutex, NULL);
#endif
	init_7620_gpio();
	__init_wm8960_voices();
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

