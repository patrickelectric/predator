# project directories
PRJDIR 		 := $(shell pwd)

OUTDIR       := $(PRJDIR)

LIBDIR       := $(PRJDIR)/lib
OBJDIR       := $(PRJDIR)/bin
CODDIR       := $(PRJDIR)/src

# C source files

#modules
C_SRC += $(CODDIR)/main.cpp
C_LIB += $(LIBDIR)/ 
	
# Define programs and commands.
CC      = 	g++

# compiler flags
CFLAGS  = `pkg-config --libs --cflags opencv` 
CFLAGS += -lpthread -Ofast -funroll-loops -O3
CFLAGS += -Wall -Wno-write-strings -Wno-return-local-addr
CFLAGS += -g 

all:
	$(CC) $^ $(C_SRC) -o $(OUTDIR)/main -I$(C_LIB) $(CFLAGS)

*.o:
	$(CC) $(CFLAGS) $(LIBDIR)/*.h -c -o $(OBJDIR)/*.o

clean:	
	rm $(BIN)/*