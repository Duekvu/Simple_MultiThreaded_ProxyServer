OBJS= MyProxy.o
CC=g++
CFLAGS= -c -Wall 
LFLAGS=-pthread  -Wall 

MyProxy: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o  MyProxy
MyProxy.o: MyProxy.cpp Declaration.h
	$(CC) $(CFLAGS) MyProxy.cpp
.PHONY: clean
clean:
	rm *.o MyProxy
