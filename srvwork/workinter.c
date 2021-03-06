#include "config.h"

#ifdef WORK_INTER
#include "comshead.h"
#include "base/pool.h"
#include "base/tools.h"
#include "workinter.h"
#include "host/voices/callvoices.h"
#include "../host/voices/gpio_7620.h"
#include "StreamFile.h"

#define IMHELP    \
  "Commands: poolnum(pn)\n"\
  "Commands: lsmem(mem) devlist(show devices list )\n"\
  "Commands: susr(look usrdb) sdev(look devlist ) sroot(look camlist and passwd)\n"\
  "Commands: del_usrtable (delete usr table)\n"\
  "Commands: map(show usr map address)\n"\
  "Commands: del_usr(deltele usr  )\n"\
  "Commands: uplist  initlist  \n"\
  "Commands: help(h)  quit(q)\n"  
void test_ConnetEvent(int event){
	printf("event =%d\n",event);
}
/*
@ 调试命令，编译固件需要去掉
@
*/
void pasreInputCmd(const char *com){
	char *p=NULL;
	if (!strcmp(com, "1")){
		enable_i2s();
		printf("Show_tlak_Light \n");
	}else if(!strcmp(com, "2")){
		Close_tlak_Light();
		printf("Close_tlak_Light \n");
	}else if(!strcmp(com, "3")){

	}else if(!strcmp(com, "4")){
		memcpy(p,"123456",6);
	}else if(!strcmp(com, "en")){
		enable_gpio();
	}else if(!strcmp(com, "q")){
		CleanSystemResources();
	}else if(!strcmp(com, "reset")){
		system("restartNetwork.sh &");
	}else if(!strcmp(com, "sleep")){
		closeSystem();
	}
}
#endif	//end WORK_INTER
