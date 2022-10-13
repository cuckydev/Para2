# SDK Environment
SHELL = /bin/sh
TOP = /usr/local/sce/ee

LIBDIR = $(TOP)/lib

# Compiler
PREFIX = ee
AS = $(PREFIX)-gcc
CC = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
LD = $(PREFIX)-gcc
DVPASM = $(PREFIX)-dvp-as
OBJDUMP = $(PREFIX)-objdump

# Compiler flags
TARGET = para

LCFILE = $(LIBDIR)/app.cmd
LIBS = \
	$(LIBDIR)/libgraph.a \
	$(LIBDIR)/libvu0.a \
	$(LIBDIR)/libdev.a \
	$(LIBDIR)/libdma.a \
	$(LIBDIR)/libcdvd.a

CFLAGS = -O2 -Wa,-al -fno-common -G0 -Isrc -I$(TOP)/../common/include -I$(TOP)/include
CXXFLAGS = $(CFLAGS) -fno-exceptions
ASFLAGS = -c -xassembler-with-cpp -Wa,-al
DVPASMFLAGS =
LDFLAGS = -Wl,-Map,$(TARGET).map -mno-crt0 -L$(TOP)/lib
TMPFLAGS =

# Dependency flags
CFLAGS += -MMD -MP
CXXFLAGS += -MMD -MP

# Sources
SRCS = \
	src/main/main.c \
	src/os/mtc.c \
	src/os/syssub.c \
	src/os/system.c

# Objects
OBJS = $(addprefix obj/, $(addsuffix .o, $(SRCS)))
OBJS += obj/crt0.o
DEPS = $(addprefix obj/, $(addsuffix .o.d, $(SRCS)))

# Compile rules
all: $(TARGET).elf

$(TARGET).elf: $(OBJS) $(LIBS)
	$(LD) -o $@ -T $(LCFILE) $(OBJS) $(LIBS) $(LDFLAGS)

obj/%.c.o: %.c
	@ mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TMPFLAGS) -MF $@.d $< -o $@ -c

obj/crt0.o: $(LIBDIR)/crt0.s
	@ mkdir -p $(@D)
	$(AS) $(ASFLAGS) $(TMPFLAGS) $< -o $@ -c

include $(wildcard $(DEPS))

clean:
	rm -rf obj/