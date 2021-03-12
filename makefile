#
# Makefile for 'dupf'
#

CC     =  gcc -c
CFLAGS =  -Zmtd -O2 -DCACHEDIR
LD     = gcc
LFLAGS = -s -Zmtd

#
# Inference Rules
#
.c.o :
	$(CC) $(CFLAGS) $*.c

#
# Files
#

OBJS = dupf.o size.o crc32.o diff.o

#
# Targets to build
#

TARGET = dupf.exe

all : $(TARGET)

#
# Dependencies
#

dupf.o      : makefile dupf.c

size.o      : makefile size.c

crc32.o     : makefile crc32.c

diff.o      : makefile diff.c

dupf.exe    : makefile dupf.def $(OBJS)
	$(LD) $(LFLAGS) -o dupf.exe dupf.def $(OBJS)


