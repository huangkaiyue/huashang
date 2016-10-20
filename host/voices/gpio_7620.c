#include "comshead.h"
#include "host/voices/callvoices.h"
#include "ralink_gpio.h"
#include "gpio_7620.h"
#include "config.h"
//#define MAIN

static Gpio gpio;

static int led_type;

//开关音频驱动函数
void open_wm8960_voices(void)
{
	ioctl(gpio.fd, TANG_GPIO3924_OPEN);
}
void close_wm8960_voices(void)
{
	ioctl(gpio.fd, TANG_GPIO3924_CLOSE);
}
//系统灯开关
void open_sys_led(void)
{
	ioctl(gpio.fd, TANG_LED_CLOSE);
}
void close_sys_led(void)
{
	ioctl(gpio.fd, TANG_LED_OPEN);
}
void Led_vigue_open(void){
	led_type=LED_VIGUE_OPEN;
	while(led_type==LED_VIGUE_OPEN){
		close_sys_led();
		sleep(1);
		open_sys_led();
		sleep(1);
	}
	open_sys_led();
}
void Led_vigue_close(void){
	led_type=LED_VIGUE_CLOSE;
}

#ifdef LED_LR
void led_left_right(unsigned char type,unsigned char io)
{
	switch(type){
		case left:
			if(io==closeled)
				ioctl(gpio.fd, TANG_LED_OPEN_LEFT);
			else if(io==openled)
				ioctl(gpio.fd, TANG_LED_CLOSE_LEFT);
			break;
		case right:
			if(io==closeled)
				ioctl(gpio.fd, TANG_LED_OPEN_RIGHT);
			else if(io==openled)
				ioctl(gpio.fd, TANG_LED_CLOSE_RIGHT);
			break;
	}
}
#endif
//串口开关
static void uart_open(void){
	if (ioctl(gpio.fd, TANG_UART_OPEN) < 0) {
		perror("ioctl uart");
		return ;
	}
	DEBUG_GPIO("uart_open..\n");
}
//锁函数
static void lock_msgEv(void)
{
	gpio.sig_lock=1;
	
}
static void unlock_msgEv(void)
{
	gpio.sig_lock=0;
}
static int check_lock_msgEv(void)
{
	if(gpio.sig_lock==1){
		return -1;
	}
	return 0;
}
//按键处理事件
static void signal_handler(int signum)
{
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
			case RESERVE_KEY2:
#ifdef SPEEK_VOICES
				gpio.speek_tolk=TOLK;
#endif
				break;
			case SPEEK_KEY:
#ifdef SPEEK_VOICES
				if(gpio.speek_tolk==SPEEK){
					end_event_std();
				}else{
					create_event_voices_key(1);
				}
#else
				end_event_std();
#endif
				break;

			case ADDVOL_KEY:	//play last
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
					default:
						break;
				}
				break;
				
			case SUBVOL_KEY:	//play next
				switch(sysMes.localplayname){
					case mp3:
						createPlayEvent((const void *)"mp3",PLAY_LAST);
						break;
					case story:
						createPlayEvent((const void *)"story",PLAY_LAST);
						break;
					case english:
						createPlayEvent((const void *)"english",PLAY_LAST);
						break;
					default:
						break;
				}
				break;
		}
		DEBUG_GPIO("signal up (%d) !!!\n",gpio.mount);
	}// end gpio_up
	else if (signum == GPIO_DOWN){
		switch(gpio.mount){
			case RESET_KEY://恢复出厂设置
				create_event_system_voices(4);
				ResetDefaultRouter();
				break;
					
			case RESERVE_KEY2://会话对讲开关键
#ifdef SPEEK_VOICES
				gpio.speek_tolk=SPEEK;
#else
				exitqttsPlay();
				createPlayEvent((const void *)"mp3",PLAY_NEXT);
#endif
				break;
				
			case NETWORK_KEY://配网键
				Net_work();
				break;
				
			case SPEEK_KEY://会话键
#ifdef SPEEK_VOICES
				if(gpio.speek_tolk==SPEEK){
					down_voices_sign();
				}else{
					create_event_voices_key(0);
				}
#else
					down_voices_sign();
#endif
				break;
			
			case RESERVE_KEY1://预留键
				exitqttsPlay();
				createPlayEvent((const void *)"story",PLAY_NEXT);
				break;
#ifdef LED_LR
			case RESERVE_KEY3:	//目录
				exitqttsPlay();
				createPlayEvent((const void *)"english",PLAY_NEXT);
				break;
			case ADDVOL_KEY:	//vol +
				SetVol(1,0);
				break;
			case SUBVOL_KEY:	//vol -
				SetVol(0,0);
				break;
			case LETFLED_KEY:	//left led
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
				break;
			case RIGHTLED_KEY:
				Net_work();
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
void enable_gpio(void)
{
	ralink_gpio_reg_info info;
	gpio_set_dir(gpio6332);
#ifdef LED_LR
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
void init_7620_gpio(void)
{
	memset(&gpio,0,sizeof(Gpio));
	gpio.fd = open(GPIO_DEV, O_RDONLY);
	if (gpio.fd < 0) {
		perror(GPIO_DEV);
		return ;
	}
	uart_open();
#if 0
	close_sys_led();
#else
	pool_add_task(Led_vigue_open,NULL);
#endif
#ifdef SPEEK_VOICES
	if (ioctl(gpio.fd,TANG_GET_DATA_3264,&gpio.data) < 0){
		perror("ioctl");
		close(gpio.fd);
		return ;
	}
#if 1
	if((0x01&(gpio.data>>7))==1){
		gpio.speek_tolk=TOLK;
	}else{
		gpio.speek_tolk=SPEEK;
	}
#else
	gpio.speek_tolk=TOLK;
#endif
#endif
	enable_gpio();
}
//去使能按键
void disable_gpio(void)
{
	if (ioctl(gpio.fd, RALINK_GPIO_DISABLE_INTP) < 0) {
		perror("disable ioctl");
		close(gpio.fd);
		return ;
	}
}
//清除GPIO
void clean_7620_gpio(void)
{
	//close_sys_led();
	disable_gpio();
	close(gpio.fd);
}
#ifdef MAIN
int main(void)
{
	init_7620_gpio();
	DEBUG_GPIO("init_7620_gpio \n");
	open_wm8960_voices();
	while(1){
		pause();
	}
	DEBUG_GPIO("clean_7620_gpio \n");
	clean_7620_gpio();
	return 0;
}
#endif //end  MAIN
