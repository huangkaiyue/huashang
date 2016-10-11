#ifndef _GPIO_7620_H
#define _GPIO_7620_H

#include "config.h"

#define DBG_GPIO
#ifdef 	DBG_GPIO 
#define DEBUG_GPIO(fmt, args...) printf("Gpio: " fmt, ## args)
#else   
#define DEBUG_GPIO(fmt, args...) { }
#endif	//end DBG_AP_STA

#define GPIO_UP		SIGUSR1	//弹起事件
#define GPIO_DOWN	SIGUSR2	//按下事件

#define	RESET_KEY		38	//恢复出厂设置键
#define RESERVE_KEY2	39	//预留键
#define	NETWORK_KEY		40	//配网键
#define	SPEEK_KEY		41	//会话键
#define	RESERVE_KEY1	42	//预留键

#define RESERVE_KEY3	14	//预留键
#define ADDVOL_KEY		16	//音量加
#define SUBVOL_KEY		17	//音量减
#define LETFLED_KEY		20	//左灯
#define RIGHTLED_KEY	21	//右灯

#define GPIO_DEV	"/dev/gpio"

#ifdef LED_LR
enum{
	left=0,
	right,
};
enum{
	openled=0,
	closeled,
};
#endif

enum{
	gpio3200,
	gpio6332,
	gpio9564,
};
typedef struct {
	int fd;
	unsigned char mount;//中断gpio口
	unsigned char sig_lock:1;//锁
}Gpio;

extern void open_wm8960_voices(void);
extern void close_wm8960_voices(void);
extern void led_left_right(unsigned char type,unsigned char io);
extern void enable_gpio(void);
extern void init_7620_gpio(void);
extern void disable_gpio(void);
extern void clean_7620_gpio(void);
#endif