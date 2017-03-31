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
#ifdef WORK_INTER
#include "srvwork/workinter.h"
#endif
static WorkQueue *DownEvent=NULL;
static unsigned char mainQueLock=0;

static unsigned char playTuling_lock=0;

void SetTuling_playLock(void){
	playTuling_lock=TULING_PLAY_LOCK;
}
void SetTuling_playunLock(void){
	playTuling_lock=TULING_PLAY_UNLOCK;
}
unsigned char getTuling_playunLock(void){
	return playTuling_lock;
}

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

//�������̶߳�����Դ��
void SetMainQueueLock(unsigned char lock){
	mainQueLock=lock;
}
//�˳������Դ
void CleanSystemResources(void){
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
	char *token=NULL;
	char *user_id  = NULL;
	
	if(argc<4){
		printf("localServer -qttspath /home/ -t 3 -userId xxxxxx -token xxxxxx -v xxx\n");
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
	
	time_t t;
	sysMes.localplayname=0;			//���ز���Ŀ¼
	sysMes.netstate=NETWORK_UNKOWN;	//����������δ֪����״̬
	sysMes.Playlocaltime=time(&t);
	set_pthread_sigblock();
	pool_init(4);	
	InitTuling((const char *) user_id,(const char *) token);	//userId��Ҫ���浽·�ɱ��� ��token Ҳ��Ҫ����
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
	switch(musicType){
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
//����ļ�������ֹ���������������ű����¶����������
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
	while(1){
		getMsgQueue(DownEvent,&msg,&event);
		if (mainQueLock == MAIN_QUEUE_LOCK){
			free((void *)msg);
			usleep(100);
			continue;
		}
		switch(event){
			case URL_VOICES_EVENT:	//url����
				//start_event_play_url();		//��ʱ���ͼ�鲥��mp3.״̬��������
				#ifdef PALY_URL_SD
					PlayUrl((const void *)msg);
				#else
					NetStreamDownFilePlay((const void *)msg);
				#endif
				free((void *)msg);
				break;
			case TULING_URL_VOICES:	//����ͼ�� ������衢���� url�ļ�
				usleep(1800*1000);	
				start_event_play_url();		//��ʱ���ͼ�鲥��mp3.״̬��������
				#ifdef PALY_URL_SD
					PlayUrl((const void *)msg);
				#else
					NetStreamDownFilePlay((const void *)msg);
				#endif
				free((void *)msg);
				break;
			case TULING_URL_MAIN:	//����ͼ�� tts�ļ�
				//start_play_tuling();
				if(PlayTulingText((const char*)msg)){		//�쳣�˳�����Ҫ��������url�����¼�
					SetMainQueueLock(MAIN_QUEUE_LOCK);		//�������mp3����
				}
				SetTuling_playunLock();
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
#ifdef PALY_URL_SD
			case WEIXIN_DOWN_MP3_EVENT:	//΢�Ŷ����ظ����¼�	
				HandlerWeixinDownMp3((const void *)msg);
				free((void *)msg);
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
