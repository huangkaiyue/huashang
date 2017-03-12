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
@��Ӳ���/���� ����ʱ��
@data :�����Ϣ����  msgSize:��Ϣ�Ĵ�С
@ ����:0��ӳɹ� -1 ���ʧ��
*/
int AddDownEvent(const char *data,int msgSize){
	return putMsgQueue(DownEvent,data, msgSize);
}
//��ȡ���Ÿ����¼�������
int getplayEventNum(void){
	return getWorkMsgNum(DownEvent);
}
static unsigned char clean_sign=0;
//��������¼�
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
* ���ش������Ĳ���,��ȡqtts ϵͳ����·����
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
	sysMes.localplayname=0;			//���ز���Ŀ¼
	sysMes.netstate=NETWORK_UNKOWN;	//����������δ֪����״̬
	sysMes.Playlocaltime=time(&t);
	set_pthread_sigblock();
	pool_init(4);
	InitMtkPlatfrom76xx();
	sleep(sleeptime);	//����һ��˯��ʱ�䣬��ֹ����sdcard��sock��ͻ������udp sock����ͨ��
	DownEvent = initQueue();
	
#ifdef WORK_INTER
	init_interface(pasreInputCmd);
#endif	//end WORK_INTER
	init_videoServer();
	init_Uart(Create_PlaySystemEventVoices,ack_batteryCtr);	//��ʼ������
	
#ifdef	LED_LR
	led_lr_oc(closeled);
#endif
#ifdef SYSTEMLOCK
	checkSystemLock();
#endif
	srand((unsigned)time(NULL));	//ȡ���������
	mkdir(CACHE_WAV_PATH,777);

}
//�Զ�������һ�׸��� musicType ���Ÿ�������(��������Ŀ¼��ʶ)
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
//����ļ�������ֹ�����������
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
			case URL_VOICES_EVENT:	//url����
				//start_event_play_url();		//��ʱ���ͼ�鲥��mp3.״̬��������
				#ifdef PALY_URL_SD
					PlayUrl((const void *)msg);
				#else
					NetStreamDownFilePlay((const void *)msg);
				#endif
				free((void *)msg);
				break;
			case TULING_URL_VOICES:	//url����
				usleep(1800*1000);
				start_event_play_url();		//��ʱ���ͼ�鲥��mp3.״̬��������
				#ifdef PALY_URL_SD
					PlayUrl((const void *)msg);
				#else
					NetStreamDownFilePlay((const void *)msg);
				#endif
				free((void *)msg);
				break;
			case TULING_URL_MAIN:	//ͼ�鲥��
				//start_play_tuling();
				if(PlayTulingText((const char*)msg)){
					cleanplayEvent(1);		//�������mp3����
				}
				free((void *)msg);
				break;
			case LOCAL_MP3_EVENT:	//���ز���
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
