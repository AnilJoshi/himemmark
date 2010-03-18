# library name and version
PACKAGE_NAME:=himemmark
PACKAGE_VERSION:=1.0
# ----------------------------------------------------------------------------
# Define architectures -> gather system information
#
OS     = $(shell uname -s)
OS_REL = $(shell uname -r)
ARCH   = $(shell uname -m)
PROC   = $(shell uname -p)
HOST   = $(shell uname -n)
SYSTEM = $(OS)-$(ARCH)
NAME = $(shell uname -n)
#
EXEDIR = $(HOME)/bin
LIBDIR = $(HOME)/lib
BASE_NAME = $(PACKAGE_NAME)-$(PACKAGE_VERSION)-$(SYSTEM)
LD_FLAGS = -lm

# ----------------------------------------------------------------------------
# Compiler options for specific machines.
#
CC = gcc -Wall -pedantic -ansi -Wstrict-prototypes
BASE_CFLAGS  = -O2
ifdef GDB
  BASE_CFLAGS = -O0 -g
endif
#
PROG_NAME = $(addsuffix -opt,$(BASE_NAME))
SYMLINK_NAME = $(addsuffix -opt,$(PACKAGE_NAME))
ifdef GDB
  PROG_NAME = $(addsuffix -gdb,$(BASE_NAME))
  SYMLINK_NAME = $(addsuffix -gdb,$(PACKAGE_NAME))
endif
EXE_NAME = $(PROG_NAME)
LIB_NAME = $(addsuffix .a,$(PROG_NAME))
SYMLINK_LIB_NAME = $(addsuffix .a,$(SYMLINK_NAME))
ifdef LINKLIB
  PROG = $(LIB_NAME)
endif
ifdef LINKEXE
  PROG = $(EXE_NAME)
  CFLAGS = $(BASE_CFLAGS)
endif

C_FILES := $(wildcard *.c)
C_OBJECTS := $(patsubst %.c,%.o,$(C_FILES))

$(PROG): $(C_OBJECTS)
    ifdef LINKEXE
	$(CC) $(CFLAGS) -o $(PROG) *.o $(LD_FLAGS)
	install $(PROG) $(EXEDIR)/$(PROG)
	rm -f $(PROG)
    endif

linkexe:
	make LINKEXE=1

linkexe_gdb:
	make LINKEXE=1 GDB=1

all:
	make clean
	make linkexe
	make clean
	make linkexe_gdb
	make clean

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
# DO NOT DELETE THIS LINE -- make depend depends on it.
