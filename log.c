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
void WritePlayUrl_Log(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"url_start\n")){
		fp =fopen("/log/url.log","w+");
	}else{
		fp =fopen("/log/url.log","a+");
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

void WiterSmartConifg_Log(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"smart_start\n")){
		fp =fopen("/log/smartconfig.log","w+");
	}else{
		fp =fopen("/log/smartconfig.log","a+");
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
void PlaySystemAmrVoicesLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"playsys_start\n")){
		fp =fopen("/home/PlaySystemAmrVoices.log","w+");
	}else{
		fp =fopen("/home/PlaySystemAmrVoices.log","a+");
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
void RequestTulingLog(const char *data,unsigned char err){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"tuling_start")){
		fp =fopen("/home/request_tuling.log","w+");
	}else{
		fp =fopen("/home/request_tuling.log","a+");
	}
	sprintf(buf,"err:%d %s\n",err,data);
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
	sprintf(buf,"%s: 0x%x\n",data,number);
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
void WriteLocalserver_Version(const char *versionMessage){
	FILE *fp = fopen("/log/localerverVersion.log","w+");
	if(fp){
		fwrite(versionMessage,1,strlen(versionMessage),fp);//写入当前版本
		fclose(fp);
	}	
}
void Write_Speekkeylog(const char *data,int num){
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"speekstart")){
		fp = fopen("/log/Speekkeylog.log","w+");
	}else{
		fp = fopen("/log/Speekkeylog.log","a+");
	}
	if(NULL == fp ){
		return ;
    }
	snprintf(buf,128,"%s %d\n",data,num);
	int size = strlen(buf);
    fwrite(buf,1,size,fp);
  	fflush(fp);
	fclose(fp);
}


void test_Save_VoicesPackt_function_log(const char *data,int value){
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"start")){
		fp = fopen("/log/Save_VoicesPackt_function.log","w+");
	}else{
		fp = fopen("/log/Save_VoicesPackt_function.log","a+");
	}
	if(NULL == fp ){
		return ;
    }
	snprintf(buf,128,"%s %d\n",data,value);
	int size = strlen(buf);
    fwrite(buf,1,size,fp);
  	fflush(fp);
	fclose(fp);
}