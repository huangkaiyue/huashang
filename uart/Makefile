PATH=/opt/buildroot-gcc463/usr/bin/
CROSS_COMPILE	=$(PATH)mipsel-linux-
CC=$(CROSS_COMPILE)gcc

SDK_PATH=/home/7620sdk/7688/sdk4300_20140916/RT288x_SDK/source
#SDK_PATH=/home/7620sdk/7688/bak7628/source/
KERNEL_PATH =$(SDK_PATH)/linux-2.6.36.x

CFLAGS = -Wall -I ../include -I ./host/StreamPlay/ -I $(KERNEL_PATH)/drivers/char/i2s/ -I $(KERNEL_PATH)/include/ -I $(KERNEL_PATH)/drivers/char/ -I $(SDK_PATH)/lib/libnvram/
#CFLAGS = -Wall -fpic -shared -I ../include -I ./host/StreamPlay/ -I $(KERNEL_PATH)/drivers/char/i2s/ -I $(KERNEL_PATH)/include/ -I $(KERNEL_PATH)/drivers/char/ -I $(SDK_PATH)/lib/libnvram/

#targ = libuartmcu.so
targ = uartmcu

all +=uart.o

LDLIBS= -L./ 
export CC

$(targ): $(all)
	$(CC) $(CFLAGS) -o $(targ) $(all) $(LDLIBS) -lpthread -lm -lbase463
#	cp $(targ) /nfs/tang/
	$(RM) -f *.gch *.bak $(all) 

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $< 

.PHONY: clean
clean:
	rm -f $(targ) $(all) 
