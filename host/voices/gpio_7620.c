#include "comshead.h"
#include "host/voices/callvoices.h"
#include "ralink_gpio.h"
#include "gpio_7620.h"
#include "../studyvoices/qtts_qisc.h"
#include "host/voices/wm8960i2s.h"
#include "log.h"
#include "nvram.h"
#include "config.h"

static Gpio gpio;

static int led_type;

#define GVOL_ADD 	VOL_ADD
#define GVOL_SUB 	VOL_SUB

//开关音频驱动函数
void open_wm8960_voices(void){
	ioctl(gpio.fd, TANG_GPIO3924_OPEN);
}
void close_wm8960_voices(void){
	ioctl(gpio.fd, TANG_GPIO3924_CLOSE);
}
//系统灯开关
void open_sys_led(void){
	ioctl(gpio.fd, TANG_LED_CLOSE);	//close 三极管 为关闭
}
void close_sys_led(void){
	ioctl(gpio.fd, TANG_LED_OPEN);
}
void Led_vigue_close(void){
	led_type=LED_VIGUE_CLOSE;
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
	open_sys_led();	
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
//按下按键，闪烁LED 
void keydown_flashingLED(void){
	led_left_right(left,closeled);
	led_left_right(right,closeled);
	usleep(100000);
	led_left_right(left,openled);
	led_left_right(right,openled);
}
void led_lr_oc(unsigned char type){
	switch(type){
		case openled:	//开
			led_left_right(left,openled);
			led_left_right(right,openled);
			break;
		case closeled:
			led_left_right(left,closeled);
			led_left_right(right,closeled);
			break;
	}
}
//设备恢复出厂设置
void ResetHostDevicesFactory(void){
	ResetWeixinBindUserMessage();
	Create_PlaySystemEventVoices(RESET_HOST_V_PLAY);
	system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan gpio");
}
//接收到微信发送过来的绑定请求
void EnableBindDev(void){	
	gpio.bindsign=BIND_DEV_OK;
}

//按键按下绑定用户请求
static void keyDownAck_userBind(void){
	if(gpio.bindsign==BIND_DEV_OK){
		BindDevToaliyun();
		Create_PlaySystemEventVoices(BIND_OK_PLAY);
		gpio.bindsign=BIND_DEV_ER;
	}else{//没有接收到绑定请求
		Create_PlayQttsEvent("小朋友请让爸爸妈妈在微信界面当中邀请小伙伴一起来聊天吧。",QTTS_GBK);
	}
}
//按下音
static void keyDown_AndSetGpioFor_play(void){
	if(GetRecordeVoices_PthreadState()!=PLAY_MP3_MUSIC){
		ioctl(gpio.fd, AUDIO_IC_CONTROL,0x01);//按下
	}
}
//抬起音
static void keyUp_AndSetGpioFor_play(void){
	if(GetRecordeVoices_PthreadState()!=PLAY_MP3_MUSIC){
		ioctl(gpio.fd, AUDIO_IC_CONTROL,0x20);//弹起
	}
}	

//串口开关
static void enableUart(void){
	if (ioctl(gpio.fd, TANG_UART_OPEN) < 0) {
		perror("ioctl uart");
		return ;
	}
	DEBUG_GPIO("enableUart..\n");
}
//锁函数
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
		
		if(time_ms < 500){		//before is 500  2017.6.28 22:43
			if(mutiplekey->key_state == KEYUP)
			{
				if(mutiplekey->key_number == ADDVOL_KEY){
					printf("[ %s ]:[ %s ] printf in line [ %d ]\n",__FILE__,__func__,__LINE__);		
					GetScard_forPlayHuashang_Music(PLAY_NEXT,EXTERN_PLAY_EVENT);
				}
				else{
					printf("[ %s ]:[ %s ] printf in line [ %d ]\n",__FILE__,__func__,__LINE__);
					GetScard_forPlayHuashang_Music(PLAY_PREV,EXTERN_PLAY_EVENT);
				}
				break;

			}
			usleep(100 * 1000);
			continue;
		}
		
		if(time_ms >=500){		//before is 500  2017.6.28 22:43
			if(playDownVoicesFlag==0){
				keyDown_AndSetGpioFor_play();
				playDownVoicesFlag++;
			}
			printf("[ %s ]:[ %s ] printf in line [ %d ]   time_ms = %d\n",__FILE__,__func__,__LINE__,time_ms);
			if(mutiplekey->key_state == KEYUP)
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

static void *weixin_mutiplekey_Thread(void *arg){
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
		
		if(time_ms < 500){		//before is 500  2017.6.28 22:43
			if(mutiplekey->key_state == KEYUP)
			{
				//短按弹起处理，播放微信语音
				GpioLog("GetWeiXinMessageForPlay ",WEIXIN_SPEEK_KEY);
				GetWeiXinMessageForPlay();
				break;

			}
			usleep(100 * 1000);
			continue;
		}
		

		if(time_ms >=500){		//before is 500  2017.6.28 22:43
			if(playDownVoicesFlag==0){
				//在这里开始录音
				keyDown_AndSetGpioFor_play();
				Create_WeixinSpeekEvent(KEYDOWN);
				playDownVoicesFlag++;
			}
			printf("[ %s ]:[ %s ] printf in line [ %d ]   time_ms = %d\n",__FILE__,__func__,__LINE__,time_ms);
			if(mutiplekey->key_state == KEYUP){
				//在这里结束录音
				Create_WeixinSpeekEvent(KEYUP);
				break;
			}

			usleep(300 * 1000);
		}

	}
	mutiplekey->PthreadState = PthreadState_exit;
	return NULL;
}

static void signal_handler(int signum){
	static key_mutiple_t mutiple_key_SUB,mutiple_key_ADD,mutiple_key_speek,mutiple_key_weixin;
	//拿到底层按键事件号码
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
	WakeupSleepSystem();
	if (signum == GPIO_UP){			//短按按键事件
		switch(gpio.mount){
			case NETWORK_KEY:		//播报WiFi名
				ShortKeyDown_ForPlayWifiMessage();
				break;
				

			case SPEEK_KEY:			//智能会话按键事件
				keyUp_AndSetGpioFor_play();
				StopTuling_RecordeVoices();
				break;
			case ADDVOL_KEY:	//短按播放喜爱内容,下一曲
				keydown_flashingLED();
				mutiple_key_ADD.key_number = ADDVOL_KEY;
				mutiple_key_ADD.key_state  = KEYUP;
				GpioLog("key up",ADDVOL_KEY);
				break;
			case SUBVOL_KEY:	//短按播放喜爱内容,上一曲
				keydown_flashingLED();
				mutiple_key_SUB.key_number = SUBVOL_KEY;
				mutiple_key_SUB.key_state  = KEYUP;
				GpioLog("key up",SUBVOL_KEY);
				break;
			case PLAY_PAUSE_KEY:	//播放、暂停
				KeydownEventPlayPause();
				break;
			case DIR_MENU_KEY:	//华上教育，设置单曲循环和列表
				SelectDirMenu();
				break;
			case WEIXIN_SPEEK_KEY:	//华上教育微信对讲
				mutiple_key_weixin.key_number = WEIXIN_SPEEK_KEY;
				mutiple_key_weixin.key_state  = KEYUP;
				break;
			case RIGHTLED_KEY:	//bind键
				keyDownAck_userBind();
				break;
		}
		DEBUG_GPIO("signal up (%d) !!!\n",gpio.mount);
	}// end gpio_up
	else if (signum == GPIO_DOWN){	//长按按键事件
		switch(gpio.mount){
			case RESET_KEY://恢复出厂设置
				ResetHostDevicesFactory();
				break;
									
			case NETWORK_KEY://配网键
				LongNetKeyDown_ForConfigWifi();
				break;
				
			case SPEEK_KEY://会话键
				keyDown_AndSetGpioFor_play();
				TulingKeyDownSingal();
				break;
			
			case PLAY_PAUSE_KEY://长按播放/暂停  切换智能会话播音人
				keyDown_AndSetGpioFor_play();
				break;
			case ADDVOL_KEY:	//长按音量加
				keydown_flashingLED();
				mutiple_key_ADD.key_state  = KEYDOWN;
				mutiple_key_ADD.key_number = ADDVOL_KEY;
				if(mutiple_key_ADD.PthreadState == PthreadState_run)
					break;
				mutiple_key_ADD.PthreadState = PthreadState_run;
				gettimeofday(&mutiple_key_ADD.time_start,0);
				pool_add_task(mus_vol_mutiplekey_Thread,(void *)&mutiple_key_ADD);
				ack_VolCtr("add",GetVol());		//----------->音量减
				GpioLog("key down",ADDVOL_KEY);
				break;
			case SUBVOL_KEY:					//长按音量减
				keydown_flashingLED();
				mutiple_key_SUB.key_state  = KEYDOWN;
				mutiple_key_SUB.key_number = SUBVOL_KEY;
				gettimeofday(&mutiple_key_SUB.time_start,0);
				if(mutiple_key_SUB.PthreadState == PthreadState_run)
					break;
				
				mutiple_key_SUB.PthreadState = PthreadState_run;
				pool_add_task(mus_vol_mutiplekey_Thread,(void *)&mutiple_key_SUB);
				ack_VolCtr("sub",GetVol());		//----------->音量减
				GpioLog("key down",SUBVOL_KEY);
				break;
			case WEIXIN_SPEEK_KEY:	//华上教育微信对讲
				mutiple_key_weixin.key_state = KEYDOWN;
				mutiple_key_weixin.key_number = WEIXIN_SPEEK_KEY;
				gettimeofday(&mutiple_key_weixin.time_start,0);
				if(mutiple_key_weixin.PthreadState == PthreadState_run)
					break;
				
				mutiple_key_SUB.PthreadState = PthreadState_run;
				pool_add_task(weixin_mutiplekey_Thread,(void *)&mutiple_key_weixin);
				
				break;
			case DIR_MENU_KEY:	//长按切换单曲和列表
				keyDown_AndSetGpioFor_play();
				GpioKey_SetStreamPlayState();
				break;
		}// end gpio_down
		DEBUG_GPIO("signal down (%d) !!!\n",gpio.mount);
	}
	else{
		DEBUG_GPIO("not know signum ...\n");
	}
	unlock_msgEv();
}
//数据方向
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
//使能按键函数
void enable_gpio(void){
	ralink_gpio_reg_info info;
	gpio_set_dir(gpio6332);
	gpio_set_dir(gpio3200);

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
	info.pid = getpid();
	info.irq = DIR_MENU_KEY;
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
	info.irq = WEIXIN_SPEEK_KEY;
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
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
}
//初始化GPIO
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
#if 1	//使能恢复出厂设置按键
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
@ 初始化mtk76xx 系列gpio，注册gpio，并使能部分按键中断
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
	pool_add_task(Led_vigue_open,NULL);
	enableResetgpio();	//使能恢复出厂设置按键 (防止开机出现死机现象，无法操作)
}
//去使能按键
void disable_gpio(void){
	if (ioctl(gpio.fd, RALINK_GPIO_DISABLE_INTP) < 0) {
		perror("disable ioctl");
		close(gpio.fd);
		return ;
	}
	enableResetgpio();	//使能恢复出厂设置按键
}
//清除GPIO
void CleanMtk76xx_gpio(void){
	sleep(1);
	disable_gpio();
	close(gpio.fd);
}
