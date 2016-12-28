#include "comshead.h"
#include "config.h"

int WriteMp3Data(char *filename,char *data,int size){
#ifdef ENABLE_LOG
	FILE *fp = fopen(filename,"w+");
	if(NULL == fp ){
		return -1;
    }
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return 0;
#endif	
}
void playurlLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"url_start\n")){
		fp =fopen("/home/url.log","w+");
	}else{
		fp =fopen("/home/url.log","a+");
	}
	
	if(NULL == fp ){
		return ;
    }
	int size = strlen(data);
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
#endif	
}

void smartConifgLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"smart_start\n")){
		fp =fopen("/home/smartconfig.log","w+");
	}else{
		fp =fopen("/home/smartconfig.log","a+");
	}
	
	if(NULL == fp ){
		return ;
    }
	int size = strlen(data);
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void playsysvoicesLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"playsys_start\n")){
		fp =fopen("/home/playsysvoices.log","w+");
	}else{
		fp =fopen("/home/playsysvoices.log","a+");
	}
	
	if(NULL == fp ){
		return ;
    }
	int size = strlen(data);
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void cleanplayLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"cleanplay_start\n")){
		fp =fopen("/home/cleanplay.log","w+");
	}else{
		fp =fopen("/home/cleanplay.log","a+");
	}
	
	if(NULL == fp ){
		return ;
    }
	int size = strlen(data);
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void eventlockLog(const char *data,int lock){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"eventlock_start\n")){
		fp =fopen("/home/eventlock.log","w+");
	}else{
		fp =fopen("/home/eventlock.log","a+");
	}
	sprintf(buf,"%s%d %s","lock:",lock,data);
	if(NULL == fp ){
		return ;
    }
	int size = strlen(buf);
    fwrite(buf,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void handleeventLog(const char *data,unsigned char msgSize){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"handleevent_start\n")){
		fp =fopen("/home/handleevent.log","w+");
	}else{
		fp =fopen("/home/handleevent.log","a+");
	}
	sprintf(buf,"%s%d %s","lock:",msgSize,data);
	if(NULL == fp ){
		return ;
    }
	int size = strlen(buf);
    fwrite(buf,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void tolkLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"tolk_start\n")){
		fp =fopen("/home/tolk.log","w+");
	}else{
		fp =fopen("/home/tolk.log","a+");
	}
	if(NULL == fp ){
		return ;
    }
	int size = strlen(data);
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void udpLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"udp_start\n")){
		fp =fopen("/home/udp_log.log","w+");
	}else{
		fp =fopen("/home/udp_log.log","a+");
	}
	if(NULL == fp ){
		return ;
    }
	int size = strlen(data);
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void tcpLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"tcp_start\n")){
		fp =fopen("/home/tcp_log.log","w+");
	}else{
		fp =fopen("/home/tcp_log.log","a+");
	}
	if(NULL == fp ){
		return ;
    }
	int size = strlen(data);
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void TimeLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"time_start\n")){
		fp =fopen("/home/time_log.log","w+");
	}else{
		fp =fopen("/home/time_log.log","a+");
	}
	if(NULL == fp ){
		return ;
    }
	int size = strlen(data);
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void JsonLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"json_start\n")){
		fp =fopen("/home/json.log","w+");
	}else{
		fp =fopen("/home/json.log","a+");
	}
	if(NULL == fp ){
		return ;
    }
	int size = strlen(data);
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void tulingLog(const char *data,unsigned char err){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"tuling_start")){
		fp =fopen("/home/tuling.log","w+");
	}else{
		fp =fopen("/home/tuling.log","a+");
	}
	sprintf(buf,"%s%d %s\n","err:",err,data);
	if(NULL == fp ){
		return ;
    }
	int size = strlen(buf);
    fwrite(buf,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}
void musicdbLog(const char *data,const char *filename){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"musicdb_start\n")){
		fp =fopen("/home/musicdb.log","w+");
	}else{
		fp =fopen("/home/musicdb.log","a+");
	}
	sprintf(buf,"%s%s %s\n","name:",data,filename);
	if(NULL == fp ){
		return ;
    }
	int size = strlen(buf);
    fwrite(buf,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
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
}
void GpioLog(const char *data,unsigned char number){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"uart_start\n")){
		fp =fopen("/home/gpio.log","w+");
	}else{
		fp =fopen("/home/gpio.log","a+");
	}
	sprintf(buf,"%s%s %x\n",data,"gpio:",number);
	if(NULL == fp ){
		return ;
    }
	int size = strlen(buf);
    fwrite(buf,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}


