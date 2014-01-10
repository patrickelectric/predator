OPENCV=	`pkg-config --libs --cflags opencv`
all:
	g++ src/main.cpp -o main $(OPENCV) -lpthread -Ofast -funroll-loops

