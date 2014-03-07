# project directories
PRJDIR 		 := $(shell pwd)

OUTDIR       := $(PRJDIR)

CODDIR       := $(PRJDIR)/src
LIBDIR       := $(PRJDIR)/lib

# C source files

#modules
C_SRC += $(CODDIR)/main.cpp
C_LIB += $(LIBDIR)/ 
	
# Define programs and commands.
CC      = 	g++

# compiler flags
CFLAGS  =`pkg-config --libs --cflags opencv` 
CFLAGS += -Wall -lpthread -Ofast -funroll-loops 

all:
	$(CC) $^ $(C_SRC) -o $(OUTDIR)/main -I$(C_LIB) $(CFLAGS)