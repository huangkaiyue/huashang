PATH=/opt/buildroot-gcc463/usr/bin/
CROSS_COMPILE	=$(PATH)mipsel-linux-
CC=$(CROSS_COMPILE)gcc
#CC=gcc

SDK_PATH=/home/7620sdk/7688/sdk4300_20140916/RT288x_SDK/source
#SDK_PATH=/home/7620sdk/7688/sdk4300_20140916/RT288x_SDK/uart_source7688
KERNEL_PATH =$(SDK_PATH)/linux-2.6.36.x

OTHEN_LIB=-lmsc463 -lTLRobot342
SYSTEM_LIB=-lpthread -ldl -lrt  $(SDK_PATH)/lib/libnvram/libnvram-0.9.28.so
OPEN_LIB= -lz -lcurl -lssl -lcrypto -lmad  -lopencore-amrnb 
MY_LIB=-lbase463 -lmtkwifi -ldownFile463 -ldemoquick463 -lsystools463

OPEN_INC=-I /home/yue/work0615/demolib/libcurl/curl-7.50.1/output/mips/include/
OTHEN_INC= -I ./host/StreamPlay/
KERNEL_INC=-I $(KERNEL_PATH)/drivers/char/i2s/ -I $(KERNEL_PATH)/include/ -I $(KERNEL_PATH)/drivers/char/
SDK_INC=-I $(SDK_PATH)/lib/libnvram/ 

CFLAGS = -Wall -I ./include $(OTHEN_INC) $(KERNEL_INC) $(SDK_INC) $(OPEN_INC)
LDFLAGS= $(SYSTEM_LIB) $(OTHEN_LIB) $(OPEN_LIB)  $(MY_LIB)      


TAR = localserver
all +=main.o
all +=log.o
all +=net/network.o
all +=net/parseCmd.o
all +=uart/uart.o
all +=udpsrv/newBindbro.o
#all +=mtkwifiLib/mtkwifi.o
all +=usrdb/sysdata.o

all +=srvwork/workinter.o

all +=host/voices/wm8960i2s.o
all +=host/voices/callvoices.o
all +=host/voices/message_wav.o
all +=host/voices/eventVoices.o
all +=host/voices/gpio_7620.o

all +=host/sdcard/sdcard.o
all +=host/studyvoices/std_worklist.o
all +=host/studyvoices/qtts_qisc.o
all +=host/ap_sta.o

all +=host/StreamPlay/madplayer.o
all +=host/StreamPlay/newStreamFile.o


export CC
$(TAR): $(all)
	$(CC) $(CFLAGS) -o $(TAR) $(all) $(LDFLAGS)
#	cp $(TAR) /nfs/yue/
#	cp $(TAR) $(SDK_PATH)/romfs/bin/
	$(RM) -f *.gch *.bak $(all) 
	
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $< 

.PHONY: clean
clean:
	rm -f $(TAR) $(all) 
