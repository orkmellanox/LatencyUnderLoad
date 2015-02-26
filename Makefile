CC 		= g++
CFLAGS 	+=  -std=c++0x -g 
LDFLAGS += 
OBJECTS	= main.o server.o client.o common.o cbuffer.o connection.o 
TARGETS	= latencyLoadTest

all: $(TARGETS)
latencyLoadTest: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
main.o: main.cpp common.h
	$(CC) -c $(CFLAGS) $<
server.o: server.cpp common.h
	$(CC) -c $(CFLAGS) $<
client.o: client.cpp common.h
	$(CC) -c $(CFLAGS) $<
common.o: common.cpp common.h connection.h
	$(CC) -c $(CFLAGS) $<	
connection.o: connection.cpp connection.h common.h cbuffer.h squeue.h
	$(CC) -c $(CFLAGS) $<	
cbuffer.o: cbuffer.cpp cbuffer.h
	$(CC) -c $(CFLAGS) $<		
clean:
	rm -f $(OBJECTS) $(TARGETS)
