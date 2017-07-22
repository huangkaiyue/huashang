#include "comshead.h"
#include "base/pool.h"
#include "srvwork/workinter.h"
#include "sysdata.h"
#include "host/voices/callvoices.h"
#include "host/voices/wm8960i2s.h"
#include "net/network.h"
#include "host/ap_sta.h"
#include "host/StreamPlay/StreamFile.h"
#include "host/voices/gpio_7620.h"
#include "config.h"
#include "host/studyvoices/prompt_tone.h"
#include "uart/uart.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/qtts_qisc.h"
#ifdef WORK_INTER
#include "srvwork/workinter.h"
#endif
#include "log.h"

static WorkQueue *DownEvent=NULL;
static unsigned char mainQueLock=0;
static unsigned char ExitLock=0;

/*
@添加播放/下载 歌曲时间
@data :添加消息数据  msgSize:消息的大小
@ 返回:0添加成功 -1 添加失败
*/
int AddDownEvent(const char *data,int msgSize){
	return putMsgQueue(DownEvent,data, msgSize);
}
//获取播放歌曲事件队列数
int getplayEventNum(void){
	return getWorkMsgNum(DownEvent);
}

//设置主线程队列资源锁
void SetMainQueueLock(unsigned char lock){
	mainQueLock=lock;
}
//退出清除资源
void CleanSystemResources(void){
	disable_gpio();
	int playEventNums =updateCurrentEventNums();
	sleep(1);
	PlayImportVoices(REBOOT_SYSTEM,playEventNums);
	CleanMtkPlatfrom76xx();
	system("reboot &");
	System_StateLog("CleanMtkPlatfrom76xx ok");
	CleanServer();
	System_StateLog("clean_videoServer ok ");
	pool_destroy();
	System_StateLog("pool_destroy ok ");
	CleanWeixinMeesageList();
	System_StateLog("clean resources finished");
}

/*
* 加载传进来的参数,提取qtts 系统语音路径等
*/
static void loadLocalServer(int argc,char *argv[]){
	int i;
	int sleeptime=3;
	char *token=NULL;
	char *user_id  = NULL;
	
	if(argc<4){
		printf("localserver -qttspath /home/ -t 3 -userId xxxxxx -token xxxxxx -v xxx\n");
		exit(1);
	}	
	memset(&sysMes,0,sizeof(SysMessage));
	for(i=0; i<argc; i++){
		int lastarg = i==argc-1;
		if(!strcmp(argv[i],"-qttspath") && !lastarg){
			char *qttspath = argv[i+1];
			memcpy(sysMes.localVoicesPath,qttspath,strlen(qttspath));
		}else if(!strcmp(argv[i],"-t") && !lastarg){
      	  	printf("i :%d sleeptime: %s\n",argc,argv[i+1]);
			sleeptime = atoi(argv[i+1]);
		}else if(!strcmp(argv[i],"-userId") && !lastarg){
			user_id = argv[i+1];
		}else if(!strcmp(argv[i],"-token") && !lastarg){
			token = argv[i+1];
		}else if(!strcmp(argv[i],"-v") && !lastarg){
			WriteLocalserver_Version((const char *)argv[i+1]);
		}
	}
	sleep(sleeptime);	//增加一定睡眠时间，防止加载sdcard和sock冲突，导致udp sock不能通信
	time_t t;
	sysMes.netstate=NETWORK_UNKOWN;	//开机属于未知网络状态
	InitWeixinMeesageList();
	set_pthread_sigblock();
	pool_init(4);	
	InitTuling((const char *) user_id,(const char *) token);	//userId需要保存到路由表当中 ，token 也需要保存
	InitMtkPlatfrom76xx();
	
	DownEvent = initQueue();
#ifdef WORK_INTER
	init_interface(pasreInputCmd);
#endif	//end WORK_INTER
	InitServer();
	init_Uart(UartEventcallFuntion,ack_batteryCtr);	//初始化串口
	led_lr_oc(closeled);
	mkdir(CACHE_WAV_PATH,777);
}
static void Create_playContinueMusic(HandlerText_t *hand){
	int timeout=0;
	Player_t *play =(Player_t *)hand->data;
	if(hand->EventNums==GetCurrentEventNums()){
		Create_CleanUrlEvent();
		if(play->playListState==AUTO_PLAY_EVENT){	//华上自动推送进入显示等待1状态
			
		}
	}
	while(1){
		usleep(100000);
		if(hand->EventNums!=GetCurrentEventNums()){ 	
			break;
		}
		if(getLock_EventQueue()==0){
			Create_PlaySystemEventVoices(CONTINUE_PLAY_MUSIC_VOICES);

			break;
		}
		if(++timeout>=23){
			break;
		}
	}
}
//主线程添加网络歌曲到队列当中播放
static void Main_Thread_AddplayUrlMusic(HandlerText_t *hand){
	Player_t *play =(Player_t *)hand->data;
	Show_musicPicture();
	Mad_PlayMusic(play);
#ifdef HUASHANG_JIAOYU
	Create_playContinueMusic(hand);
#else
	Create_playContinueMusic(hand);
#if 0
	if(play->playListState==AUTO_PLAY_EVENT){			//内部自身产生播放事件
		CreatePlayDefaultMusic_forPlay(play->musicname);//musicname 暂时定义采用这个结构成语变量存放播放类型
		goto exit1;
	}
#endif	
#endif
exit1:
	free((void *)hand->data);
exit0:
	free((void *)hand);
}
//主线程添加本地到队列当中播放
static void Main_Thread_AddPlayLocalSdcard_Music(HandlerText_t *hand){
	Player_t * player =hand->data;
	Show_musicPicture();
	if(Mad_PlayMusic((const char *)hand->data)){
		goto exit0;
	}
	Write_huashangTextLog("Main_Thread_AddPlayLocalSdcard_Music");
	if(GetStreamPlayState()==MUSIC_SINGLE_LIST){	//单曲循环
		CreatePlayListMuisc((const char *)hand->data,PLAY_MUSIC_SDCARD);
	}else{											//自动播放
		if(getEventNum()==0&&getWorkMsgNum(DownEvent)==0){
			Huashang_GetScard_forPlayMusic(PLAY_NEXT,EXTERN_PLAY_EVENT);
		}	
	}
exit0:	
	free((void *)hand->data);
	free((void *)hand);
	usleep(1000);
}
//主线程播放图灵歌曲
static void Main_Thread_playTuLingMusic(HandlerText_t *hand){
	usleep(800*1000);
	if(hand->EventNums!=GetCurrentEventNums()){
		goto exit0;
	}
	RequestTulingLog((const char *)"Main_Thread_playTuLingMusic startplay");
	Show_musicPicture();
	Mad_PlayMusic((Player_t *)hand->data);
	if(hand->EventNums==GetCurrentEventNums()){
		Create_playContinueMusic(hand);
	}	
	RequestTulingLog((const char *)"Main_Thread_playTuLingMusic endplay");
exit0:
	free((void *)hand->data);
	free((void *)hand);
}
//检查文件锁，防止配网、启动联网脚本导致多次启动进程
static void checkFileLock(void){
	if (access(LOCAL_SERVER_FILE_LOCK, 0) < 0){
		FILE *fp =fopen(LOCAL_SERVER_FILE_LOCK, "w+");
		if(fp){	
			fclose(fp);
		}
	}
	else{
		printf("please delete %s \n",LOCAL_SERVER_FILE_LOCK);
		exit(1);
	}
}
//信号处理函数
void recvErrorSignal(int sig)  {  
    printf("received signal %d !!!\n",sig);  
	if(ExitLock){
		printf("%s: exitlock \n",__func__);
		return ;
	}
	System_StateLog("received signal");
	ExitLock=1;
	CleanSystemResources();
	exit(0);
}  

int main(int argc, char **argv){   
	checkFileLock();
	loadLocalServer(argc,argv);
	char *msg=NULL;
	int event=0;
	signal(SIGSEGV, recvErrorSignal); 
	while(1){
		getMsgQueue(DownEvent,&msg,&event);
		if (mainQueLock == MAIN_QUEUE_LOCK){
			free((void *)msg);
			usleep(100);
			continue;
		}
		switch(event){
			case URL_VOICES_EVENT:	//url播放
				Main_Thread_AddplayUrlMusic((HandlerText_t *)msg);
				printf("%s: Main_Thread_AddplayUrlMusic end\n",__func__);
				break;
			case TULING_URL_VOICES:	//播放图灵 语音点歌、故事 url文件
				Main_Thread_playTuLingMusic((HandlerText_t *)msg);
				printf("%s: Main_Thread_playTuLingMusic end\n",__func__);
				break;
			case TULING_URL_MAIN:	//播放图灵 tts文件
				if(PlayTulingText((HandlerText_t *)msg)){	//异常退出，需要清理后面的url播放事件
					SetMainQueueLock(MAIN_QUEUE_LOCK);		//清理后面mp3播放
				}
				printf("%s: PlayTulingText end\n",__func__);
				break;
			case LOCAL_MP3_EVENT:	//本地播放
				Main_Thread_AddPlayLocalSdcard_Music((HandlerText_t *)msg);
				printf("%s: Main_Thread_AddPlayLocalSdcard_Music end\n",__func__);
				break;				
			case QUIT_MAIN:
				printf("end main !!!\n");
				CleanSystemResources();
				goto exit0;
		}
	}
exit0:
	return 0;
}
