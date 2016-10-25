#include "comshead.h"

int WriteMp3Data(char *filename,char *data,int size)
{
	FILE *fp = fopen(filename,"w+");
	if(NULL == fp ){
		return -1;
    }
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return 0;
}
void playurlLog(const char *data){
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
	return ;
}

void smartConifgLog(const char *data){
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
}
void playsysvoicesLog(const char *data){
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
}
void cleanplayLog(const char *data){
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
}
void eventlockLog(const char *data,unsigned char lock){
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
}
void handleeventLog(const char *data,unsigned char msgSize){
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
}
void tolkLog(const char *data){
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
}


