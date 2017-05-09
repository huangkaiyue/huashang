#ifndef	UART_H_
#define UART_H_


#define MSTIME		0x16	//��ǰʱ�� 0001 0110 ->0b01101000
#define MSOPEN		0x23	//��ʱ�� 0010 0011 ->0b01000100
#define MSCLOSE		0x33	//��ʱ�� 0011 0011 ->0b01001100
#define MSBATTERY	0x81	//�������
#define MSCLOCK		0xa3	//���ӿ��� 1010 0011

#define SMCLOSETIME	0x42	//MCU��ʱ��ʱ���ػ��źŷ��� 0100 0010 ->0b01000010
#define SMBATTERY	0x52	//��ص��� 0101 0010 ->0b01001010
#define SMCLOSE		0x62	//���ػ���ʾ 0110 0010 ->0b01000110
#define SMBATTYPE	0x72	//���״̬ 0111 0010 ->0b01001110
#define SMOK		0xf2	//�����ź� 1111 0010 ->0b01001111
#define SMLOW		0x91	//�͵�����

#define FACECMD		0x02

#define OPEN				1
#define MUC_CLOSE_SYSTEM	2
#define TIME				3
#define BATTYPE				4
#define SMOKER_OK			5
#define SMOKER_ER			6	
#define CLOCK				7
#define FACE				8

#define UART_EVENT_CLOSE_SYSTEM	1	//mcu�ػ��¼�
#define UART_EVENT_LOW_BASTERRY 3	//������


#define OPEN_SYSTEM_PICTURE			1	//��������
#define CLOSE_SYSTEM_PICTURE		2	//�ػ�����
#define CONNECT_WIFI_ING_PICTURE	3	//wifi-���ӹ��̶���
#define CONNECT_WIFI_OK_PICTURE		4	//wifi-�ɹ�����
#define	BATTERY_PICTURE				5	//��ص���
#define	WEIXIN_PICTURE				6	//��Ϣ
#define MUSIC_SHAPE_PICTURE			7	//����-���ζ���
#define MUSIC_HZ_PICTURE			8	//����-Ƶ�׶���

#define FACE_jingya_42				42	//��̬����-����
#define FACE_qinqin_51				51	//��̬����-����
#define FACE_feel_happy_54			54	//��̬����-΢Ц
#define FACE_thumbs_up_63			63	//��̬����-��
#define FACE_eye_show_64			64	//��̬����-գ��

#define SERIAL_SOC_PATH		"/dev/ttyS0"
#define SPEED_SOC			9600
#define CLOCK_TIME			3			//������ǰ����ʱ��

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

typedef struct {//��ǰʱ��
	unsigned char timehead;
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char min;
	unsigned char chack;
}AckTimeData;

typedef struct {//���ػ�ʱ��
	unsigned char ochead;
	unsigned char hour;
	unsigned char min;
	unsigned char chack;
}AckOCData;

typedef struct {//���ػ�ʱ��
	unsigned char ochead;
	unsigned char chack;
}AckOCBat;

typedef struct {//���ػ�ʱ��
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
@ ��������:	��ʼ������
@ ����ֵ: >0	��ʼ���ɹ���ʼ������
@			-1	��ʼ��ʧ��
*/
extern int init_Uart(void VoicesEvent(int event),void ack_batteryCtr(int recvdata,int power););
extern void SocSendMenu(unsigned char str,char *senddata);
//���ڴ�����
extern int Get_batteryVaule(void);
extern int get_dc_state(void);

#endif
