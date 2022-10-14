# SDK Environment
SHELL = /bin/sh
TOP = /usr/local/sce/iop

LIBDIR = $(TOP)/lib

# Compiler
PREFIX = iop
AS = $(PREFIX)-gcc
CC = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
LD = $(PREFIX)-gcc
DVPASM = $(PREFIX)-dvp-as
OBJDUMP = $(PREFIX)-objdump

# Compiler flags
MODULE = wave2ps2
IRX = $(MODULE).irx
ILB = src/$(MODULE).ilb
TBL = $(MODULE).tbl
ENT = src/$(MODULE).o

LIBS = -ilb=libsd.ilb -ilb=iop.ilb

CFLAGS = -I$(TOP)/common/include -Iinclude -Isrc -Wall -G0 -g
ASFLAGS = -I$(TOP)/common/include -Iinclude -G0

COMPILE.s = $(CC) $(ASFLAGS) $(CFLAGS)

# Sources
OBJS = \
	src/bgm_entr.o \
	src/bgm_com.o

# Compile rules
all: $(IRX)

$(IRX): $(OBJS)
	$(LINK.o) $(OBJS) -o $(IRX) $(LIBS)

clean:
	$(RM) $(OBJS) $(IRX)
