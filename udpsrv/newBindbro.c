#include "comshead.h"
#include "config.h"
#include "base/cJSON.h"

#ifdef SELECT_UDP
int SendTo(int sockfd,char *data,int size,struct sockaddr_in *peer)
{
	char *cachedata = (char *)calloc(1,size+16);
	if(cachedata==NULL){
		perror("calloc error !!!");
		return;
	}
	snprintf(cachedata,16,"%s%d%s","head:",size,":");
	memcpy(cachedata+16,data,size);
	sendto(sockfd, (char *)cachedata, size+16, 0,(struct sockaddr*)peer, sizeof(struct sockaddr));
	free(cachedata);
	return 0;
}
void recv_brocastCtr(int sockfd,struct sockaddr_in *peer,char *recvdata)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	char IP[20]={0};
	GetNetworkcardIp("apcli0",IP);
	//GetNetworkcardIp("eth2.2",IP);
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "brocast");
	cJSON_AddStringToObject(pItem, "ip",IP); 
	cJSON_AddNumberToObject(pItem, "port", 20000);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	printf("recv_brocastCtr :\n%s(%d)\n",szJSON,strlen(szJSON));
	int i;
	for(i=0;i<4;i++){
		SendTo(sockfd, (char *)szJSON, strlen(szJSON),peer);
		usleep(10*1000);
	}
	cJSON_Delete(pItem);
}

void test_brocastCtr(int sockfd,struct sockaddr_in *peer,char *recvdata)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	char IP[20]={0};
	GetNetworkcardIp("apcli0",IP);
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "TestNet");
	cJSON_AddStringToObject(pItem, "ip",IP); 
	cJSON_AddNumberToObject(pItem, "port", 20000);
	if(!strcmp(IP,""))
	{
		cJSON_AddStringToObject(pItem, "status","failed");
	}else{
		cJSON_AddStringToObject(pItem, "status","ok");
	}
	szJSON = cJSON_Print(pItem);
	printf("test_brocastCtr :\n%s(%d)\n",szJSON,strlen(szJSON));
	int i;
	for(i=0;i<2;i++){
		SendTo(sockfd, (char *)szJSON, strlen(szJSON),peer);
		usleep(10*1000);
		}
	cJSON_Delete(pItem);
}

#else
#include "broadcast.h"
#include "base/pool.h"

static short broSock=0; 
static unsigned char broLive=0;

static int SendBroMsg(char *msg,int size,struct sockaddr_in *peer)
{
	return sendto(broSock, (char *)msg, size, 0,(struct sockaddr*)peer, sizeof(struct sockaddr));
}
/*
@函数功能:	广播返回函数
@参数:	recvdata 广播ip
*/
void recv_brocastCtr(int sockfd,struct sockaddr_in *peer,char *recvdata)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	char IP[20];
	memset(IP,0,20);
	int i;
	GetNetworkcardIp("apcli0",IP);
	
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "brocast");
	cJSON_AddStringToObject(pItem, "ip",IP); 
	cJSON_AddNumberToObject(pItem, "port", 20000);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	printf("recv_brocastCtr :\n%s(%d)\n",szJSON,strlen(szJSON));
	for(i=0;i<3;i++)
		SendBroMsg(szJSON,strlen(szJSON),peer);
	cJSON_Delete(pItem);
}

void handler_brocastMsg(int sockfd,struct sockaddr_in *peer,char *recvdata)
{
	cJSON * pJson = cJSON_Parse(recvdata);
	if(NULL == pJson)
	{
		return -1;
	}
	printf("handler_brocastMsg :recvdata=%s\n",recvdata);
	cJSON * pSub = cJSON_GetObjectItem(pJson, "handler");
	if(NULL == pSub)
	{
		printf("get json data  failed\n");
		goto exit;
	}
	printf("handler : %s\n", pSub->valuestring);
	if(!strcmp(pSub->valuestring,"brocast"))
	{
		// get number from json
		pSub = cJSON_GetObjectItem(pJson, "ip");
		if(NULL == pSub)
		{
			printf("get vol failed\n");
			goto exit;
		}
		if(!strcmp(pSub->valuestring,"null"))
		{
			recv_brocastCtr(broSock,peer,recvdata);
		}
	}else if(!strcmp(pSub->valuestring,"updateHost")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"newversion")){//有新版本，需要更新

		}else if(!strcmp(pSub->valuestring,"start")){	//正在下载镜像
		
		}else if(!strcmp(pSub->valuestring,"error")){	//下载错误

		}else if(!strcmp(pSub->valuestring,"end")){		//下载结束
		}
	}else if(!strcmp(pSub->valuestring,"updateImage")){
		if(!strcmp(pSub->valuestring,"start")){			//开始更新image

		}else if(!strcmp(pSub->valuestring,"error")){	//更新image错误

		}else if(!strcmp(pSub->valuestring,"end")){		//更新image结束
		
		}
	}
exit:
	cJSON_Delete(pJson);
	return 0;
}
/*******************************************************
函数功能:app运行发送过来绑定信息同一个网络下，绑定主机操作，
只能有一台设备请求服务器绑定
参数:  
返回值:无
********************************************************/
static void* recvfrom_brocastmsg(void *arg)
{
  	struct sockaddr_in peer;
  	int size=0,len = sizeof(struct sockaddr);
	char recvbuf[256];
	while(broLive)
  	{	
    	if((size = recvfrom(broSock, recvbuf,256,0,(struct sockaddr*)&peer,(socklen_t *)&len))<=0)
    	{
			usleep(300000);
      		continue;
    	}
#if 0
		printf("recvbuf = %s\n",recvbuf);
		FILE *log = fopen("log.txt","a+");
		if(log!=NULL){
			fwrite(recvbuf,size,1,log);
			fclose(log);
		}
#endif
		handler_brocastMsg(broSock,&peer,recvbuf);
	}
	broLive=2;
	memset(recvbuf,0,256);
	return NULL;
 }

void init_broadcast(int port)
{
	broSock= create_listen_brocast(NULL,port);
	broLive =1;
	if(pthread_create_attr(recvfrom_brocastmsg,NULL))
	{
		perror("create udp bro failed!");
		return ;
	}
}
void clean_broadcast(void)
{
	int timeout=0;
	broLive=0;
	while(broLive==0){
		if(++timeout>40||broLive==2){
			break;
		}
		usleep(100000);
	}
	close(broSock);
	usleep(3000);
}
#endif

