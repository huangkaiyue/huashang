#include "comshead.h"
#include "config.h"
#include "udpsrv/broadcast.h"
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
#include "../host/studyvoices/qtts_qisc.h"

#define MAIN_DOWN
#ifdef MAIN_DOWN
static WorkQueue *DownEvent=NULL;
int AddDownEvent(char *data,int msgSize){
	return putMsgQueue(DownEvent,(const char *)data, msgSize);
}
#endif
int getplayEventNum(void)
{
	return getWorkMsgNum(DownEvent);
}
static unsigned char clean_sign=0;
void cleanplayEvent(unsigned char type){
	clean_sign=type;
}
void init_system_net(void)
{
	set_pthread_sigblock();
	InitSysList(FRIST_SMART_LIST,FRIST_PASSWD);//初始化用户数据库
	pool_init(3);
#ifndef SELECT_UDP
	init_broadcast(UDP_BRO_PORT);
#endif
}
int clean_resources(void)
{
	clean_main_voices();
	clean_videoServer();
#ifndef SELECT_UDP
	clean_broadcast();
#endif
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
	}
	for(i=0; i<argc; i++){
		int lastarg = i==argc-1;
		if(!strcmp(argv[i],"-qttspath") && !lastarg){
			char *qttspath = argv[i+1];
			memcpy(sysMes.sd_path,qttspath,strlen(qttspath));
		}
	}
}

int main(int argc, char **argv)
{   
	checkConnectFile();
	memset(&sysMes,0,sizeof(SysMessage));
	
	//sysMes.network_timeout=0;
	sysMes.localplayname=0;		//本地播放目录
	time_t t;
	sysMes.Playlocaltime=time(&t);

	loadLocalServer(argc,argv);
	init_system_net();
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
#ifdef	SYSTEMLOCK
	int opennumber=getSystemLock();
	if(opennumber>SYSTEMLOCKNUM){	//检查开机次数
		QttsPlayEvent("权限次数不够。请联系软件所属公司，深圳日晖网讯有限公司，常先生。或者唐工 QQ ：121109281。",QTTS_SYS);
		sleep(10);
		QttsPlayEvent("权限次数不够。请联系软件所属公司，深圳日晖网讯有限公司，常先生。或者唐工 QQ ：121109281。",QTTS_SYS);
		sleep(10);
		QttsPlayEvent("权限次数不够。请联系软件所属公司，深圳日晖网讯有限公司，常先生。或者唐工 QQ ：121109281。",QTTS_SYS);
		sleep(10);
		QttsPlayEvent("权限次数不够。请联系软件所属公司，深圳日晖网讯有限公司，常先生。或者唐工 QQ ：121109281。",QTTS_SYS);
		sleep(10);
		SetSystemTime(1);
	}
	setSystemLock((opennumber+1));
#endif
	
#ifdef MAIN_DOWN
	char *msg=NULL;
	int event=0;
	while(1){
		getMsgQueue(DownEvent,&msg,&event);
		if(clean_sign==1){
			free((void *)msg);
			usleep(100);
			continue;
		}
		if(URL_VOICES_EVENT==event){
#ifdef PALY_URL_SD
			PlayUrl((const void *)msg);
#else
			NetStreamDownFilePlay((const void *)msg);
#endif
			free((void *)msg);
		}
#ifdef 	LOCAL_MP3
		else if(LOCAL_MP3_EVENT==event){
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
		}				//end LOCAL_MP3_EVENT
#endif
		else if(QUIT_MAIN==event){
			printf("end main !!!\n");
			break;
		}
	}
#endif
	return 0;
}

