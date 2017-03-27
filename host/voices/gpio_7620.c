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
#define ONCEVOL		//音量加一次

#ifndef ONCEVOL
#define VOLWAITTIME		300*1000	//音量加减时间间隔
#define VOLKEY_CHANG	2			//长按时间
static int volstart_time=0;
static unsigned char voltype=VOLKEYUP;
static unsigned char KeyNum=0;
static void handlevolandnext(void){
	time_t t;
	int volendtime=0;
	int ret;
	while(1){
		volendtime=time(&t);
		printf("volendtime-volstart_time = %d\n",volendtime-volstart_time);
		if((volendtime-volstart_time)<VOLKEY_CHANG){
			if(voltype==VOLKEYUP){
				//下一曲
				if(KeyNum==ADDVOL_KEY){
					createPlayEvent((const void *)"xiai",PLAY_NEXT);	//下一曲
				}else if(KeyNum==SUBVOL_KEY){
					createPlayEvent((const void *)"xiai",PLAY_PREV);	//上一曲
				}
				break;
			}
			usleep(10*1000);
			continue;
		}else if((volendtime-volstart_time)<5){
			if(voltype==VOLKEYUP){
				break;
			}
			if(KeyNum==ADDVOL_KEY){
				ret = Setwm8960Vol(VOL_ADD,0);	//音量加
			}else if(KeyNum==SUBVOL_KEY){
				ret = Setwm8960Vol(VOL_SUB,0);	//音量减
			}
			if(ret==1){	//音量加满了
				break;
			}
			//音量加
			usleep(VOLWAITTIME);
		}else{
			break;
		}
	}
	KeyNum=0;
}
/*
@ 复用按键接口，短按切换歌曲，长按设置音量加
@ 
@
*/
static void VolAndNextKey(unsigned char state,unsigned char dir){
	time_t t;
	if(KeyNum==0||dir==KeyNum){
		if(state==VOLKEYDOWN){	//按下
			volstart_time=time(&t);
			voltype=VOLKEYDOWN;
			KeyNum=dir;
			pool_add_task(handlevolandnext,NULL);
		}else{			//弹起
			voltype=VOLKEYUP;
		}
	}
}
#endif

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
#ifdef QITUTU_SHI
	open_sys_led();	//
#endif
#ifdef TANGTANG_LUO
	open_sys_led();	//
#endif
#ifdef DATOU_JIANG
	close_sys_led();
#endif
}

void Led_vigue_close(void){
	led_type=LED_VIGUE_CLOSE;
}
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
#ifdef QITUTU_SHI
	led_lr_oc(openled);
#endif
#ifdef TANGTANG_LUO
	led_lr_oc(openled);
#endif
#ifdef DATOU_JIANG
	led_lr_oc(closeled);
#endif
}
void Led_System_vigue_close(void){
	led_system_type=LED_VIGUE_CLOSE;
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
//获取wifi 名字并播放
static void GetWifiName_AndIpaddressPlay(void){
	char buf[128]={0};
	char *wifi = nvram_bufget(RT2860_NVRAM, "ApCliSsid");
	char IP[20]={0};
	GetNetworkcardIp((char * )"apcli0",IP);
	if(strlen(wifi)>0){
		snprintf(buf,128,"已连接 wifi %s  IP地址是 %s",wifi,IP);
		Create_PlayQttsEvent(buf,QTTS_GBK);
	}
}
#ifdef QITUTU_SHI 
//按键按下绑定用户请求
static void keyDownAck_userBind(void){
	if(gpio.bindsign==BIND_DEV_OK){
		BindDevToaliyun();
		Create_PlaySystemEventVoices(BIND_OK_PLAY);
		gpio.bindsign=BIND_DEV_ER;
	}else{//没有接收到绑定请求
		Create_PlayQttsEvent("快去邀请小伙伴一起来聊天吧。",QTTS_GBK);
	}
}
//响应微信呼叫请求
static void Ack_WeixinCall(void){
	if(gpio.callbake==1){
		Create_PlaySystemEventVoices(TALK_CONFIRM_OK_PLAY);//"确认消息回复成功，请发上传语音。"
		gpio.callbake==0;
		Ack_CallDev();
	}else{
		if(getEventNum()==0){	//防止添加的事件太多
			Create_PlaySystemEventVoices(TALK_CONFIRM_ER_PLAY);//"当前还没有人呼叫你"
		}
	}
}
#endif

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
void EnableBindDev(void){
	gpio.bindsign=BIND_DEV_OK;
}
void EnableCallDev(void){
	gpio.callbake=1;
}
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

//按键处理事件
//-------------------------------------------QITUTU_SHI--------------------------------------------
#ifdef QITUTU_SHI
static void signal_handler(int signum){
	//拿到底层按键事件号码
	if (ioctl(gpio.fd, TANG_GET_NUMBER,&gpio.mount) < 0){
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
	if(check_lock_msgEv()){
		return ;
	}
	lock_msgEv();
	if (signum == GPIO_UP){			//短按按键事件
		switch(gpio.mount){
			case NETWORK_KEY:		//播报WiFi名
				GetWifiName_AndIpaddressPlay();
				break;
				
			case RESERVE_KEY2:
#ifdef SPEEK_VOICES1
				//gpio.speek_tolk=SPEEK;
				ReadSpeekGpio();
#endif
				break;

			case SPEEK_KEY:
				if(gpio.speek_tolk==SPEEK){
					end_event_std();
				}else{
					Create_WeixinSpeekEvent(VOLKEYUP);
				}
				break;
#ifdef 	LOCAL_MP3
			case ADDVOL_KEY:	//短按播放喜爱内容,下一曲
				keydown_flashingLED();
#ifdef ONCEVOL	
				createPlayEvent((const void *)"xiai",PLAY_PREV);
#else
				VolAndNextKey(VOLKEYUP,ADDVOL_KEY);
#endif
				GpioLog("key up",ADDVOL_KEY);
				break;
			case SUBVOL_KEY:	//短按播放喜爱内容,上一曲
				keydown_flashingLED();
#ifdef ONCEVOL
				createPlayEvent((const void *)"xiai",PLAY_NEXT);
#else
				VolAndNextKey(VOLKEYUP,SUBVOL_KEY);
#endif
				GpioLog("key up",SUBVOL_KEY);
				break;
#endif
			case RESERVE_KEY1:	//播放、暂停
				keyStreamPlay();
				break;
			case RESERVE_KEY3:	//play last
				keydown_flashingLED();
				CreateLikeMusic();
				break;
			case LETFLED_KEY:	//回复键
				Ack_WeixinCall();
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
				Create_PlaySystemEventVoices(4);	//需要修改语音如下:
				//ResetDefaultRouter();
				system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan gpio");
				break;
					
			case RESERVE_KEY2://会话对讲开关键
				//gpio.speek_tolk=TOLK;
				ReadSpeekGpio();
				break;
				
			case NETWORK_KEY://配网键
				NetKeyDown_ForConfigWifi();
				break;
				
			case SPEEK_KEY://会话键
#ifdef	SPEEK_VOICES1
				ReadSpeekGpio();	//-----bug
#endif
				if(gpio.speek_tolk==SPEEK){
					TulingKeyDownSingal();
				}else{
					Create_WeixinSpeekEvent(VOLKEYDOWN);
				}			
				break;
			
			case RESERVE_KEY1://预留键
				break;
#ifdef	LED_LR
			case ADDVOL_KEY:	//长按音量加
				keydown_flashingLED();
#ifdef ONCEVOL
				Setwm8960Vol(GVOL_ADD,0);
#else
				VolAndNextKey(VOLKEYDOWN,ADDVOL_KEY);
#endif
				ack_VolCtr("add",GetVol());		//----------->音量减
				GpioLog("key down",ADDVOL_KEY);
				break;
			case SUBVOL_KEY:	//长按音量减
				keydown_flashingLED();
#ifdef ONCEVOL
				Setwm8960Vol(GVOL_SUB,0);
#else
				VolAndNextKey(VOLKEYDOWN,SUBVOL_KEY);
#endif
				ack_VolCtr("sub",GetVol());		//----------->音量减
				GpioLog("key down",SUBVOL_KEY);
				break;
			case LETFLED_KEY:	//play next
				break;
#endif
			case RESERVE_KEY3:	//长按，删除收藏歌曲
				DelLikeMusic();
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
	//拿到底层按键事件号码
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
			case NETWORK_KEY:		//短按播报WiFi名
				GetWifiName_AndIpaddressPlay();
				break;

			case SPEEK_KEY:
#ifdef SPEEK_VOICES
				if(gpio.speek_tolk==SPEEK){
					end_event_std();
				}else{
					Create_WeixinSpeekEvent(VOLKEYUP);
				}
#else
				end_event_std();
#endif
				break;
			
#ifdef 	LOCAL_MP3
			case ADDVOL_KEY:	//play last
				switch(sysMes.localplayname){
					case mp3:
						createPlayEvent((const void *)"mp3",PLAY_PREV);
						break;
					case story:
						createPlayEvent((const void *)"story",PLAY_PREV);
						break;
					case english:
						createPlayEvent((const void *)"english",PLAY_PREV);
						break;
					case guoxue:
						createPlayEvent((const void *)"guoxue",PLAY_PREV);
						break;
					default:
						break;
				}
				break;
			case SUBVOL_KEY:	//play next
				switch(sysMes.localplayname){
					case mp3:
						createPlayEvent((const void *)"mp3",PLAY_NEXT);
						break;
					case story:
						createPlayEvent((const void *)"story",PLAY_NEXT);
						break;
					case english:
						createPlayEvent((const void *)"english",PLAY_NEXT);
						break;
					case guoxue:
						createPlayEvent((const void *)"guoxue",PLAY_NEXT);
						break;
					default:
						break;
				}
				break;
#endif
			case RIGHTLED_KEY:
#if 0
				if(gpio.ledletf==1){
					gpio.ledletf=0;
					led_left_right(left,closeled);
				}else{
					gpio.ledletf=1;
					led_left_right(left,openled);
				}
				if(gpio.ledright==1){	//right led
					gpio.ledright=0;
					led_left_right(right,closeled);
				}else{
					gpio.ledright=1;
					led_left_right(right,openled);
				}
#else
				if(sysMes.localplayname==english){
					keyStreamPlay();
				}else{
					createPlayEvent((const void *)"english",PLAY_NEXT);
				}
#endif
				break;
#ifdef	LOCAL_MP3
			case RESERVE_KEY2:
#ifdef	SPEEK_VOICES1
				//gpio.speek_tolk=SPEEK;
				ReadSpeekGpio();
#else
				if(sysMes.localplayname==english){
					keyStreamPlay();
				}else{
					createPlayEvent((const void *)"english",PLAY_NEXT);
				}
#endif
				break;

			case RESERVE_KEY1://预留键
				if(sysMes.localplayname==guoxue){
					keyStreamPlay();
				}else{
					createPlayEvent((const void *)"guoxue",PLAY_NEXT);
				}
				break;
			case RESERVE_KEY3:	//目录
				if(sysMes.localplayname==mp3){
					keyStreamPlay();
				}else{
					createPlayEvent((const void *)"mp3",PLAY_NEXT);
				}
				break;
			case LETFLED_KEY:	//left led
				if(sysMes.localplayname==story){
					keyStreamPlay();
				}else{
					createPlayEvent((const void *)"story",PLAY_NEXT);
				}
				break;
#endif
		}
		DEBUG_GPIO("signal up (%d) !!!\n",gpio.mount);
	}// end gpio_up
	else if (signum == GPIO_DOWN){
		switch(gpio.mount){
			case RESET_KEY://恢复出厂设置
				Create_PlaySystemEventVoices(4);
				//ResetDefaultRouter();
				system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan gpio");
				break;
					
			case RESERVE_KEY2://会话对讲开关键
#ifdef	SPEEK_VOICES1
				//gpio.speek_tolk=TOLK;
				ReadSpeekGpio();
#endif
				break;
				
			case NETWORK_KEY://配网键
				NetKeyDown_ForConfigWifi();
				break;
				
			case SPEEK_KEY://会话键
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
			
			case RESERVE_KEY1://预留键
				break;
#ifdef	LED_LR
			case RESERVE_KEY3:	//目录
				break;
			case ADDVOL_KEY:	//vol +
				Setwm8960Vol(GVOL_ADD,0);
				ack_VolCtr("add",GetVol());//----------->音量减
				break;
			case SUBVOL_KEY:	//vol -
				Setwm8960Vol(GVOL_SUB,0);
				ack_VolCtr("sub",GetVol());//----------->音量减
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
	//拿到底层按键事件号码
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
			case NETWORK_KEY:		//播报WiFi名
				GetWifiName_AndIpaddressPlay();
				break;

			case SPEEK_KEY:
				end_event_std();
				break;
		}
		DEBUG_GPIO("signal up (%d) !!!\n",gpio.mount);
	}// end gpio_up
	else if (signum == GPIO_DOWN){
		switch(gpio.mount){
			case RESET_KEY://恢复出厂设置
				Create_PlaySystemEventVoices(4);
				//ResetDefaultRouter();
				system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan gpio");
				break;
				
			case NETWORK_KEY://配网键
				NetKeyDown_ForConfigWifi();
				break;
				
			case SPEEK_KEY://会话键
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

//数据方向
static int gpio_set_dir(int r){
	int req;
	if (r == gpio6332){
		req = RALINK_GPIO6332_SET_DIR_IN;
		if (ioctl(gpio.fd, req, (0x1f<<6)) < 0) {
			perror("ioctl");
			close(gpio.fd);
			return -1;
		}
	}
	else if (r == gpio9564)
		req = RALINK_GPIO9564_SET_DIR_IN;
	else{
		req = RALINK_GPIO_SET_DIR_IN;
		if (ioctl(gpio.fd, req, (0xcd<<14)) < 0) {
			perror("ioctl");
			close(gpio.fd);
			return -1;
		}
	}
}
//使能按键函数
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
	info.irq = RESERVE_KEY1;
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
//初始化GPIO
static void enableResetgpio(void){
	if (ioctl(gpio.fd, RALINK_GPIO6332_SET_DIR_IN, (0x1<<6)) < 0) {
		perror("ioctl");
		close(gpio.fd);
		return -1;
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
#if	0
	close_sys_led();
#else
	pool_add_task(Led_vigue_open,NULL);
#endif
#ifdef SPEEK_VOICES
	ReadSpeekGpio();	//读取会话对讲功能拨动键
#endif
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
