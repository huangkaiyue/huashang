#ifndef	UART_H_
#define UART_H_


#define MSTIME		0x16	//当前时间 0001 0110 ->0b01101000
#define MSOPEN		0x23	//定时开 0010 0011 ->0b01000100
#define MSCLOSE		0x33	//定时关 0011 0011 ->0b01001100
#define MSBATTERY	0x81	//请求电量

#define SMCLOSETIME	0x42	//MCU定时定时到关机信号发送 0100 0010 ->0b01000010
#define SMBATTERY	0x52	//电池电量 0101 0010 ->0b01001010
#define SMCLOSE		0x62	//开关机提示 0110 0010 ->0b01000110
#define SMBATTYPE	0x72	//充电状态 0111 0010 ->0b01001110
#define SMOK		0xf2	//握手信号 1111 0010 ->0b01001111
#define SMLOW		0x91	//低电提醒


#define OPEN		1
#define CLOSE		2
#define TIME		3
#define BATTYPE		4
#define SMOKER		5

#define SERIAL_SOC_PATH		"/dev/ttyS0"
#define SPEED_SOC			9600

//#define DBG_UART
#ifdef DBG_UART 
#define DEBUG_UART(fmt, args...) printf("Uart: " fmt, ## args)  
#else   
#define DEBUG_UART(fmt, args...) { }  
#endif	//end DBG_AP_STA


typedef struct {
	unsigned char head;
	unsigned char data;
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
extern int get_battery(void);
extern int get_charge(void);

#endif
