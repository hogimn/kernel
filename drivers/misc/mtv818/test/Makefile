#CC=arm-generic-linux-gnueabi-gcc
#CC=arm-none-eabi-gcc
#CC=arm-linux-gnueabi-gcc
CC=arm-linux-gcc

TARGET		= testapp

INCLUDES	= -I. -I./.. -I./../src 
SUBLIBS		=  -lpthread
CFLAGS		= -Wall -D_REENTRANT 
LDFLAGS		= 

SRCS    = test.c 
OBJS	= $(SRCS:.c=.o)

#CFLAGS      += -D__MAIN__

.SUFFIXES: .c .o

# .PHONY: all clean

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

$(TARGET):	$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(SUBLIBS) -lm -static
#	cp testapp /home/RootFS/v210/linux/home/root/


all:	$(TARGET)

clean:
	rm -f $(TARGET) *.o *.a *~

distclean:
	rm -f $(TARGET) *.o *.a *.bak *~ .depend

dep:    depend

depend:
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend


#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif

