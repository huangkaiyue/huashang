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
