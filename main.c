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

static WorkQueue *DownEvent=NULL;
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
static unsigned char clean_sign=0;
//清除播放事件
void cleanplayEvent(unsigned char type){
	clean_sign=type;
}

void clean_resources(void){
	CleanMtkPlatfrom76xx();
	clean_videoServer();
	pool_destroy();
	AddDownEvent((const char *)"baibai",QUIT_MAIN);
	printf("clean resources finished \n");
}
/*
* 加载传进来的参数,提取qtts 系统语音路径等
*/
static void loadLocalServer(int argc,const char *argv[]){
	int i;
	char *aliUrl=NULL;
	int sleeptime=3;
	if(argc<2){
		printf("localServer -qttspath /home/\n");
		exit(1);
	}	
	memset(&sysMes,0,sizeof(SysMessage));
	for(i=0; i<argc; i++){
		int lastarg = i==argc-1;
		if(!strcmp(argv[i],"-qttspath") && !lastarg){
			char *qttspath = argv[i+1];
			memcpy(sysMes.sd_path,qttspath,strlen(qttspath));
		}else if(!strcmp(argv[i],"-t") && !lastarg){
      	  	printf("i :%d sleeptime: %s\n",argc,argv[i+1]);
			sleeptime = atoi(argv[i+1]);
		}
	}
	
	time_t t;
	sysMes.localplayname=0;			//本地播放目录
	sysMes.netstate=NETWORK_UNKOWN;	//开机不属于未知网络状态
	sysMes.Playlocaltime=time(&t);
	set_pthread_sigblock();
	pool_init(4);
	InitMtkPlatfrom76xx();
	sleep(sleeptime);	//增加一定睡眠时间，防止加载sdcard和sock冲突，导致udp sock不能通信
	DownEvent = initQueue();
	
#ifdef WORK_INTER
	init_interface(pasreInputCmd);
#endif	//end WORK_INTER
	init_videoServer();
	init_Uart(Create_PlaySystemEventVoices,ack_batteryCtr);	//初始化串口
	
#ifdef	LED_LR
	led_lr_oc(closeled);
#endif
#ifdef SYSTEMLOCK
	checkSystemLock();
#endif
	srand((unsigned)time(NULL));	//取随机数基数
	mkdir(CACHE_WAV_PATH,777);

}
//自动播放下一首歌曲 musicType 播放歌曲类型(用来区分目录标识)
static void autoPlayNextMusic(unsigned char musicType){
	setAutoPlayMusicTime();
	switch(musicType)
	{
		case mp3:
			createPlayEvent((const void *)"mp3", PLAY_NEXT);
			break;
		case story:
			createPlayEvent((const void *)"story", PLAY_NEXT);
			break;
		case english:
			createPlayEvent((const void *)"english", PLAY_NEXT);
			break;
		case guoxue:
			createPlayEvent((const void *)"guoxue", PLAY_NEXT);
			break;
		default:
			sysMes.localplayname=0;
		break;
	}	
} 
//检查文件锁，防止多次启动进程
static void checkFileLock(void)
{
	if (access("/var/localserver.lock", 0) < 0)
	{
		fopen("/var/localserver.lock", "w+");
	}
	else
	{
		printf("please delete /var/localserver.lock \n");
		exit(1);
	}
}

int main(int argc, char **argv)
{   
	checkFileLock();
#if 1
	loadLocalServer(argc,argv);
#else
	sleep(20);
	init_videoServer();
	while(1){
		sleep(1);
	}
#endif
	char *msg=NULL;
	int event=0;
	while(1)
	{
		getMsgQueue(DownEvent,&msg,&event);
		if (clean_sign == 1)
		{
			free((void *)msg);
			usleep(100);
			continue;
		}
		switch(event)
		{
			case URL_VOICES_EVENT:	//url播放
				//start_event_play_url();		//暂时解决图灵播放mp3.状态不对问题
				#ifdef PALY_URL_SD
					PlayUrl((const void *)msg);
				#else
					NetStreamDownFilePlay((const void *)msg);
				#endif
				free((void *)msg);
				break;
			case TULING_URL_VOICES:	//url播放
				usleep(1800*1000);
				start_event_play_url();		//暂时解决图灵播放mp3.状态不对问题
				#ifdef PALY_URL_SD
					PlayUrl((const void *)msg);
				#else
					NetStreamDownFilePlay((const void *)msg);
				#endif
				free((void *)msg);
				break;
			case TULING_URL_MAIN:	//图灵播放
				//start_play_tuling();
				if(PlayTulingText((const char*)msg)){
					cleanplayEvent(1);		//清理后面mp3播放
				}
				free((void *)msg);
				break;
			case LOCAL_MP3_EVENT:	//本地播放
				printf("LOCAL_MP3_EVENT start play\n");
				playLocalMp3((const char *)msg);
				free((void *)msg);
				usleep(1000);
			#if 1
				if(getEventNum()==0&&getWorkMsgNum(DownEvent)==0){
					autoPlayNextMusic(sysMes.localplayname);
				}			
			#endif				
				break;
#ifdef TEST_PLAY_EQ_MUSIC			
			case TEST_PLAY_EQ_WAV:
				TestPlay_EqWavFile((const void *)msg);
				free((void *)msg);
				usleep(1000);
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
