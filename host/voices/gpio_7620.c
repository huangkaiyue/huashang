#include <stdio.h>             
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/autoconf.h>

#include "ralink_gpio.h"
#include "host/voices/callvoices.h"
#include "gpio_7620.h"
#include "config.h"
//#define MAIN

static Gpio gpio;

//������Ƶ��������
void open_wm8960_voices(void)
{
	ioctl(gpio.fd, TANG_GPIO3924_OPEN);
}
void close_wm8960_voices(void)
{
	ioctl(gpio.fd, TANG_GPIO3924_CLOSE);
}
//ϵͳ�ƿ���
void open_sys_led(void)
{
	ioctl(gpio.fd, TANG_LED_OPEN);
}
void close_sys_led(void)
{
	ioctl(gpio.fd, TANG_LED_CLOSE);
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
//���ڿ���
static void uart_open(void){
	if (ioctl(gpio.fd, TANG_UART_OPEN) < 0) {
		perror("ioctl uart");
		return ;
	}
	DEBUG_GPIO("uart_open..\n");
}
//������
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
//���������¼�
unsigned char ledletf=0;
unsigned char ledright=0;
static void signal_handler(int signum)
{
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
			case SPEEK_KEY:
#ifdef	SPEEK
				end_event_std();
#else
				create_event_voices_key(1);
#endif
				break;
		}
		DEBUG_GPIO("signal up (%d) !!!\n",gpio.mount);
	}// end gpio_up
	else if (signum == GPIO_DOWN){
		switch(gpio.mount){
			case RESET_KEY://�ָ���������
				create_event_system_voices(4);
				ResetDefaultRouter();
				break;
					
			case RESERVE_KEY2://Ԥ����
				exitqttsPlay();
				createPlayEvent((const void *)"mp3");
				break;
				
			case NETWORK_KEY://������
				Net_work();
				break;
				
			case SPEEK_KEY://�Ự��
#ifdef	SPEEK
				down_voices_sign();
#else
				create_event_voices_key(0);
#endif
				break;
			
			case RESERVE_KEY1://Ԥ����
				exitqttsPlay();
				createPlayEvent((const void *)"story");
				break;
#ifdef LED_LR
			case RESERVE_KEY3:	//Ŀ¼
				exitqttsPlay();
				createPlayEvent((const void *)"english");
				break;
			case ADDVOL_KEY:	//vol +
				SetVol(1,0);
				break;
			case SUBVOL_KEY:	//vol -
				SetVol(0,0);
				break;
			case LETFLED_KEY:	//left led
				if(ledletf==1){
					ledletf=0;
					led_left_right(left,closeled);
				}else{
					ledletf=1;
					led_left_right(left,openled);
				}
				break;
			case RIGHTLED_KEY:	//right led
				if(ledright==1){
					ledright=0;
					led_left_right(right,closeled);
				}else{
					ledright=1;
					led_left_right(right,openled);
				}
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

//���ݷ���
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
//ʹ�ܰ�������
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
//��ʼ��GPIO
void init_7620_gpio(void)
{
	memset(&gpio,0,sizeof(Gpio));
	gpio.fd = open(GPIO_DEV, O_RDONLY);
	if (gpio.fd < 0) {
		perror(GPIO_DEV);
		return ;
	}
	uart_open();
	//open_sys_led();
	enable_gpio();
}
//ȥʹ�ܰ���
void disable_gpio(void)
{
	if (ioctl(gpio.fd, RALINK_GPIO_DISABLE_INTP) < 0) {
		perror("disable ioctl");
		close(gpio.fd);
		return ;
	}
}
//���GPIO
void clean_7620_gpio(void)
{
	close_sys_led();
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
