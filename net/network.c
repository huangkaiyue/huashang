#include "comshead.h"
#include "base/demo_tcp.h"
#include "network.h"
#include "sysdata.h"
#include "base/udp_sock.h"
#include "../uart/uart.h"
#include "base/pool.h"

#ifdef 	TCP_LOG	
#include "systools.h"
#endif

//#define SERVER_LOG
#ifdef SERVER_LOG
#define ServerLog(fmt, args...) printf("ServerLog: " fmt, ## args)
#else
#define ServerLog(fmt, args...) { }
#endif

Server *ctrSer=NULL;
static void  __close_server(Server *ser);

static Server * Server_Alloc(void)
{
	Server *ser = calloc(sizeof(Server),1);
	if(!ser)
	{
		ServerLog("calloc Server failed \n");
		return NULL;
	}
	return ser;
}
static void Server_Free(Server *ser)
{
	int timeout=0;
	if(ser){
		ser->quit =0;
		close(ser->listenfd);
		ser->listenfd=-1;
		while(ser->quit==0){
			if(++timeout>40||ser->quit==2){
				break;
			}
			usleep(100000);
		}
		__close_server(ser);
  		free(ser);
	}
	ser=NULL;
}
#if 0
void showclient(Server *ser)  
{  
    int i;  
    ServerLog("client amount: %d\n", ser->conn_amount);  
    for (i = 0; i < BACKLOG; i++) {  
        ServerLog("[%d]:%d  ", i, ser->fd_A[i]);  
    }  
    ServerLog("\n\n");  
} 
#endif
static int initSock(Server *ser,int port)
{
	int sock =create_server(NULL,port);
	if(sock==-1)
	{
		return -1;
	}
	ser->listenfd = sock;
	ser->quit =1;
	return 0;
}

static void delete_socket(Server *ser,int sockfd)
{
	int i=0;
	for (i = 0; i < ser->conn_amount; i++) 
	{  									
		if(ser->fd_A[i]==sockfd)
		{
			// client close  
			ServerLog("client[%d] close\n", i);
			ser->conn_amount--;
			close(ser->fd_A[i]);
			FD_CLR(ser->fd_A[i], &ser->fdsr);
			ser->fd_A[i] = 0;
		}
	} 
}
/*
@  回应控制消息
@  sockfd 回应请求描述符 data 回应数据 size 数据包大小
@  无
*/
static int __send_ctrl_ack(Server *ser,int sockfd,char *data,int size)
{
	int ret=0;
	char *cachedata = (char *)calloc(1,size+16);
	snprintf(cachedata,16,"%s%d%s","head:",size,":");
	memcpy(cachedata+16,data,size);
	ServerLog("-------------------------__send_ctrl_ack(%d)-------------------------\n",sockfd);
	ret = send(sockfd,cachedata,size+16,0);
	free(cachedata);
	ServerLog("--------------------------\n%s\n----------------------------\n",data);
	if(ret==0)
	{
		perror("sockfd is close ");
		delete_socket(ser,sockfd);
		return -1;
	}else if(ret==-1)
	{
		perror("send ctrl ack failed");
		return -1;
	}
	return 0;
}

int send_ctrl_ack(int sockfd,char *data,int size)
{	
	return __send_ctrl_ack(ctrSer,sockfd,data,size);
}

static int __sendAll_Ack(Server *ser,char *data,int size)
{
	int i;
	for (i = 0; i < ser->conn_amount; i++) 
	{
		if(ser->fd_A[i]>0)
			__send_ctrl_ack(ser,ser->fd_A[i],data,size);
	}
	return 0;
}

int sendAll_Ack(char *data,int size)
{
	return __sendAll_Ack(ctrSer,data,size);
}
static int SendUdp_Ack(Server *ser,struct sockaddr_in *addr,char *data,int size)
{
	int ret=0;
	char *cachedata = (char *)calloc(1,size+16);
	snprintf(cachedata,16,"%s%d%s","head:",size,":");
	memcpy(cachedata+16,data,size);
	ServerLog("----------------------------SendUdp_Ack(%s)--------------------------\n",inet_ntoa(addr->sin_addr));
	ret = sendto(ser->broSock,cachedata,size+16,0,addr,sizeof(struct sockaddr));
	free(cachedata);
	ServerLog("%s\n---------------------------------------------------------\n",data);
	return ret;
}
int UdpAll_Ack(char *data,int size)
{
	int i;
	for (i = 0; i < ctrSer->conn_amount; i++) 
	{
		if(ctrSer->fd_A[i]>0)
			SendUdp_Ack(ctrSer,&(ctrSer->addr[i]),data,size);
	}
	return 0;
}
//设置非延时发包
static int SetTcpNoDelay(int sockfd) {
    int yes = 1;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1) {
    	ServerLog("setsockopt(TCP_NODELAY) failed");
        return -1;
    }
    return 0;
}
/***********************************************************
@  将连接上来的客户端添加到队列当中
@  new_fd 新连接上来的fd client_addr 客户端的地址
@  无
***********************************************************/
static void add_queue(Server *ser,int new_fd,struct sockaddr_in client_addr)
{
	int i=0;
//	int vol;
//	int Flags;
	if (ser->conn_amount < BACKLOG){	
	 	for(i=0;i<BACKLOG;i++){
			if(ser->fd_A[i]==0)
				break;
		}
		int flags = fcntl(new_fd, F_GETFL, 0);
		fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);
		SetTcpNoDelay(new_fd);
		ser->fd_A[i] = new_fd;
		init_addr(&(ser->addr[i]),inet_ntoa(client_addr.sin_addr),CTRL_PORT);
		ServerLog("new connection addr[%d] %s : %d\n", ser->conn_amount,  
				 inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));  
		ack_alluserCtr(new_fd,get_battery(),get_charge());
		if (new_fd > ser->maxsock){
			ser->maxsock = new_fd;
		}
		ser->conn_amount++;
	 }else{	
		 ServerLog("max connections arrive, exit\n");
		 send(new_fd, "bye", 4, 0);
		 close(new_fd);  
	 }	
}
/***********************************************************
@函数功能: 处理接收到的UDP和TCP数据
@
@ 
***********************************************************/
static void recv_ctrlMsg(Server *ser,char recvbuf[])
{
	int  i=0, ret=0;
#ifdef SELECT_UDP
	struct sockaddr_in peer;
	int len = sizeof(struct sockaddr);
	if (FD_ISSET(ser->broSock, &ser->fdsr)){ 
		if((ret = recvfrom(ser->broSock, recvbuf,512,0,(struct sockaddr*)&peer,(socklen_t *)&len))>0)
		{
#ifdef 	UDP_LOG	
			writeLog("/home/udp_log.txt",recvbuf);
#endif
			handler_CtrlMsg(ser->broSock,recvbuf,ret,&peer);
		}
	}
#endif
	// check every fd in the set  
	for (i = 0; i < ser->conn_amount; i++){
		if (FD_ISSET(ser->fd_A[i], &ser->fdsr)){
			while(1){
				ret = recv(ser->fd_A[i], recvbuf, 512, 0);
				if (ret == 0){	// client close  
					ServerLog("client[%d] close\n", i);
					ser->conn_amount--;
					FD_CLR(ser->fd_A[i], &ser->fdsr);
					ser->fd_A[i] = 0;  
				}else if(ret == -1){
					ServerLog("recv error");
					break;
				}else{
					ServerLog("recv_ctrlMsg \n");
#ifdef 	TCP_LOG	
					writeLog("/home/tcp_log.txt",recvbuf);
#endif
					handler_CtrlMsg(ser->fd_A[i],recvbuf,ret,NULL);
				}  
			}
		}  
	} 
}
/*
@ 接收控制台消息
@ 
@ 无
*/
static void *Ctrl_Server(void *arg)
{
	Server *ser = (Server *)arg;
	struct timeval tv;
	int i=0;	
	char recvbuf[512]={0};
	memset(&tv, 0, sizeof(struct timeval));
	tv.tv_sec = 2;
		
	FD_ZERO(&ser->fdsr);
	FD_SET(ser->listenfd, &ser->fdsr);
	int ret =0;
	int new_fd=0; 
	struct sockaddr_in client_addr;
	size_t sin_size =sizeof(struct sockaddr_in);
	ser->maxsock = ser->listenfd;  
	ServerLog("ser->maxsock = %d\n",ser->maxsock);
	while(ser->quit){
		FD_ZERO(&ser->fdsr);
		
#ifdef SELECT_UDP
		FD_SET(ser->broSock, &ser->fdsr);
#endif
		FD_SET(ser->listenfd, &ser->fdsr);
		tv.tv_sec = 2;	
		tv.tv_usec = 0;  
		// add active connection to fd set	
		for (i = 0; i < BACKLOG; i++){  
			if (ser->fd_A[i] != 0){  
				FD_SET(ser->fd_A[i], &ser->fdsr);  
			}  
		}  
		ret = select(ser->maxsock + 1, &ser->fdsr, NULL, NULL, &tv);  
		if (ret < 0){  
			perror("select error ");  
			continue; 	
		}
		else if (ret == 0){  
			//ServerLog("select timeout\n");  
			continue;  
		}  
		recv_ctrlMsg(ser,recvbuf);
		// check whether a new connection comes 
		if (FD_ISSET(ser->listenfd, &ser->fdsr)){  
			new_fd = accept(ser->listenfd, (struct sockaddr *)&client_addr, &sin_size); 
			ServerLog("new_fd =%d listenfd=%d  mount = %d\n",new_fd,ser->listenfd,ser->conn_amount);
			if (new_fd <= 0){
				perror("Ctrl_Server :accept error");
				sleep(1);
				continue;
			}
			ServerLog("recv ctrl accept\n");
			if(ser->conn_amount >= (BACKLOG-1)){
				ack_batteryCtr(get_battery(),get_charge());
			}
			add_queue(ser,new_fd,client_addr);
		}
		memset(recvbuf,0,512);
	}
	ser->quit=2;
}

#ifdef SELECT_UDP
int SendtoServicesWifi(char *msg,int size)
{
	return sendto(ctrSer->broSock,msg,size,0,(const struct sockaddr *)&ctrSer->wifiAddr,sizeof(struct sockaddr_in));
}
#endif
#ifdef SPEEK_VOICES
int SendtoaliyunServices(const void *msg,int size)
{
	return sendto(ctrSer->broSock,msg,size,0,(const struct sockaddr *)&ctrSer->speekAddr,sizeof(struct sockaddr_in));
}

#endif
static void checkNetFile(void)
{
	if(access("/var/startNet.lock",0) < 0){
		fopen("/var/startNet.lock","w+");
	}
}

/*
@ 初始化视频服务器 ,消息控制、视频数据服务
@ 
@ 无
*/
void init_videoServer(void)
{
	//控制数据端口
	ctrSer= Server_Alloc();
	if(!ctrSer){
		return ;
	}
#ifdef SELECT_UDP
	ctrSer->broSock= create_listen_brocast(NULL,CTRL_PORT);
	if(ctrSer->broSock<0){
		return ;
	}
	char IP[20]={0};
	if(GetNetworkcardIp("br0",IP))
	{
			perror("get br0 ip failed");
			return ;
	}
	init_addr(&ctrSer->wifiAddr, IP,  20003);
	//GetNetState();
	checkNetFile();
#endif
#ifdef SPEEK_VOICES
	init_addr(&ctrSer->speekAddr, IP,  SPEEK_PORT);
#endif
	initSock(ctrSer,BASE_PORT);
	if(pthread_create_attr(Ctrl_Server,ctrSer)){
		ServerLog("pthread_create_attr Ctrl_Server failed \n");
		return ;
	}
	ServerLog("init videoServerr success\n");
}

static void  __close_server(Server *ser)
{
	int  i=0;
	 // close other connections  
    for (i = 0; i < BACKLOG; i++){  
        if (ser->fd_A[i] != 0){  
            close(ser->fd_A[i]);
        }  
    } 
}
/*
@ 接收控制台消息
@ 
@ 无
*/
void clean_videoServer(void)
{
#ifdef SELECT_UDP
	if(ctrSer->broSock>0){
		close(ctrSer->broSock);
	}
#endif
	//close(ctrSer->maxsock);
	Server_Free(ctrSer);
	ServerLog(" clean_videoServer success\n");
}

