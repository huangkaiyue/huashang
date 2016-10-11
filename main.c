#include <stdio.h>
#include <netinet/in.h>    
#include <arpa/inet.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

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


void test_git(void)
{

}

void test_v2()
{

}

#define MAIN_DOWN
#ifdef MAIN_DOWN
static WorkQueue *DownEvent=NULL;
int AddDownEvent(char *data,int msgSize){
	return putMsgQueue(DownEvent,(const char *)data, msgSize);
}
#endif

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
	printf("clean resources finished \n");
	return 0;
}
int main(int argc, char **argv)
{   
	checkConnectFile();
#ifndef TEST_SDK
	sleep(7);
#endif	//end TEST_SDK
	memset(&sysMes,0,sizeof(SysMessage));

	if(argc<2)
	{
		printf("please input sdcrad path:  serverip: ./localserver /media/mmcblk0p1/ \n");
		return -1;
	}
	memcpy(sysMes.sd_path,argv[1],strlen(argv[1]));
	init_system_net();
	init_wm8960_voices();
	
	DownEvent = initQueue();

	sysMes.network_timeout=0;
#ifdef WORK_INTER
	init_interface(pasreInputCmd);
#endif	//end WORK_INTER
	init_videoServer();

#ifdef UART
	init_Uart(create_event_system_voices,ack_batteryCtr);//tang : 2016.6.30 change for uart init
#endif	//end UART
#ifdef	LED_LR
	led_left_right(left,closeled);
	led_left_right(right,closeled);
#endif
#ifdef MAIN_DOWN
	char *msg=NULL;
	int event=0;
	while(1){
		getMsgQueue(DownEvent,&msg,&event);
		if(URL_VOICES_EVENT==event){
#ifdef PALY_URL_SD
			PlayUrl((const void *)msg);
#else
			NetStreamDownFilePlay((const void *)msg);
#endif
			free((void *)msg);
		}
		else if(LOCAL_MP3_EVENT==event){
			playLocalMp3((const char *)msg);
			usleep(100*1000);
			if(getWorkMsgNum(DownEvent)==0){
				if(strstr(msg,TF_STORY_PATH))
					createPlayEvent((const void *)"story");
				if(strstr(msg,TF_MP3_PATH))
					createPlayEvent((const void *)"mp3");
				if(strstr(msg,TF_TEST_PATH))
					createPlayEvent((const void *)"testmp3");
#ifdef	LED_LR
				if(strstr(msg,TF_ENGLISH_PATH))
					createPlayEvent((const void *)"english");
#endif
			}
			free((void *)msg);
		}				//end LOCAL_MP3_EVENT
	}
#endif
	return 0;
}

