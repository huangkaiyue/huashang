#include "comshead.h"
#include "host/voices/callvoices.h"
#include "ralink_gpio.h"
#include "gpio_7620.h"
#include "../sdcard/musicList.h"
#include "../studyvoices/qtts_qisc.h"
#include "host/voices/wm8960i2s.h"
#include "nvram.h"
#include "config.h"
//#define MAIN

static Gpio gpio;

static int led_type;
static int led_system_type;

#define GVOL_ADD 	VOL_ADD
#define GVOL_SUB 	VOL_SUB

//������Ƶ��������
void open_wm8960_voices(void){
	ioctl(gpio.fd, TANG_GPIO3924_OPEN);
}
void close_wm8960_voices(void){
	ioctl(gpio.fd, TANG_GPIO3924_CLOSE);
}
//ϵͳ�ƿ���
void open_sys_led(void){
	ioctl(gpio.fd, TANG_LED_CLOSE);	//close ������ Ϊ�ر�
}
void close_sys_led(void){
	ioctl(gpio.fd, TANG_LED_OPEN);
}
void Led_vigue_open(void){
	Led_vigue_close();
	close_sys_led();
	usleep(500*1000);
	open_sys_led();
	usleep(500*1000);
	led_type=LED_VIGUE_OPEN;
	while(led_type==LED_VIGUE_OPEN){
		close_sys_led();
		usleep(500*1000);
		open_sys_led();
		usleep(500*1000);
	}
#if defined(TANGTANG_LUO) ||defined(QITUTU_SHI) || defined(HUASHANG_JIAOYU)
	open_sys_led();	//
#elif defined(DATOU_JIANG)
	close_sys_led();
#endif
}

void Led_System_vigue_close(void){
	led_system_type=LED_VIGUE_CLOSE;
}
void Led_vigue_close(void){
	led_type=LED_VIGUE_CLOSE;
}
//��˸led ָʾ��
void Led_System_vigue_open(void){
	Led_System_vigue_close();
	usleep(300*1000);
	led_lr_oc(closeled);
	usleep(300*1000);
	led_system_type=LED_VIGUE_OPEN;
	while(led_system_type==LED_VIGUE_OPEN){
		led_lr_oc(openled);
		usleep(300*1000);
		led_lr_oc(closeled);
		usleep(300*1000);
	}
#if defined(TANGTANG_LUO) ||defined(QITUTU_SHI) || defined(HUASHANG_JIAOYU)
	led_lr_oc(openled);
#elif defined(DATOU_JIANG)
	led_lr_oc(closeled);
#endif
}
static void led_left_right(unsigned char type,unsigned char io){
	switch(type){
		case left:
			if(io==closeled)
				ioctl(gpio.fd, TANG_LED_CLOSE_LEFT);
			else if(io==openled)
				ioctl(gpio.fd, TANG_LED_OPEN_LEFT);
			break;
		case right:
			if(io==closeled)
				ioctl(gpio.fd, TANG_LED_CLOSE_RIGHT);
			else if(io==openled)
				ioctl(gpio.fd, TANG_LED_OPEN_RIGHT);
			break;
	}
}
//���°�������˸LED 
void keydown_flashingLED(void){
	led_left_right(left,closeled);
	led_left_right(right,closeled);
	usleep(100000);
	led_left_right(left,openled);
	led_left_right(right,openled);
}
void led_lr_oc(unsigned char type){
	switch(type){
		case openled:	//��
			led_left_right(left,openled);
			led_left_right(right,openled);
			break;
		case closeled:
			led_left_right(left,closeled);
			led_left_right(right,closeled);
			break;
	}
}

#if defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)
//�������°��û�����
static void keyDownAck_userBind(void){
	if(gpio.bindsign==BIND_DEV_OK){
		BindDevToaliyun();
		Create_PlaySystemEventVoices(BIND_OK_PLAY);
		gpio.bindsign=BIND_DEV_ER;
	}else{//û�н��յ�������
		Create_PlayQttsEvent("��ȥ����С���һ��������ɡ�",QTTS_GBK);
	}
}
//��Ӧ΢�ź�������
static void Ack_WeixinCall(void){
	if(gpio.callbake==1){
		Create_PlaySystemEventVoices(TALK_CONFIRM_OK_PLAY);//"ȷ����Ϣ�ظ��ɹ����뷢�ϴ�������"
		gpio.callbake==0;
		Ack_CallDev();
	}else{
		if(getEventNum()==0){	//��ֹ��ӵ��¼�̫��
			Create_PlaySystemEventVoices(TALK_CONFIRM_ER_PLAY);//"��ǰ��û���˺�����"
		}
	}
}
static void keyDown_AndSetGpioFor_play(void){
	ioctl(gpio.fd, AUDIO_IC_CONTROL,0x01);//����
}
static void keyUp_AndSetGpioFor_play(void){
	ioctl(gpio.fd, AUDIO_IC_CONTROL,0x20);//����
}	
#endif

//���ڿ���
static void enableUart(void){
	if (ioctl(gpio.fd, TANG_UART_OPEN) < 0) {
		perror("ioctl uart");
		return ;
	}
	DEBUG_GPIO("enableUart..\n");
}
//������
static void lock_msgEv(void){
	gpio.sig_lock=1;
	
}
static void unlock_msgEv(void){
	gpio.sig_lock=0;
}
static int check_lock_msgEv(void){
	if(gpio.sig_lock==1){
		return -1;
	}
	return 0;
}
//���յ�΢�ŷ��͹����İ�����
void EnableBindDev(void){	
	gpio.bindsign=BIND_DEV_OK;
}
//���յ�΢�ŷ��͹����ĺ�������
void EnableCallDev(void){	
	gpio.callbake=1;
}
//��ȡ�ײ���������״̬
static void ReadSpeekGpio(void){
	//writeLog((const char * )"/log/read_gpio.txt",(const char * )"start read\n");
	if (ioctl(gpio.fd,TANG_GET_DATA_3264,&gpio.data) < 0){
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	if((0x01&(gpio.data>>7))==1){
		gpio.speek_tolk=SPEEK;
		//writeLog((const char * )"/log/read_gpio.txt",(const char * )"SPEEK state\n");
	}else{
		gpio.speek_tolk=TOLK;
		//writeLog((const char * )"/log/read_gpio.txt",(const char * )"TOLK state\n");
	}
}



//���ڰ������̰����ã���Ҫ���������Ӽ�����������������
typedef struct {
	unsigned char PthreadState;
	unsigned int key_number;	//������
	unsigned int key_state;		//����״̬:����/����
	
	struct timeval time_start,time_end;
}key_mutiple_t;

#define PthreadState_exit 	0
#define PthreadState_run	1


static void *mus_vol_mutiplekey_Thread(void *arg){
	time_t t;
	int volendtime=0;
	unsigned int time_ms = 0;
	key_mutiple_t *mutiplekey = (key_mutiple_t *)arg;
	int playDownVoicesFlag=0;;
	while(1){
		
		gettimeofday(&mutiplekey->time_end,0);
		time_ms = 1000000*(mutiplekey->time_end.tv_sec - mutiplekey->time_start.tv_sec) + mutiplekey->time_end.tv_usec - mutiplekey->time_start.tv_usec;
		time_ms /= 1000;

		printf("[ %s ]:[ %s ] printf in line [ %d ]   time_ms = %d\n",__FILE__,__func__,__LINE__,time_ms);
		
		if(time_ms < 500){
			if(mutiplekey->key_state == VOLKEYUP)
			{
				if(mutiplekey->key_number == ADDVOL_KEY){
					printf("[ %s ]:[ %s ] printf in line [ %d ]\n",__FILE__,__func__,__LINE__);
#if defined(QITUTU_SHI) 					
					Create_playMusicEvent((const void *)XIAI_DIR,PLAY_NEXT);
#elif defined(HUASHANG_JIAOYU)
					Create_playMusicEvent((const void *)HUASHANG_GUOXUE_DIR,PLAY_NEXT);
#endif
				}
				else{
					printf("[ %s ]:[ %s ] printf in line [ %d ]\n",__FILE__,__func__,__LINE__);
#if defined(QITUTU_SHI)					
					Create_playMusicEvent((const void *)XIAI_DIR,PLAY_PREV);
#elif defined(HUASHANG_JIAOYU)
					Create_playMusicEvent((const void *)HUASHANG_GUOXUE_DIR,PLAY_PREV);
#endif

				}
	
				break;

			}
			usleep(100 * 1000);
			continue;
		}
		

		if(time_ms >=500){
			if(playDownVoicesFlag==0){
				keyDown_AndSetGpioFor_play();
				playDownVoicesFlag++;
			}
			printf("[ %s ]:[ %s ] printf in line [ %d ]   time_ms = %d\n",__FILE__,__func__,__LINE__,time_ms);
			if(mutiplekey->key_state == VOLKEYUP)
				break;

			if(mutiplekey->key_number == ADDVOL_KEY){
				printf("[ %s ]:[ %s ] printf in line [ %d ]\n",__FILE__,__func__,__LINE__);
				if(Setwm8960Vol(VOL_ADD,0) == -1)
					break;
			}
				

			else{
				printf("[ %s ]:[ %s ] printf in line [ %d ]\n",__FILE__,__func__,__LINE__);
				if(Setwm8960Vol(VOL_SUB,0) == -1)
					break;
			}

			usleep(300 * 1000);
		}

	}
	mutiplekey->PthreadState = PthreadState_exit;
	return NULL;
}

//-------------------------------------------QITUTU_SHI--------------------------------------------
#ifdef QITUTU_SHI	//ʯ��
static void signal_handler(int signum){

	static key_mutiple_t mutiple_key_SUB,mutiple_key_ADD,mutiple_key_speek;

	//�õ��ײ㰴���¼�����
	if (ioctl(gpio.fd, TANG_GET_NUMBER,&gpio.mount) < 0){
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	printf("signum = %d\n",signum);
	if(check_lock_msgEv()){
		printf("error is lock signal_handler\n");
		return ;
	}
	lock_msgEv();
	if (signum == GPIO_UP){			//�̰������¼�
		switch(gpio.mount){
			case NETWORK_KEY:		//����WiFi��
				ShortKeyDown_ForPlayWifiMessage();
				break;
				
			case RESERVE_KEY2:
#ifdef SPEEK_VOICES1
				ReadSpeekGpio();
#endif
				break;

			case SPEEK_KEY:			//�Խ������ܻỰ����
				if(gpio.speek_tolk==SPEEK){
					StopTuling_RecordeVoices();
				}else{
					Create_WeixinSpeekEvent(VOLKEYUP);
				}
				break;
#ifdef 	LOCAL_MP3
			case ADDVOL_KEY:	//�̰�����ϲ������,��һ��
				keydown_flashingLED();

				mutiple_key_ADD.key_number = ADDVOL_KEY;
				mutiple_key_ADD.key_state  = VOLKEYUP;

				GpioLog("key up",ADDVOL_KEY);
				break;
			case SUBVOL_KEY:	//�̰�����ϲ������,��һ��
				keydown_flashingLED();

				mutiple_key_SUB.key_number = SUBVOL_KEY;
				mutiple_key_SUB.key_state  = VOLKEYUP;
				GpioLog("key up",SUBVOL_KEY);

				break;
#endif
			case PLAY_PAUSE_KEY:	//���š���ͣ
				keydown_flashingLED();	
				keyStreamPlay();
				break;
			case RESERVE_KEY3:	//ʹ���ղظ���
				keydown_flashingLED();
				Enable_SaveLoveMusicFlag();
				break;
			case LETFLED_KEY:	//�ظ���
				Ack_WeixinCall();
				break;
			case RIGHTLED_KEY:	//bind��
				keyDownAck_userBind();
				break;
		}
		DEBUG_GPIO("signal up (%d) !!!\n",gpio.mount);
	}// end gpio_up
	else if (signum == GPIO_DOWN){	//���������¼�
		switch(gpio.mount){
			case RESET_KEY://�ָ���������
				Create_PlaySystemEventVoices(RESET_HOST_V_PLAY);	//��Ҫ�޸���������:
				system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan gpio");
				break;
					
			case RESERVE_KEY2://�Ự�Խ����ؼ�
				ReadSpeekGpio();
				break;
				
			case NETWORK_KEY://������
				LongNetKeyDown_ForConfigWifi();
				break;
				
			case SPEEK_KEY://�Խ������ܻỰ����
#ifdef	SPEEK_VOICES1 
				ReadSpeekGpio();	//ÿ�ζ�ȡ�ײ�������������״̬
#endif
				if(gpio.speek_tolk==SPEEK){
					TulingKeyDownSingal();
				}else{
					Create_WeixinSpeekEvent(VOLKEYDOWN);
				}			
				break;
			
			case PLAY_PAUSE_KEY://������ͣ����(�������ᷢ�����źţ���������)
				break;
#ifdef	LED_LR
			case ADDVOL_KEY:	//����������
				keydown_flashingLED();
				mutiple_key_ADD.key_state  = VOLKEYDOWN;
				mutiple_key_ADD.key_number = ADDVOL_KEY;
				if(mutiple_key_ADD.PthreadState == PthreadState_run)
					break;
				mutiple_key_ADD.PthreadState = PthreadState_run;
				gettimeofday(&mutiple_key_ADD.time_start,0);
				pool_add_task(mus_vol_mutiplekey_Thread,(void *)&mutiple_key_ADD);
				ack_VolCtr("add",GetVol());		//----------->������
				GpioLog("key down",ADDVOL_KEY);
				break;
			case SUBVOL_KEY:	//����������
				keydown_flashingLED();

				mutiple_key_SUB.key_state  = VOLKEYDOWN;
				mutiple_key_SUB.key_number = SUBVOL_KEY;
				gettimeofday(&mutiple_key_SUB.time_start,0);
				if(mutiple_key_SUB.PthreadState == PthreadState_run)
					break;
				mutiple_key_SUB.PthreadState = PthreadState_run;
				pool_add_task(mus_vol_mutiplekey_Thread,(void *)&mutiple_key_SUB);
				ack_VolCtr("sub",GetVol());		//----------->������
				GpioLog("key down",SUBVOL_KEY);
				break;
			case LETFLED_KEY:	//play next
				break;
#endif
			case RESERVE_KEY3:	//������ɾ���ղظ���
				Delete_LoveMusic();
				break;
		}// end gpio_down
		DEBUG_GPIO("signal down (%d) !!!\n",gpio.mount);
	}
	else{
		DEBUG_GPIO("not know signum ...\n");
	}
	unlock_msgEv();
}
#endif
#ifdef HUASHANG_JIAOYU		//���Ͻ���
//#define TEST_PLAY_KEY
static void signal_handler(int signum){

	static key_mutiple_t mutiple_key_SUB,mutiple_key_ADD,mutiple_key_speek;
	//�õ��ײ㰴���¼�����
	if (ioctl(gpio.fd, TANG_GET_NUMBER,&gpio.mount) < 0){
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	printf("signum = %d\n",signum);
	if(check_lock_msgEv()){
		printf("error is lock signal_handler\n");
		return ;
	}
	lock_msgEv();
	if (signum == GPIO_UP){			//�̰������¼�
		switch(gpio.mount){
			case NETWORK_KEY:		//����WiFi��
#ifdef TEST_PLAY_KEY
				keydown_flashingLED();
				GpioKey_SetStreamPlayState();
				break;
#endif
				ShortKeyDown_ForPlayWifiMessage();
				break;
				
			case RESERVE_KEY2:
#ifdef SPEEK_VOICES1
				ReadSpeekGpio();
#endif
				break;

			case SPEEK_KEY:			//���ܻỰ�����¼�
				keyUp_AndSetGpioFor_play();
				StopTuling_RecordeVoices();
				break;
#ifdef 	LOCAL_MP3
			case ADDVOL_KEY:	//�̰�����ϲ������,��һ��
				keydown_flashingLED();
				mutiple_key_ADD.key_number = ADDVOL_KEY;
				mutiple_key_ADD.key_state  = VOLKEYUP;
				GpioLog("key up",ADDVOL_KEY);
				break;
			case SUBVOL_KEY:	//�̰�����ϲ������,��һ��
				keydown_flashingLED();
				mutiple_key_SUB.key_number = SUBVOL_KEY;
				mutiple_key_SUB.key_state  = VOLKEYUP;
				GpioLog("key up",SUBVOL_KEY);
				break;
#endif
			case PLAY_PAUSE_KEY:	//���š���ͣ
				keydown_flashingLED();	
				keyStreamPlay();
				break;
			case RESERVE_KEY3:	//���Ͻ��������õ���ѭ�����б�
				keydown_flashingLED();
				keyDown_AndSetGpioFor_play();
				GpioKey_SetStreamPlayState();
				break;
			case HUASHANG_WEIXIN_SPEEK_KEY:	//���Ͻ���΢�ŶԽ�
				keyUp_AndSetGpioFor_play();
				Create_WeixinSpeekEvent(VOLKEYUP);
				break;
			case RIGHTLED_KEY:	//bind��
				keyDownAck_userBind();
				break;
		}
		DEBUG_GPIO("signal up (%d) !!!\n",gpio.mount);
	}// end gpio_up
	else if (signum == GPIO_DOWN){	//���������¼�
		switch(gpio.mount){
			case RESET_KEY://�ָ���������
				Create_PlaySystemEventVoices(RESET_HOST_V_PLAY);	//��Ҫ�޸���������:
				system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan gpio");
				break;
					
			case RESERVE_KEY2://�Ự�Խ����ؼ�
				ReadSpeekGpio();
				break;
				
			case NETWORK_KEY://������
				LongNetKeyDown_ForConfigWifi();
				break;
				
			case SPEEK_KEY://�Ự��
#ifdef TEST_PLAY_KEY
				Create_playMusicEvent((const void *)HUASHANG_GUOXUE_DIR,PLAY_NEXT);
				break;
#endif
				keyDown_AndSetGpioFor_play();
				TulingKeyDownSingal();
				break;
			
			case PLAY_PAUSE_KEY://��������/��ͣ  �л����ܻỰ������
				keyDown_AndSetGpioFor_play();
				Huashang_changePlayVoicesName();
				break;
			case ADDVOL_KEY:	//����������
				keydown_flashingLED();
				mutiple_key_ADD.key_state  = VOLKEYDOWN;
				mutiple_key_ADD.key_number = ADDVOL_KEY;
				if(mutiple_key_ADD.PthreadState == PthreadState_run)
					break;
				mutiple_key_ADD.PthreadState = PthreadState_run;
				gettimeofday(&mutiple_key_ADD.time_start,0);
				pool_add_task(mus_vol_mutiplekey_Thread,(void *)&mutiple_key_ADD);
				ack_VolCtr("add",GetVol());		//----------->������
				GpioLog("key down",ADDVOL_KEY);
				break;
			case SUBVOL_KEY:					//����������
				keydown_flashingLED();
				mutiple_key_SUB.key_state  = VOLKEYDOWN;
				mutiple_key_SUB.key_number = SUBVOL_KEY;
				gettimeofday(&mutiple_key_SUB.time_start,0);
				if(mutiple_key_SUB.PthreadState == PthreadState_run)
					break;
				
				mutiple_key_SUB.PthreadState = PthreadState_run;
				pool_add_task(mus_vol_mutiplekey_Thread,(void *)&mutiple_key_SUB);
				ack_VolCtr("sub",GetVol());		//----------->������
				GpioLog("key down",SUBVOL_KEY);
				break;
			case HUASHANG_WEIXIN_SPEEK_KEY:	//���Ͻ���΢�ŶԽ�
				keyDown_AndSetGpioFor_play();
				Create_WeixinSpeekEvent(VOLKEYDOWN);
				break;
			case RESERVE_KEY3:	//������ɾ���ղظ���
				Delete_LoveMusic();
				break;
		}// end gpio_down
		DEBUG_GPIO("signal down (%d) !!!\n",gpio.mount);
	}
	else{
		DEBUG_GPIO("not know signum ...\n");
	}
	unlock_msgEv();
}

#endif
//----------------------------------------DATOU_JIANG-----------------------------------------------
#ifdef DATOU_JIANG
static void signal_handler(int signum){
	//�õ��ײ㰴���¼�����
	if (ioctl(gpio.fd, TANG_GET_NUMBER,&gpio.mount) < 0){
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	if(check_lock_msgEv()){
		return ;
	}
	lock_msgEv();
	if (signum == GPIO_UP){
		switch(gpio.mount){
			case NETWORK_KEY:		//�̰�����WiFi��
				ShortKeyDown_ForPlayWifiMessage();
				break;

			case SPEEK_KEY:
#ifdef SPEEK_VOICES
				if(gpio.speek_tolk==SPEEK){
					StopTuling_RecordeVoices();
				}else{
					Create_WeixinSpeekEvent(VOLKEYUP);
				}
#else
				StopTuling_RecordeVoices();
#endif
				break;
			
#ifdef 	LOCAL_MP3
			case ADDVOL_KEY:	//play last
				switch(sysMes.localplayname){
					case mp3:
						Create_playMusicEvent((const void *)"mp3",PLAY_PREV);
						break;
					case story:
						Create_playMusicEvent((const void *)"story",PLAY_PREV);
						break;
					case english:
						Create_playMusicEvent((const void *)"english",PLAY_PREV);
						break;
					case guoxue:
						Create_playMusicEvent((const void *)"guoxue",PLAY_PREV);
						break;
					default:
						break;
				}
				break;
			case SUBVOL_KEY:	//play next
				switch(sysMes.localplayname){
					case mp3:
						Create_playMusicEvent((const void *)"mp3",PLAY_NEXT);
						break;
					case story:
						Create_playMusicEvent((const void *)"story",PLAY_NEXT);
						break;
					case english:
						Create_playMusicEvent((const void *)"english",PLAY_NEXT);
						break;
					case guoxue:
						Create_playMusicEvent((const void *)"guoxue",PLAY_NEXT);
						break;
					default:
						break;
				}
				break;
#endif
			case RIGHTLED_KEY:
				if(sysMes.localplayname==english){
					keyStreamPlay();
				}else{
					Create_playMusicEvent((const void *)"english",PLAY_NEXT);
				}
				break;
#ifdef	LOCAL_MP3
			case RESERVE_KEY2:
#ifdef	SPEEK_VOICES1
				ReadSpeekGpio();
#else
				if(sysMes.localplayname==english){
					keyStreamPlay();
				}else{
					Create_playMusicEvent((const void *)"english",PLAY_NEXT);
				}
#endif
				break;

			case PLAY_PAUSE_KEY://������ͣ
				if(sysMes.localplayname==guoxue){
					keyStreamPlay();
				}else{
					Create_playMusicEvent((const void *)"guoxue",PLAY_NEXT);
				}
				break;
			case RESERVE_KEY3:	//Ŀ¼
				if(sysMes.localplayname==mp3){
					keyStreamPlay();
				}else{
					Create_playMusicEvent((const void *)"mp3",PLAY_NEXT);
				}
				break;
			case LETFLED_KEY:	//left led
				if(sysMes.localplayname==story){
					keyStreamPlay();
				}else{
					Create_playMusicEvent((const void *)"story",PLAY_NEXT);
				}
				break;
#endif
		}
		DEBUG_GPIO("signal up (%d) !!!\n",gpio.mount);
	}// end gpio_up
	else if (signum == GPIO_DOWN){
		switch(gpio.mount){
			case RESET_KEY://�ָ���������
				Create_PlaySystemEventVoices(RESET_HOST_V_PLAY);
				system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan gpio");
				break;
					
			case RESERVE_KEY2://�Ự�Խ����ؼ�
#ifdef	SPEEK_VOICES1
				//gpio.speek_tolk=TOLK;
				ReadSpeekGpio();
#endif
				break;
				
			case NETWORK_KEY://������
				LongNetKeyDown_ForConfigWifi();
				break;
				
			case SPEEK_KEY://�Ự��
#ifdef	SPEEK_VOICES
#ifdef	SPEEK_VOICES1
				ReadSpeekGpio();
#endif
				printf("%d \n",gpio.speek_tolk);
				if(gpio.speek_tolk==SPEEK){
					TulingKeyDownSingal();
				}else{
					Create_WeixinSpeekEvent(VOLKEYDOWN);
				}
#else
				TulingKeyDownSingal();
#endif
				break;
			
			case PLAY_PAUSE_KEY://Ԥ����
				break;
#ifdef	LED_LR
			case RESERVE_KEY3:	//Ŀ¼
				break;
			case ADDVOL_KEY:	//vol +
				Setwm8960Vol(GVOL_ADD,0);
				ack_VolCtr("add",GetVol());//----------->������
				break;
			case SUBVOL_KEY:	//vol -
				Setwm8960Vol(GVOL_SUB,0);
				ack_VolCtr("sub",GetVol());//----------->������
				break;
			case LETFLED_KEY:	//left led
				break;
			case RIGHTLED_KEY:
				break;
#endif
		}// end gpio_down
		DEBUG_GPIO("signal down (%d) !!!\n",gpio.mount);
	}
	else{
		DEBUG_GPIO("not know signum ...\n");
	}
	unlock_msgEv();
}
#endif
//------------------------------------TANGTANG_LUO-------------------------------------------------
#ifdef TANGTANG_LUO
static void signal_handler(int signum){
	//�õ��ײ㰴���¼�����
	if (ioctl(gpio.fd, TANG_GET_NUMBER,&gpio.mount) < 0){
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	if(check_lock_msgEv()){
		return ;
	}
	lock_msgEv();
	if (signum == GPIO_UP){
		switch(gpio.mount){
			case NETWORK_KEY:		//����WiFi��
				ShortKeyDown_ForPlayWifiMessage();
				break;

			case SPEEK_KEY:
				StopTuling_RecordeVoices();
				break;
		}
		DEBUG_GPIO("signal up (%d) !!!\n",gpio.mount);
	}// end gpio_up
	else if (signum == GPIO_DOWN){
		switch(gpio.mount){
			case RESET_KEY://�ָ���������
				Create_PlaySystemEventVoices(RESET_HOST_V_PLAY);
				system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan gpio");
				break;
				
			case NETWORK_KEY://������
				LongNetKeyDown_ForConfigWifi();
				break;
				
			case SPEEK_KEY://�Ự��
				TulingKeyDownSingal();
				break;
		}// end gpio_down
		DEBUG_GPIO("signal down (%d) !!!\n",gpio.mount);
	}
	else{
		DEBUG_GPIO("not know signum ...\n");
	}
	unlock_msgEv();
}
#endif

//���ݷ���
static void gpio_set_dir(int r){
	int req;
	if (r == gpio6332){
		req = RALINK_GPIO6332_SET_DIR_IN;
		if (ioctl(gpio.fd, req, (0x1f<<6)) < 0) {
			perror("ioctl");
			close(gpio.fd);
		}
	}
	else if (r == gpio9564)
		req = RALINK_GPIO9564_SET_DIR_IN;
	else{
		req = RALINK_GPIO_SET_DIR_IN;
		if (ioctl(gpio.fd, req, (0xcd<<14)) < 0) {
			perror("ioctl");
			close(gpio.fd);
		}
	}
}
//ʹ�ܰ�������
void enable_gpio(void){
	ralink_gpio_reg_info info;
	gpio_set_dir(gpio6332);
#ifdef	LED_LR
	gpio_set_dir(gpio3200);
#endif
	if (ioctl(gpio.fd, RALINK_GPIO_ENABLE_INTP) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
//------------------------------------------------------------
	info.pid = getpid();
	info.irq = RESET_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	info.pid = getpid();
	info.irq = NETWORK_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	info.pid = getpid();
	info.irq = SPEEK_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	info.pid = getpid();
	info.irq = PLAY_PAUSE_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	info.pid = getpid();
	info.irq = RESERVE_KEY2;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
//------------------------------------------------------------
#ifdef LED_LR
	info.pid = getpid();
	info.irq = RESERVE_KEY3;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	info.pid = getpid();
	info.irq = ADDVOL_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	info.pid = getpid();
	info.irq = SUBVOL_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	info.pid = getpid();
	info.irq = LETFLED_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	info.pid = getpid();
	info.irq = RIGHTLED_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
#endif
//------------------------------------------------------------
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
}
//��ʼ��GPIO
static void enableResetgpio(void){
	if (ioctl(gpio.fd, RALINK_GPIO6332_SET_DIR_IN, (0x1<<6)) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	if (ioctl(gpio.fd, RALINK_GPIO_ENABLE_INTP) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
#if 1	//ʹ�ָܻ��������ð���
	ralink_gpio_reg_info info;
	info.pid = getpid();
	info.irq = RESET_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
#endif
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
}
/*
@ 
@ ��ʼ��mtk76xx ϵ��gpio��ע��gpio����ʹ�ܲ��ְ����ж�
@
*/
void InitMtk76xx_gpio(void){
	memset(&gpio,0,sizeof(Gpio));
	gpio.fd = open(GPIO_DEV, O_RDONLY);
	if (gpio.fd < 0) {
		perror(GPIO_DEV);
		return ;
	}
	enableUart();
#if	0
	close_sys_led();
#else
	pool_add_task(Led_vigue_open,NULL);
#endif
#ifdef SPEEK_VOICES
	ReadSpeekGpio();	//��ȡ�Ự�Խ����ܲ�����
#endif
	enableResetgpio();	//ʹ�ָܻ��������ð��� (��ֹ�����������������޷�����)
}
//ȥʹ�ܰ���
void disable_gpio(void){
	if (ioctl(gpio.fd, RALINK_GPIO_DISABLE_INTP) < 0) {
		perror("disable ioctl");
		close(gpio.fd);
		return ;
	}
	enableResetgpio();	//ʹ�ָܻ��������ð���
}
//���GPIO
void CleanMtk76xx_gpio(void){
	//close_sys_led();
	disable_gpio();
	close(gpio.fd);
}
#ifdef MAIN
int main(void){
	InitMtk76xx_gpio();
	DEBUG_GPIO("InitMtk76xx_gpio \n");
	open_wm8960_voices();
	while(1){
		pause();
	}
	DEBUG_GPIO("clean_7620_gpio \n");
	clean_7620_gpio();
	return 0;
}
#endif //end  MAIN
