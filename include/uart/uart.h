#ifndef	UART_H_
#define UART_H_


#define MSTIME		0x16	//当前时间 0001 0110 ->0b01101000
#define MSOPEN		0x23	//定时开 0010 0011 ->0b01000100
#define MSCLOSE		0x33	//定时关 0011 0011 ->0b01001100
#define MSBATTERY	0x81	//请求电量
#define MSCLOCK		0xa3	//闹钟开机 1010 0011

#define SMCLOSETIME	0x42	//MCU定时定时到关机信号发送 0100 0010 ->0b01000010
#define SMBATTERY	0x52	//电池电量 0101 0010 ->0b01001010
#define SMCLOSE		0x62	//开关机提示 0110 0010 ->0b01000110
#define SMBATTYPE	0x72	//充电状态 0111 0010 ->0b01001110
#define SMOK		0xf2	//握手信号 1111 0010 ->0b01001111
#define SMLOW		0x91	//低电提醒

#define FACECMD		0x02

#define OPEN				1
#define MUC_CLOSE_SYSTEM	2
#define TIME				3
#define BATTYPE				4
#define SMOKER_OK			5
#define SMOKER_ER			6	
#define CLOCK				7
#define FACE				8

#define UART_EVENT_CLOSE_SYSTEM	1	//mcu关机事件
#define UART_EVENT_LOW_BASTERRY 3	//电量低

#define AC_BATTERRY				5	//正在充电
#define BATTERRY				6	//电池供电
#define POWER_FULL				7	

#define CLEAR_SYSTEM_PICTURE		0	//清屏目
#define OPEN_SYSTEM_PICTURE			1	//开机动画
#define CLOSE_SYSTEM_PICTURE		2	//关机动画
#define CONNECT_WIFI_OK_PICTURE		3	//wifi-成功连接
#define PLAY_MUSIC_NUM1				4	//播放音乐第1种
#define PLAY_MUSIC_NUM2				5	//播放音乐第2种
#define PLAY_MUSIC_NUM3				6	//播放音乐第3种
#define PLAY_MUSIC_NUM4				7	//播放音乐第4种

#define WAIT_CTRL_NUM1				8	//等待操作第1种
#define WAIT_CTRL_NUM2				9	//等待操作第2种
#define WAIT_CTRL_NUM3				10	//等待播放状态
#define WAIT_CTRL_NUM4				11	//声音停止

#define KEY_CTRL_PICTURE			12	//按键图片

#define SHOW_TLAK_LIGHT				13	//显示嘴灯
#define CLOSE_TLAK_LIGHT			14	//关闭嘴灯
#define PLAY_PAUSE					15	//关闭嘴灯


#define RESET_SYSTEM				20	//

#define SERIAL_SOC_PATH		"/dev/ttyS0"
#define SPEED_SOC			9600
#define CLOCK_TIME			3			//闹钟提前开机时间

//#define DBG_UART
#ifdef DBG_UART 
#define DEBUG_UART(fmt, args...) printf("Uart: " fmt, ## args)  
#else   
#define DEBUG_UART(fmt, args...) { }  
#endif	//end DBG_AP_STA

typedef struct {
	unsigned char head;
	unsigned char data;
	unsigned char cache;
}ReacData;

typedef struct {//当前时间
	unsigned char timehead;
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char min;
	unsigned char chack;
}AckTimeData;

typedef struct {//开关机时间
	unsigned char ochead;
	unsigned char hour;
	unsigned char min;
	unsigned char chack;
}AckOCData;

typedef struct {//开关机时间
	unsigned char ochead;
	unsigned char chack;
}AckOCBat;

typedef struct {//开关机时间
	unsigned char ochead;
	unsigned char type;
	unsigned char chack;
}AckSmok;

typedef struct{
	unsigned char quit;
	unsigned char startSystem;
	unsigned char charge;
	unsigned char battery;
	void (*voicesEvent)(int event);
	void (*Ack_batteryCtr)(int recvdata,int power);
}uart_t;


/*
@ 函数功能:	初始化串口
@ 返回值: >0	初始化成功开始读串口
@			-1	初始化失败
*/
extern int init_Uart(void VoicesEvent(int event),void ack_batteryCtr(int recvdata,int power););
extern void SocSendMenu(unsigned char str,char *senddata);
//串口处理函数
extern int Get_batteryVaule(void);
extern int get_dc_state(void);

extern void __ReSetSystem(void);
#endif
