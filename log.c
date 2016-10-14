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

