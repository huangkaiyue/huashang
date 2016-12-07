#ifndef _GPIO_7620_H
#define _GPIO_7620_H

#include "config.h"

#define DBG_GPIO
#ifdef 	DBG_GPIO 
#define DEBUG_GPIO(fmt, args...) printf("Gpio: " fmt, ## args)
#else   
#define DEBUG_GPIO(fmt, args...) { }
#endif	//end DBG_AP_STA

#define GPIO_UP		SIGUSR1	//弹起事件--端按事件
#define GPIO_DOWN	SIGUSR2	//按下事件--长按事件
#if 0
#define	RESET_KEY		39	//恢复出厂设置键
#define RESERVE_KEY2	38	//会话对讲开关键
#else
#define	RESET_KEY		38	//恢复出厂设置键
#define RESERVE_KEY2	39	//英语
#endif
#define	NETWORK_KEY		40	//配网键
#define	SPEEK_KEY		41	//会话键
#define	RESERVE_KEY1	42	//科技

#define RESERVE_KEY3	21	//儿歌
#define ADDVOL_KEY		16	//上一曲--音量加
#define SUBVOL_KEY		17	//下一曲--音量减
#define LETFLED_KEY		20	//素质
#define RIGHTLED_KEY	14	//配网--灯

#define GPIO_DEV	"/dev/gpio"
#define SPEEK		0
#define TOLK		1
#define BIND_DEV	2

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

#define	LED_VIGUE_OPEN	1
#define LED_VIGUE_CLOSE	0

enum{
	gpio3200,
	gpio6332,
	gpio9564,
};
typedef struct {
	int fd;
	unsigned int mount;//中断gpio口
	unsigned char sig_lock:2,//锁
			speek_tolk:2,
			ledletf:2,
			ledright:2;
	unsigned int data;
}Gpio;

extern void open_wm8960_voices(void);
extern void close_wm8960_voices(void);
extern void led_left_right(unsigned char type,unsigned char io);
extern void Led_System_vigue_open(void);
extern void enable_gpio(void);
extern void init_7620_gpio(void);
extern void disable_gpio(void);
extern void clean_7620_gpio(void);
extern void Led_vigue_open(void);
extern void Led_vigue_close(void);

#endif
