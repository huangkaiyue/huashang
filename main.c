#include "comshead.h"
#include "base/pool.h"
#include "srvwork/workinter.h"
#include "sysdata.h"
#include "host/voices/callvoices.h"
#include "host/voices/wm8960i2s.h"
#include "net/network.h"
#include "uart/uart.h"
#include "host/ap_sta.h"
#include "host/StreamPlay/StreamFile.h"
#include "host/voices/gpio_7620.h"
#include "host/sdcard/musicList.h"
#include "config.h"
#include "host/studyvoices/prompt_tone.h"

static WorkQueue *DownEvent=NULL;
int AddDownEvent(char *data,int msgSize){
	return putMsgQueue(DownEvent,(const char *)data, msgSize);
}
int getplayEventNum(void){
	return getWorkMsgNum(DownEvent);
}
static unsigned char clean_sign=0;
//清除播放事件
void cleanplayEvent(unsigned char type){
	clean_sign=type;
}

int clean_resources(void){
	clean_main_voices();
	clean_videoServer();
	pool_destroy();
	AddDownEvent("baibai",QUIT_MAIN);
	printf("clean resources finished \n");
	return 0;
}
static void loadLocalServer(int argc,const char *argv[]){
	int i;
	char *aliUrl=NULL;
	if(argc<2){
		printf("LocalServer -qttspath /home/\n");
		exit(1);
	}	
	memset(&sysMes,0,sizeof(SysMessage));
	for(i=0; i<argc; i++){
		int lastarg = i==argc-1;
		if(!strcmp(argv[i],"-qttspath") && !lastarg){
			char *qttspath = argv[i+1];
			memcpy(sysMes.sd_path,qttspath,strlen(qttspath));
		}
	}
	time_t t;
	sysMes.localplayname=0;		//本地播放目录
	sysMes.Playlocaltime=time(&t);
	set_pthread_sigblock();
	pool_init(3);
	init_wm8960_voices();
	DownEvent = initQueue();
#ifdef WORK_INTER
	init_interface(pasreInputCmd);
#endif	//end WORK_INTER
	init_videoServer();
	init_Uart(create_event_system_voices,ack_batteryCtr);	//初始化串口
#ifdef	LED_LR
	led_left_right(left,closeled);
	led_left_right(right,closeled);
#endif
#ifdef SYSTEMLOCK
	checkSystemLock();
#endif
	mkdir(CACHE_WAV_PATH,777);
}

int main(int argc, char **argv){   
	checkConnectFile();
	loadLocalServer(argc,argv);
	char *msg=NULL;
	int event=0;
	while(1){
		getMsgQueue(DownEvent,&msg,&event);
		if(clean_sign==1){
			free((void *)msg);
			usleep(100);
			continue;
		}
		switch(event){
			case URL_VOICES_EVENT:
				#ifdef PALY_URL_SD
					PlayUrl((const void *)msg);
				#else
					NetStreamDownFilePlay((const void *)msg);
				#endif
				free((void *)msg);
				break;
			case LOCAL_MP3_EVENT:	
				playLocalMp3((const char *)msg);
				free((void *)msg);
				usleep(1000);
			#if 1
				if(getEventNum()==0&&getWorkMsgNum(DownEvent)==0){
				switch(sysMes.localplayname){
					case mp3:
						createPlayEvent((const void *)"mp3",PLAY_NEXT_AUTO);
						break;
					case story:
						createPlayEvent((const void *)"story",PLAY_NEXT_AUTO);
						break;
					case english:
						createPlayEvent((const void *)"english",PLAY_NEXT_AUTO);
						break;
					case guoxue:
						createPlayEvent((const void *)"guoxue",PLAY_NEXT_AUTO);
						break;
					default:
						sysMes.localplayname=0;
						break;
					}
				}			
			#endif				
				break;
			case QUIT_MAIN:
				printf("end main !!!\n");
				goto exit0;
		}
	}
exit0:
	return 0;
}

