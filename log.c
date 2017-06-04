#include "comshead.h"
#include "config.h"

static void GetDate(char *date){
    //const char* wday[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    //const char* mon[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    time_t timep;
    struct tm* p;
    time(&timep);
    p = gmtime(&timep);
    snprintf(date,128,"%d-%02d-%02d-%02d:%02d:%02d",(1900+p->tm_year),p->tm_mon+1,p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec);
    printf("%s\n", date);
}

void WritePlayUrl_Log(const char *data1,const char *data2){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data1,"start")){
		fp =fopen("/log/playurl.log","w+");
	}else{
		fp =fopen("/log/playurl.log","a+");
	}
	if(NULL == fp ){
		return ;
    }
    fprintf(fp,"%s:%s\n",data1,data2);
  	fflush(fp);
	fclose(fp);
#endif	
}


void PlaySystemAmrVoicesLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"playsys_start\n")){
		fp =fopen("/log/PlaySystemAmrVoices.log","w+");
	}else{
		fp =fopen("/log/PlaySystemAmrVoices.log","a+");
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
		fp =fopen("/log/cleanplay.log","w+");
	}else{
		fp =fopen("/log/cleanplay.log","a+");
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
void WriteEventlockLog(const char *data,int lock){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"eventlock_start\n")){
		fp =fopen("/log/eventlock.log","w+");
	}else{
		fp =fopen("/log/eventlock.log","a+");
	}
	sprintf(buf,"%s%d %s","lock:",lock,data);
	if(NULL == fp ){
		return ;
    }
	int size = strlen(buf);
    fwrite(buf,1,size,fp);
  	fflush(fp);
	fclose(fp);
#endif
}
void handleeventLog(const char *data,unsigned char msgSize){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"handleevent_start\n")){
		fp =fopen("/log/handleevent.log","w+");
	}else{
		fp =fopen("/log/handleevent.log","a+");
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
void PlayQtts_log(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"tolk_start\n")){
		fp =fopen("/log/playQtts.log","w+");
	}else{
		fp =fopen("/log/playQtts.log","a+");
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
		fp =fopen("/log/udp_log.log","w+");
	}else{
		fp =fopen("/log/udp_log.log","a+");
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
void RecvTcp_dataLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	if(!strcmp(data,"tcp_start\n")){
		fp =fopen("/log/tcpdata.log","w+");
	}else{
		fp =fopen("/log/tcpdata.log","a+");
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
void systemTimeLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	fp =fopen("/log/time_log.log","a+");
	if(NULL == fp ){
		return ;
    }
	char date[128]={0};
	GetDate(date);
	fprintf(fp,"%s--->%s\n",date,data);
  	fflush(fp);
	fclose(fp);
	return ;
#endif
}

void RequestTulingLog(const char *data){
#ifdef ENABLE_LOG	
	FILE *fp = NULL;
	char buf[128];
	if(!strcmp(data,"tuling_start")){
		fp =fopen("/log/request_tuling.log","w+");
	}else{
		fp =fopen("/log/request_tuling.log","a+");
	}
	sprintf(buf,"%s\n",data);
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
	fp =fopen("/log/musicdb.log","a+");
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
		fp =fopen("/log/uart.log","w+");
	}else{
		fp =fopen("/log/uart.log","a+");
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
		fp =fopen("/log/gpio.log","w+");
	}else{
		fp =fopen("/log/gpio.log","a+");
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
#ifdef ENABLE_LOG	
	FILE *fp = fopen("/log/localerverVersion.log","w+");
	if(fp){
		fwrite(versionMessage,1,strlen(versionMessage),fp);//写入当前版本
		fclose(fp);
	}	
#endif
}
void Write_Speekkeylog(const char *data,int num){
#ifdef ENABLE_LOG	
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
#endif
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

void Write_tulinglog(const char *logStr){
	writeLog((const char * )"/log/tuling_log.txt",logStr);
}

void WiterSmartConifg_Log(const char *data1,const char *data2){
#ifdef ENABLE_LOG	
	FILE *fplog = NULL;
	if(!strcmp(data2,"start")){
		fplog =fopen("/log/smartconfig.log","w+");
	}else{
		fplog =fopen("/log/smartconfig.log","a+");
	}
	
	if(NULL == fplog ){
		return ;
    }
	if(fplog){
		fprintf(fplog,"%s--->%s\n",data1,data2);
		fclose(fplog);
	}
	return ;
#endif
}

void Write_huashang_log(const char *data1,const char *data2,int val){
#ifdef ENABLE_LOG	
	FILE *fplog = NULL;
	if(!strcmp(data2,"start")){
		fplog =fopen("/log/huashang.log","w+");
	}else{
		fplog =fopen("/log/huashang.log","a+");
	}
	
	if(NULL == fplog ){
		return ;
    }
	if(fplog){
		fprintf(fplog,"%s--->%s--->%d\n",data1,data2,val);
		fclose(fplog);
	}
	return ;
#endif	
}
void Write_tulingTextLog(const char *txt){
#ifdef ENABLE_LOG	
	FILE *fplog = NULL;
	if(!strcmp(txt,"start")){
		fplog =fopen("/log/tulingText.log","w+");
	}else{
		fplog =fopen("/log/tulingText.log","a+");
	}
	
	if(NULL == fplog ){
		return ;
    }
	if(fplog){
		fprintf(fplog,"%s\n",txt);
		fclose(fplog);
	}
	return ;
#endif		
}
void Write_huashangTextLog(const char *text){
#ifdef ENABLE_LOG	
	FILE *fplog = NULL;
	if(!strcmp(text,"start")){
		fplog =fopen("/log/huashang_play.log","w+");
	}else{
		fplog =fopen("/log/huashang_play.log","a+");
	}
	
	if(NULL == fplog ){
		return ;
    }
	if(fplog){
		fprintf(fplog,"%s\n",text);
		fclose(fplog);
	}
	return ;
#endif	
}
void WriteRateTextLog(const char *funtion,const char *text,int rate){
#ifdef ENABLE_LOG	
		FILE *fplog = NULL;
		fplog =fopen("/log/commonRate.log","a+");		
		if(NULL == fplog ){
			return ;
		}
		if(fplog){
			fprintf(fplog,"%s: %s %d\n",funtion,text,rate);
			fclose(fplog);
		}
		return ;
#endif	

}
