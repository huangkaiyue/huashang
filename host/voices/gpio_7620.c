#include "comshead.h"
#include "host/voices/callvoices.h"
#include "ralink_gpio.h"
#include "gpio_7620.h"
#include "host/studyvoices/qtts_qisc.h"
#include "host/voices/wm8960i2s.h"
#include "huashangMusic.h"
#include "host/studyvoices/prompt_tone.h"
#include "log.h"
#include "nvram.h"
#include "config.h"

static Gpio gpio;
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
void Stop_light_500Hz(void){
	gpio.lightRunState=LIGHT_500HZ_STOP;
}
static void *__Running_light_500Hz(void *arg){
	while(gpio.lightRunState==LIGHT_500HZ_RUNING){
		close_sys_led();
		usleep(500*1000);
		open_sys_led();
		usleep(500*1000);
	}
	open_sys_led();	
	return NULL;
}
void Running_light_500Hz(void){
	if(gpio.lightRunState=LIGHT_500HZ_RUNING){
		return;
	}
	led_lr_oc(closeled);
	gpio.lightRunState=LIGHT_500HZ_RUNING;
	pool_add_task(Running_light_500Hz,NULL);
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
	Create_PlaySystemEventVoices(CMD_56_RESET_SYSTEM);
	system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan gpio");
}
//接收到微信发送过来的绑定请求
void EnableBindDev(void){	
	gpio.bindsign=BIND_DEV_OK;
}

//按键按下绑定用户请求
static int keyDownAck_userBind(void){
	if(gpio.bindsign==BIND_DEV_OK){
		gpio.bindsign=BIND_DEV_ER;
		BindDevToaliyun();
		Create_PlaySystemEventVoices(CMD_28_HANDLE_BIND);
		return 0;
	}
	return -1;
}
void keyDown_AndSetGpioFor_clean(void){
	//if(GetRecordeVoices_PthreadState()!=PLAY_MP3_MUSIC){
		ioctl(gpio.fd, AUDIO_IC_CONTROL,0x00);//按下
	//}
}

//按下音
void keyDown_AndSetGpioFor_play(void){
	if(GetRecordeVoices_PthreadState()!=PLAY_MP3_MUSIC){
		printf("play key down voices \n");
		ioctl(gpio.fd, AUDIO_IC_CONTROL,0x01);//按下
	}
	keyDown_AndSetGpioFor_clean();
}

//抬起音
void keyUp_AndSetGpioFor_play(void){
	if(GetRecordeVoices_PthreadState()==PLAY_MP3_MUSIC){
		StreamPause();
		usleep(100000);
		ioctl(gpio.fd, AUDIO_IC_CONTROL,0x20);//弹起
		usleep(200000);
		StreamPlay();
	}else{
		ioctl(gpio.fd, AUDIO_IC_CONTROL,0x20);//弹起
	}
	keyDown_AndSetGpioFor_clean();
}	

//串口开关
static void enableUart(void){
	if (ioctl(gpio.fd, TANG_UART_OPEN) < 0) {
		perror("ioctl uart");
		return ;
	}
	DEBUG_GPIO("enableUart..\n");
}
//锁gpio事件函数，防止底层触发按键太频繁
static void lock_msgEv(void){
	printf("%s: lock \n",__func__);
	gpio.sig_lock=1;	
}
//解锁函数
static void unlock_msgEv(void){
	printf("%s: un lock \n",__func__);
	gpio.sig_lock=0;
}
//检查当前锁
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
					Huashang_GetScard_forPlayMusic(PLAY_NEXT,EXTERN_PLAY_EVENT);
				}
				else{
					printf("[ %s ]:[ %s ] printf in line [ %d ]\n",__FILE__,__func__,__LINE__);
					Huashang_GetScard_forPlayMusic(PLAY_PREV,EXTERN_PLAY_EVENT);
				}
				break;

			}
			usleep(100 * 1000);
			continue;
		}
		
		if(time_ms >=500){		//before is 500  2017.6.28 22:43
			if(playDownVoicesFlag==0){
				//keyUp_AndSetGpioFor_play();
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
				//keyDown_AndSetGpioFor_play();
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

void PlayWakeUpVoices(void){
	printf("%s: wake up system\n",__func__);
	if (GetWeixinMessageFlag()==NOT_MESSAGE) {		
		//Create_PlayImportVoices(CMD_20_CONNET_OK); 		//20、(8634代号)小培老师与总部课堂连接成功，我们来聊天吧！（每次连接成功的语音，包括唤醒）
/*
		if(checkNetWorkLive(DISABLE_CHECK_VOICES_PLAY)){
			Handle_PlayTaiBenToNONetWork(GetCurrentEventNums());
		}else{
			PlayImportVoices(AMR_20_CONNET_OK, GetCurrentEventNums());	
		}
*/		
		PlayImportVoices(AMR_20_CONNET_OK, GetCurrentEventNums());
	}else if(GetWeixinMessageFlag()==WEIXIN_MESSAGE){
		//Create_PlayImportVoices(CMD_24_WAKEUP_RECV_MSG); //24、你有新消息，请按信息键听取吧！（唤醒之后播放，播放网络成功之后）
		PlayImportVoices(AMR_24_NEW_MESSAGE, GetCurrentEventNums());
	}else if(GetWeixinMessageFlag()==WEIXIN_PUSH_MESSAGE){
		//Create_PlayImportVoices(CMD_25_WAKEUP_RECV_MSG); //25、你有新故事未听取,按信息键开始听吧！（唤醒之后播放，播放网络成功之后）
		PlayImportVoices(AMR_25_NEW_STROY, GetCurrentEventNums());
	}
	unlock_msgEv();
}
static void signal_handler(int signum){
	static key_mutiple_t mutiple_key_SUB,mutiple_key_ADD,mutiple_key_speek,mutiple_key_weixin;
	//printf("%s : recv signum =%d\n",__func__,signum);
	//拿到底层按键事件号码
	if (ioctl(gpio.fd, TANG_GET_NUMBER,&gpio.mount) < 0){
		perror("ioctl");
		return ;
	}
	//printf("%s : signum = %d gpio.mount=%d\n",__func__,signum,gpio.mount);
	if(check_lock_msgEv()&&gpio.mount!=RESET_KEY){
		printf("error is lock signal_handler\n");
		return ;
	}
	lock_msgEv();
	if(checkAndWakeupSystem()){
		printf("%s : signum = %d gpio.mount=%d  exit\n",__func__,signum,gpio.mount);
		return ;
	}
	//printf("%s : signum = %d gpio.mount=%d  GPIO_UP=%d\n",__func__,signum,gpio.mount,GPIO_UP);
	if (signum == GPIO_UP){			//短按按键事件
		switch(gpio.mount){
			case NETWORK_KEY:		//播报WiFi名
				ShortKeyDown_ForPlayWifiMessage();
				break;
			case SPEEK_KEY:			//智能会话按键事件
				//keyUp_AndSetGpioFor_play();
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
				//keyUp_AndSetGpioFor_play();
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
				if(!LongNetKeyDown_ForConfigWifi()){
					return;
				}
				break;
				
			case SPEEK_KEY://会话键
				if(!keyDownAck_userBind()){
					goto exit0;
				}
				TulingKeyDownSingal();
				printf("end ... TulingKeyDownSingal ...\n");
				break;
			
			case PLAY_PAUSE_KEY://长按播放/暂停  切换智能会话播音人
				//keyUp_AndSetGpioFor_play();
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
				//printf("start get sub time \n");
				if(mutiple_key_SUB.PthreadState == PthreadState_run)
					break;
				//printf("start run sub pthread \n");
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
				
				mutiple_key_weixin.PthreadState = PthreadState_run;
				pool_add_task(weixin_mutiplekey_Thread,(void *)&mutiple_key_weixin);
				
				break;
			case DIR_MENU_KEY:	//长按切换单曲和列表
				//keyUp_AndSetGpioFor_play();
				GpioKey_SetStreamPlayState();
				break;
		}// end gpio_down
		DEBUG_GPIO("signal down (%d) !!!\n",gpio.mount);
	}
	else{
		DEBUG_GPIO("unknow signum ...\n");
	}
exit0:	
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
	unlock_msgEv();
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
	//使能恢复出厂设置按键
	ralink_gpio_reg_info info;
	info.pid = getpid();
	info.irq = RESET_KEY;
	if (ioctl(gpio.fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return ;
	}

	gpio_set_dir(gpio6332);
	gpio_set_dir(gpio3200);

//------------------------------------------------------------
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
	printf("................enable gpio.......... ok \n");
}
//去使能按键
void disable_gpio(void){
	lock_msgEv();
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
	gpio.lightRunState=LIGHT_500HZ_STOP;
	enableUart();
	Running_light_500Hz();
	enableResetgpio();	//使能恢复出厂设置按键 (防止开机出现死机现象，无法操作)
	disable_gpio();
}

//清除GPIO
void CleanMtk76xx_gpio(void){
	sleep(1);
	disable_gpio();
	close(gpio.fd);
}
