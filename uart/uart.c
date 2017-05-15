#include "comshead.h"
#include "base/tools.h"
#include "uart/uart.h"
#include "systools.h"
#include "log.h"
#include "config.h"

static ReacData data;
static int serialFd[2];
static char quit=0;
static uart_t *uartCtr=NULL;

#if 0
/*************************************************
* ��������:�����λ������
* ��  ��  :��Ҫ�������
* ����ֵ  :�ɹ������ֵ
**************************************************/
static unsigned char bin8_rev(unsigned char data) 
{ 
    data=((data&0xf0)>>4) | ((data&0x0f)<<4); 
    data=((data&0xCC)>>2) | ((data&0x33)<<2); 
    data=((data&0xAA)>>1) | ((data&0x55)<<1); 
    return data; 
}
#endif
/*************************************************
* ��������:��ȡ��ص���
* ��  ��  :��
* ����ֵ  :��ص���
**************************************************/
int Get_batteryVaule(void){
	switch(uartCtr->battery){
		case 0:
			return 10;
		case 1:
			return 25;
		case 2:
			return 50;
		case 3:
			return 75;
		case 4:
		case 5:
			return 100;
		default:
			return 50;
	}
}
/*************************************************
* ��������:��ȡ���״̬
* ��  ��  :��
* ����ֵ  :0 δ��� 1���
**************************************************/
int get_dc_state(void){
	if(uartCtr->charge!=1){
		uartCtr->charge=0;
	}
	return uartCtr->charge;
}
/*************************************************
* ��������:���ʹ�����Ϣ�˵�
* ��  ��  :size ��Ϣ��С senddata ������Ϣ
* ����ֵ  :��
**************************************************/
static int uart_send_soc(unsigned char senddata[],int size){
	if(serialFd[0]>0){
		int wSize=0,ret;
		while(wSize<size){
			ret = write(serialFd[0],senddata+wSize,1);
			usleep(1);
			wSize+=ret;
		}
	}
	return 0;
}
/*************************************************
* ��������:��ȡ����ʱ��
* ��  ��  :timedata ��ȡ����ĵ�ַ
* ����ֵ  :��
**************************************************/
static int GetLocalTime(AckTimeData *timedata){
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep); /*ȡ�õ���ʱ��*/
	timedata->timehead=MSTIME;
	if((p->tm_year)<100)
		return -1;
	timedata->year=((1900+p->tm_year)-2000);
	timedata->month=(( 1+p->tm_mon));
	timedata->day=(p->tm_mday);
	timedata->hour=(p->tm_hour)+8;
	if(timedata->hour >= 24){
		timedata->hour -= 24;
		timedata->day += 1;
	}
	timedata->min=(p->tm_min);
	timedata->chack=timedata->year+timedata->month+timedata->day+timedata->hour+timedata->min+MSTIME;
	DEBUG_UART ("%d/%d/%d \n", (timedata->year),( timedata->month), timedata->day);
	DEBUG_UART("%d:%d\n",timedata->hour, timedata->min);//, p->tm_sec
	return 0;
}
/*************************************************
* ��������:���ʹ�����Ϣ�˵�
* ��  ��  :str ���� senddata ������Ϣ
* ����ֵ  :��
**************************************************/
void SocSendMenu(unsigned char str,char *senddata){
	char *p;
	char *pt;
	AckSmok smok;
	AckOCBat bat;
	AckOCData data;
	AckTimeData timedata;
	unsigned char facebuf[3];
	char checksum;
	switch(str){
		case OPEN://��ʱ����
			data.ochead=MSOPEN;
			pt=strtok_r(senddata,":",&p);
			data.hour=atoi(pt);
			data.min=atoi(p);
			data.chack=data.hour+data.min+data.ochead;
			DEBUG_UART("system open time is %d:%d\n",data.hour,data.min);
			uart_send_soc((unsigned char *)&data,sizeof(AckOCData));
			break;
		case CLOCK:
			data.ochead=MSCLOCK;
			pt=strtok_r(senddata,":",&p);
			data.hour=atoi(pt);
			if(atoi(p)<CLOCK_TIME){
				if(data.hour==0){
					data.hour=24;
				}
				data.hour -= 1;
				data.min=(atoi(p)+60)-CLOCK_TIME;
			}else{
				data.min=atoi(p)-CLOCK_TIME;
			}
			data.chack=data.hour+data.min+data.ochead;
			DEBUG_UART("system open time is %d:%d\n",data.hour,data.min);
			uart_send_soc((unsigned char *)&data,sizeof(AckOCData));
			break;
		case MUC_CLOSE_SYSTEM://��ʱ�ػ�
			data.ochead=MSCLOSE;
			pt=strtok_r(senddata,":",&p);
			data.hour=atoi(pt);
			data.min=atoi(p);
			data.chack=data.hour+data.min+data.ochead;
			DEBUG_UART("syetem close time is %d:%d\n",data.hour,data.min);
			uart_send_soc((unsigned char *)&data,sizeof(AckOCData));
			break;
		case TIME://��ǰʱ��
			if(GetLocalTime(&timedata)==0){
				uart_send_soc((unsigned char *)&timedata,sizeof(AckTimeData));
			}
			break;
		case BATTYPE://������ȡ
			bat.ochead=MSBATTERY;
			bat.chack=MSBATTERY;
			uart_send_soc((unsigned char *)&bat,sizeof(AckOCBat));
			break;
		case SMOKER_OK://�����ź�
			smok.ochead=SMOK;
			smok.type=0x55;
			smok.chack=SMOK+0x55;
			uart_send_soc((unsigned char *)&smok,sizeof(AckSmok));
			break;
		case SMOKER_ER://�����ź�
			smok.ochead=SMOK;
			smok.type=0xaa;
			smok.chack=SMOK+0xaa;
			uart_send_soc((unsigned char *)&smok,sizeof(AckSmok));
			break;
		case FACE:
			checksum = FACECMD + *senddata;
			memset(facebuf,0,sizeof(facebuf));
			facebuf[0] = FACECMD;
			facebuf[1] = *senddata;
			facebuf[2] = checksum;
			uart_send_soc(facebuf,sizeof(facebuf));
			break;
	}
}
void showFacePicture(unsigned char faceNums){
#if defined(HUASHANG_JIAOYU)	
	unsigned char faceBuf =faceNums;
	SocSendMenu(FACE,(char *)&faceNums);
#endif	
}
/*************************************************
* ��������:������Ϣ������
* ��  ��  :��
* ����ֵ  :��
**************************************************/
static unsigned char batterylow=0;	//�����͵��־
static int CacheUarl(void){
	//printf("===%x+%x=%x===================\n",data.head,data.data,data.cache);
	if(((data.head+data.data)&0x0ff)==data.cache){
		return 0;
	}
	else{
		return -1;
	}
}
static void handle_uartMsg(int fd ,unsigned char buf,int size){
	if(data.head==0x0){
		data.head=buf;
		DEBUG_UART("\n===head ===%x==\n",data.head);
		switch(data.head){
			case SMCLOSETIME://MCU��ʱ��ʱ���ػ��źŷ���
			case SMBATTERY://��ص��� 100��75��50��25��10
			case SMCLOSE://���ػ�
			case SMBATTYPE://���״̬
			case SMOK://�����ź�
				break;
			default:
				data.head=0x0;
				break;
		}
	}
	else if(data.data==0x0){
		data.data=buf;
		DEBUG_UART("===data ===%x==\n",data.data);
		switch(data.head){
			case SMCLOSE://���ػ�
			case SMCLOSETIME://MCU��ʱ��ʱ���ػ��źŷ���
			case SMBATTERY://��ص��� 100��75��50��25��10
			case SMBATTYPE://���״̬
			case SMOK://�����ź�
				break;
			default:
				data.head=0x0;
				data.data=0x0;
				break;
		}
	}else{
		data.cache=buf;
		DEBUG_UART("===cache ===%x==\n",data.cache);
		switch(data.head){
			case SMCLOSE://���ػ�
			case SMCLOSETIME://MCU��ʱ��ʱ���ػ��źŷ���
				DEBUG_UART("handle_uartMsg SMCLOSETIME \n");
				if(CacheUarl()==0){
					uartCtr->voicesEvent(UART_EVENT_CLOSE_SYSTEM);
					SocSendMenu(SMOKER_OK,0);
				}else{
					SocSendMenu(SMOKER_ER,0);
				}
				break;
				
			case SMBATTERY://��ص��� 100��75��50��25��10
			case SMBATTYPE://���״̬
				if((data.data&0x80)==0x80){//���
					DEBUG_UART("handle_uartMsg	SMBATTYPE OK \n");
					uartCtr->charge=1;
					batterylow=1;
				}
				else if((data.data&0x80)==0x00){//δ���
					DEBUG_UART("handle_uartMsg	SMBATTYPE ERROR \n");
					uartCtr->charge=0;
				}
				DEBUG_UART("SMBATTYPE bat=%d\n",data.data);
				if(CacheUarl()==0){
					data.data&=0xf;
					SocSendMenu(SMOKER_OK,0);
					uartCtr->battery=data.data;
					uartCtr->Ack_batteryCtr(Get_batteryVaule(),uartCtr->charge);
					if(uartCtr->battery<=1&&batterylow==0){		//��������25��������
						uartCtr->voicesEvent(UART_EVENT_LOW_BASTERRY);
						batterylow=1;
					}
				}else{
					SocSendMenu(SMOKER_ER,0);
				}
				break;
				
			case SMOK://�����ź�
				if(data.data==0x55){//��ȷ
					DEBUG_UART("handle_uartMsg OK \n");
				}
				else if(data.data==0xaa){//����
					DEBUG_UART("handle_uartMsg error \n");
				}
				break;

		}
		data.data=0x0;
		data.head=0x0;
		data.cache=0x0;
	}
}
/*************************************************
* ��������:��ȡ������Ϣ
* ��  ��  :��
* ����ֵ  :��
**************************************************/
static void *uart_read_serial(void){	
	struct timeval timeout = {2, 0};
	fd_set rdfd;
	int ret =0,r_size=1;
	unsigned char buf=0x0;
	data.head=0x0;
	while(quit){
		FD_ZERO(&rdfd);
		FD_SET(serialFd[0],&rdfd);
		ret = select(serialFd[0] + 1,&rdfd, NULL,NULL,&timeout);
		switch(ret){
			case -1:
				break;
			case 0:
				usleep(300*1000);//sleep 1s
				break;
			default: 
				if (FD_ISSET(serialFd[0], &rdfd)){
					UartLog("start<---------->",11111111111);
					DEBUG_UART("start<---------->\n");
					for(;r_size>0;){
						r_size = read(serialFd[0],&buf,1);
						if(r_size!=0){
							DEBUG_UART("recv from uart msg buf =0x%x\n",buf);
							UartLog("reav",buf);
							handle_uartMsg(serialFd[0],buf,r_size);
						}
					}
					r_size=1;
					buf=0x0;
					UartLog("end<---------->",11111111111);
					DEBUG_UART("end<---------->\n");
				}
		}
	}
	close(serialFd[0]);
	return NULL;
}
/*************************************************
* ��������:���ڳ�ʼ��
* ��  ��  :VoicesEvent ϵͳ�� �ص�����
			ack_batteryCtr �������� �ص�����
* ����ֵ  :0 �ɹ� -1ʧ��
**************************************************/
int init_Uart(void VoicesEvent(int event),void ack_batteryCtr(int recvdata,int power)){
	quit=1;
	uartCtr = (uart_t *)calloc(1,sizeof(uart_t));
	if(uartCtr==NULL)
		return -1;
	uartCtr->voicesEvent=VoicesEvent;
	uartCtr->Ack_batteryCtr=ack_batteryCtr;
	serialFd[0] = serial_open(SERIAL_SOC_PATH,SPEED_SOC);
	if(pthread_create_attr(uart_read_serial,NULL)){
		perror("pthread_create uart_read_serial failed");
		return -1;
	}
	usleep(10000);
	SocSendMenu(3,NULL);

	usleep(10000);
	SocSendMenu(4,NULL);
	return 0;
}

//#define TEST
#ifdef TEST
/*************************************************
* ��������:��ӡ��ȡ��ǰʱ��
* ��  ��  :��
* ����ֵ  :��
**************************************************/
void get_time(void){
	time_t timep;
	time(&timep);
	printf("%s",asctime(gmtime(&timep)));
}
void UartLog(const char *data,unsigned char number){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"uart_start\n")){
		fp =fopen("/home/uart.log","w+");
	}else{
		fp =fopen("/home/uart.log","a+");
	}
	sprintf(buf,"%s%s %x\n",data,"0x:",number);
	if(NULL == fp ){
		return ;
    }
	int size = strlen(buf);
    fwrite(buf,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
};

void VoicesEvent(int event){
	printf("VoicesEvent event = %d ..\n",event);
}
void ack_batteryCtr(int recvdata,int power){
	printf("recvdata = %d,power = %d\n",recvdata,power);
}
int main(int argc,char *argv[]){
	char bufo[12]="0:30";
	char bufc[12]="0:27";
	char bufd[12]="17:20";
	init_Uart(VoicesEvent,ack_batteryCtr);
	sleep(1);
	char cmd[20];
	while(1){
		memset(cmd,0,20);
		fgets(cmd,20,stdin);
		if(!strncmp(cmd,"1",1)){
				printf("cmd ----->1--%s---\n",bufo);
				SocSendMenu(1,bufo);
		}
		else if(!strncmp(cmd,"2",1)){
				printf("cmd ----->2--%s---\n",bufc);
				SocSendMenu(2,bufc);
		}
		else if(!strncmp(cmd,"3",1)){
				printf("cmd ----->3-----\n");
				SocSendMenu(3,0);
		}
		else if(!strncmp(cmd,"4",1)){
				printf("argv ----->4-----\n");
				SocSendMenu(4,0);
		}
		else if(!strncmp(cmd,"7",1)){
				printf("cmd ----->7-----\n");
				SocSendMenu(7,bufd);
		}
	}
	return 0;
}
#endif
