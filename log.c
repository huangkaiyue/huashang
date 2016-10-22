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
void urlLogStart(char *data,int size)
{
	FILE *fp = fopen("/home/url.log","w+");
	if(NULL == fp ){
		return ;
    }
    fwrite(data,1,size,fp);
  	fflush(fp);
	fclose(fp);
	return ;
}

void urlLogEnd(char *data,int size)
{
	FILE *fp = fopen("/home/url.log","a+");
	if(NULL == fp ){
		return ;
    }
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
