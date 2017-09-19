#include "comshead.h"
#include "config.h"

static void GetDate(char *date){
    time_t timep;
    struct tm* p;
    time(&timep);
    p = gmtime(&timep);
    snprintf(date,128,"%d-%02d-%02d-%02d:%02d:%02d",(1900+p->tm_year),p->tm_mon+1,p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec);
    printf("%s\n", date);
}
static void __WriteLog_File(const char *file,const char *str1,const char *str2,int val){
#ifdef ENABLE_LOG
	FILE *fp = NULL;
	if(!strcmp(str1,"start")){
		fp =fopen(file,"w+");
	}else{
		fp =fopen(file,"a+");
	}
	if(NULL == fp ){
		return ;
    	}
	char dateBuf[128]={0};
	GetDate(dateBuf);
	fprintf(fp,"%s:%s--->%s--->%d\n",dateBuf,str1,str2,val);
	fflush(fp);
	fclose(fp);
#endif
}
void WritePlayUrl_Log(const char *data1,const char *data2){
	__WriteLog_File("/log/playurl.log",data1,data2,0);
}
void cleanplayLog(const char *data){
	__WriteLog_File("/log/cleanplay.log",data,"",0);
}
void WriteEventlockLog(const char *data,int lock){
	__WriteLog_File("/log/eventlock.log",data,"",lock);
}
void udpLog(const char *data){
	__WriteLog_File("/log/udp_log.log",data,"",0);
}
void RecvTcp_dataLog(const char *data){
	__WriteLog_File("/log/tcpdata.log",data,"",0);
}
void System_StateLog(const char *data){
	__WriteLog_File("/log/system_state.log",data,"",0);
}

void UartLog(const char *data,unsigned char number){
	__WriteLog_File("/log/uart.log",data,"",number);
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


void SpeekEvent_process_log(const char *str1,const char *str2,int value){
	__WriteLog_File("/log/SpeekEvent_process_log.log",str1,str2,value);
}

void Write_tulinglog(const char *logStr){
	writeLog((const char * )"/log/tuling_log.txt",logStr);
}
void Write_StartLog(const char *logStr,int timeout){
	__WriteLog_File((const char * )"/log/start_log.txt",logStr,"",timeout);
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
void WriteRateTextLog(const char *funtion,const char *text,int rate,int newRate){
#ifdef ENABLE_LOG	
		FILE *fplog = NULL;
		fplog =fopen("/log/commonRate.log","a+");		
		if(NULL == fplog ){
			return ;
		}
		if(fplog){
			fprintf(fplog,"%s: %s current=%d new=%d\n",funtion,text,rate,newRate);
			fclose(fplog);
		}
		return ;
#endif	

}
void Write_playAmrFile(const char *buf){
	writeLog((const char * )"/log/playamrfile.txt",buf);
}
void WriteVersionMessage(const char *str1,const char *str2,int val){
	__WriteLog_File((const char *)"/log/verion_weixin.txt",str1,str2,val);
}
