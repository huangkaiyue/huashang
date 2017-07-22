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

#define	RESET_KEY					38	//恢复出厂设置键
#define RESERVE_KEY2				39	//会话对讲开关键

#define	NETWORK_KEY					40	//配网键
#define	SPEEK_KEY					41	//会话键
#define	PLAY_PAUSE_KEY				42	//播放/暂停

#define DIR_MENU_KEY				20	//目录切换按键

#define ADDVOL_KEY					16	//上一曲--音量加
#define SUBVOL_KEY					17	//下一曲--音量减


#define RIGHTLED_KEY				14	//微信发送过来的绑定请求
				
#define WEIXIN_SPEEK_KEY			21	//微信对讲


#define GPIO_DEV	"/dev/gpio"
#define SPEEK		0		//微信对讲状态
#define TOLK		1		//智能会话状态

#define BIND_DEV_ER	0
#define BIND_DEV_OK	1

#define KEYDOWN	0	//按下
#define KEYUP	1	//弹起



enum{
	left=0,
	right,
};
enum{
	openled=0,
	closeled,
};

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
	unsigned char sig_lock:4,//锁
		bindsign:4;
}Gpio;

//用于按键长短按复用，主要用于音量加减与上下曲按键复用
typedef struct {
	unsigned char PthreadState;
	unsigned int key_number;	//按键号
	unsigned int key_state;		//按键状态:按下/弹起
	
	struct timeval time_start,time_end;
}key_mutiple_t;

#define PthreadState_exit 	0
#define PthreadState_run	1

extern void open_wm8960_voices(void);
extern void close_wm8960_voices(void);
extern void led_lr_oc(unsigned char type);
extern void enable_gpio(void);
extern void InitMtk76xx_gpio(void);
extern void disable_gpio(void);
extern void clean_7620_gpio(void);
extern void Led_vigue_open(void);
extern void Led_vigue_close(void);

#endif
