#
# Makefile for 'Hamster biff
#

CC	= gcc -c
CFLAGS	= -Zmtd -O2 -DDEBUG
LD	= gcc
LDFLAGS	= -s -Zmtd
EMXPATH = D:\emx

#
# Inference Rules
#
.c.o :
	$(CC) $(CFLAGS) $*.c

#
# Target to Build
#

TARGET = hambiffj.exe hambiffe.exe

all : $(TARGET)

jp : hambiffj.exe

en : hambiffe.exe

#
# Files to Use
#

SRCS = main.c bitmap.c window.c biff.c pop3.c setup.c about.c
OBJS = main.o bitmap.o window.o biff.o pop3.o setup.o about.o
LIBS = -lsocket
BMPS = bmpwait.bmp bmppoll.bmp bmpmail.bmp bmpfail.bmp

hambiffj.exe : $(OBJS) hambiff.def hamresjp.res
	$(LD) $(LDFLAGS) -o hambiffj.exe hambiff.def hamresjp.res $(OBJS) $(LIBS)

hambiffe.exe : $(OBJS) hambiff.def hamresen.res
	$(LD) $(LDFLAGS) -o hambiffe.exe hambiff.def hamresen.res $(OBJS) $(LIBS)

hamresjp.res : hamresjp.rc hamres.h $(BMPS) hambiff.ico
	rc -r -i $(EMXPATH)\include hamresjp.rc

hamresen.res : hamresen.rc hamres.h $(BMPS) hambiff.ico
	rc -r -i $(EMXPATH)\include hamresen.rc

main.o   : main.c hambiff.h hamres.h

bitmap.o : bitmap.c hambiff.h hamres.h

window.o : window.c hambiff.h hamres.h

biff.o   : biff.c hambiff.h hamres.h

pop3.o   : pop3.c

setup.o  : setup.c hambiff.h hamres.h

about.o  : about.c hambiff.h hamres.h

