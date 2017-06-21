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
#include "host/sdcard/musicList.h"
#include "config.h"
#include "host/studyvoices/prompt_tone.h"
#include "uart/uart.h"
#include "host/studyvoices/std_worklist.h"
#ifdef WORK_INTER
#include "srvwork/workinter.h"
#endif
#include "log.h"
static WorkQueue *DownEvent=NULL;
static unsigned char mainQueLock=0;

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
	CleanMtkPlatfrom76xx();
	clean_videoServer();
	pool_destroy();
	AddDownEvent((const char *)"baibai",QUIT_MAIN);
	printf("clean resources finished \n");
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
	sysMes.localplayname=0;			//本地播放目录
	sysMes.netstate=NETWORK_UNKOWN;	//开机不属于未知网络状态
	sysMes.auto_count_starttime=time(&t);
	sysMes.enableCountStarttime=DISABLE_count_time;
	set_pthread_sigblock();
	pool_init(4);	
	InitTuling((const char *) user_id,(const char *) token);	//userId需要保存到路由表当中 ，token 也需要保存
	InitMtkPlatfrom76xx();
	
	DownEvent = initQueue();
	
#ifdef WORK_INTER
	init_interface(pasreInputCmd);
#endif	//end WORK_INTER
	init_videoServer();
	init_Uart(UartEventcallFuntion,ack_batteryCtr);	//初始化串口
	
#ifdef	LED_LR
	led_lr_oc(closeled);
#endif
	srand((unsigned)time(NULL));	//取随机数基数
	mkdir(CACHE_WAV_PATH,777);
}
//自动播放下一首歌曲 musicType 播放歌曲类型(用来区分目录标识)
static void autoPlayNextMusic(unsigned char musicType){
	switch(musicType){
#if defined(DATOU_JIANG)		
		case mp3:
			GetSdcardMusicNameforPlay(mp3,TF_MP3_PATH,PLAY_NEXT);
			break;
		case story:
			GetSdcardMusicNameforPlay(story,TF_STORY_PATH,PLAY_NEXT);
			break;
		case english:
			GetSdcardMusicNameforPlay(english,TF_ENGLISH_PATH,PLAY_NEXT);
			break;
		case guoxue:
			GetSdcardMusicNameforPlay(guoxue,TF_GUOXUE_PATH,PLAY_NEXT);
			break;
#elif defined(QITUTU_SHI)				
		case xiai:
			GetSdcardMusicNameforPlay(xiai,XIMALA_MUSIC_DIRNAME,PLAY_NEXT);
			break;
#elif defined(HUASHANG_JIAOYU)			
		case huashang:
			GetScard_forPlayHuashang_Music((const void *)HUASHANG_GUOXUE_DIR,PLAY_NEXT,EXTERN_PLAY_EVENT);
			break;
#endif			
		default:
			sysMes.localplayname=0;
		break;
	}	
}
//主线程添加网络歌曲到队列当中播放
static void Main_Thread_AddplayUrlMusic(HandlerText_t *hand){
	Player_t *play =(Player_t *)hand->data;
	Show_musicPicture();
	Mad_PlayMusic(play);
	if(play->playListState==AUTO_PLAY_EVENT){			//内部自身产生播放事件
		CreatePlayDefaultMusic_forPlay(play->musicname);//musicname 暂时定义采用这个结构成语变量存放播放类型
		goto exit1;
	}else{
		if(GetStreamPlayState()==MUSIC_SINGLE_LIST){	//单曲循环 hand->data 添加到队列，不能释放
			CreatePlayListMuisc((const char *)hand->data,PLAY_MUSIC_NETWORK);
			goto exit0;
		}
//#ifdef HUASHANG_JIAOYU
		if(hand->EventNums==GetCurrentEventNums()){
			Create_PlaySystemEventVoices(CONTINUE_PLAY_MUSIC_VOICES);
		}
//#endif
	}
exit1:
	free((void *)hand->data);
exit0:
	free((void *)hand);
}
//主线程添加本地到队列当中播放
static void Main_Thread_AddPlayLocalSdcard_Music(HandlerText_t *hand){
	Player_t * player =NULL;
	Show_musicPicture();
	if(Mad_PlayMusic((const char *)hand->data)){
		goto exit0;
	}
	if(player->playListState==AUTO_PLAY_EVENT){			//内部自身产生播放事件
		CreatePlayDefaultMusic_forPlay(player->musicname);//musicname 暂时定义采用这个结构成语变量存放播放类型
		goto exit0;
	}
	if(GetStreamPlayState()==MUSIC_SINGLE_LIST){	//单曲循环
		CreatePlayListMuisc((const char *)hand->data,PLAY_MUSIC_SDCARD);
	}else{											//自动播放
		if(getEventNum()==0&&getWorkMsgNum(DownEvent)==0){
			autoPlayNextMusic(sysMes.localplayname);
		}	
	}
exit0:	
	free((void *)hand->data);
	free((void *)hand);
	usleep(1000);
}
static void Main_Thread_playTuLingMusic(HandlerText_t *hand){
	usleep(800*1000);
	if(hand->EventNums!=GetCurrentEventNums()){
		goto exit0;
	}
	RequestTulingLog((const char *)"Main_Thread_playTuLingMusic startplay");
	start_event_play_Mp3music();
	Show_musicPicture();
	Mad_PlayMusic((Player_t *)hand->data);
	if(hand->EventNums==GetCurrentEventNums()){
		Create_PlaySystemEventVoices(CONTINUE_PLAY_MUSIC_VOICES);
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

int main(int argc, char **argv){   
	checkFileLock();
	loadLocalServer(argc,argv);
	char *msg=NULL;
	int event=0;
	//showFacePicture(CONNECT_WIFI_ING_PICTURE);
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
#ifdef PALY_URL_SD
			case WEIXIN_DOWN_MP3_EVENT:	//微信端下载歌曲事件	
				HandlerWeixinDownMp3((const void *)msg);
				free((void *)msg);
				printf("%s: HandlerWeixinDownMp3 end\n",__func__);
				break;
#endif				
			case QUIT_MAIN:
				printf("end main !!!\n");
				goto exit0;
		}
	}
exit0:
	return 0;
}
