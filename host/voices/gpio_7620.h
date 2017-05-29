#ifndef _GPIO_7620_H
#define _GPIO_7620_H

#include "config.h"

#define DBG_GPIO
#ifdef 	DBG_GPIO 
#define DEBUG_GPIO(fmt, args...) printf("Gpio: " fmt, ## args)
#else   
#define DEBUG_GPIO(fmt, args...) { }
#endif	//end DBG_AP_STA

#define GPIO_UP		SIGUSR1	//�����¼�--�˰��¼�
#define GPIO_DOWN	SIGUSR2	//�����¼�--�����¼�

#define	RESET_KEY					38	//�ָ��������ü�
#define RESERVE_KEY2				39	//�Ự�Խ����ؼ�

#define	NETWORK_KEY					40	//������
#define	SPEEK_KEY					41	//�Ự��
#define	PLAY_PAUSE_KEY				42	//����/��ͣ

#define RESERVE_KEY3				20	//�ղظ���

#define ADDVOL_KEY					16	//��һ��--������
#define SUBVOL_KEY					17	//��һ��--������


#define RIGHTLED_KEY				14	//΢�ŷ��͹����İ�����

#define LETFLED_KEY					21	//����
#define HUASHANG_WEIXIN_SPEEK_KEY	LETFLED_KEY	//���Ͻ���΢�ŶԽ�
#define WEIXIN_SPEEK_KEY			LETFLED_KEY	//΢�ŶԽ�


#define GPIO_DEV	"/dev/gpio"
#define SPEEK		0		//΢�ŶԽ�״̬
#define TOLK		1		//���ܻỰ״̬

#define BIND_DEV_ER	0
#define BIND_DEV_OK	1

#define VOLKEYDOWN	0	//����
#define VOLKEYUP	1	//����



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
	unsigned int mount;//�ж�gpio��
	unsigned char sig_lock:2,//��
			speek_tolk:2,
			ledletf:2,
			ledright:2;
	unsigned char callbake:2,
		bindsign:6;
	unsigned int data;
}Gpio;

extern void open_wm8960_voices(void);
extern void close_wm8960_voices(void);
extern void led_lr_oc(unsigned char type);
extern void Led_System_vigue_open(void);
extern void Led_System_vigue_close(void);
extern void enable_gpio(void);
extern void InitMtk76xx_gpio(void);
extern void disable_gpio(void);
extern void clean_7620_gpio(void);
extern void Led_vigue_open(void);
extern void Led_vigue_close(void);

#endif
