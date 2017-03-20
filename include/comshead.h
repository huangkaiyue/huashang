#ifndef _COMSHEAD_H
#define _COMSHEAD_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/autoconf.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>
#include <limits.h>
#include <getopt.h>
#include <memory.h>
#include <time.h>

#include <termios.h>
#include <linux/input.h>

#include <bits/types.h>
#include <dirent.h>
#include <sys/vfs.h>

#include <sys/socket.h>
#include <netinet/in.h>    
#include <arpa/inet.h>
#include <linux/tcp.h>
#include <assert.h>
#include <linux/netfilter_ipv4.h>

#include <setjmp.h>  
#include <stdarg.h> 


#define MAIN_QUEUE_LOCK		1		//对主线程队列上锁，将队列里面的事件全部清掉
#define MAIN_QUEUE_UNLOCK	0		//对主线程队列解锁，只要有事件进到队列当中，直接消费

#endif
