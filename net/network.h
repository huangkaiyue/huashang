#ifndef _NETWORK_H
#define _NETWORK_H

#include <netinet/in.h>
#include "config.h"

#define BACKLOG 5     // how many pending connections queue will hold  

#include "StreamFile.h"
typedef struct{
	unsigned char quit;
	unsigned char playstate[BACKLOG];
	int listenfd;
	int broSock;
	int fd_A[BACKLOG];    // accepted connection fd 
	struct sockaddr_in addr[BACKLOG];
	int conn_amount;    // current connection amount  
	int maxsock; 
	fd_set fdsr;		
	struct sockaddr_in wifiAddr;
#ifdef SPEEK_VOICES
	struct sockaddr_in speekAddr;
#endif
}Server;

#ifdef SPEEK_VOICES
#define SPEEK_PORT	20011
#endif

#define BASE_PORT		20000
#define CTRL_PORT		BASE_PORT+1

extern void handler_CtrlMsg(int sockfd,char *recvdata,int size,struct sockaddr_in *peer);
extern int send_ctrl_ack(int sockfd,char *data,int size);
extern int sendAll_Ack(char *data,int size);
extern void init_videoServer(void);
extern void clean_videoServer(void);
extern void ack_allplayerCtr(void *data,Player_t *player);
extern void ack_playCtr(int nettype,Player_t *player,unsigned char playState);
extern void ack_batteryCtr(int recvdata,int power);
extern void ack_alluserCtr(const int sockfd,int state,int power);
extern void GetNetState(void);

#endif
