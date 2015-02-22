CC 		= g++
CFLAGS 	+=  -std=c++0x 
LDFLAGS += 
OBJECTS	= main.o server.o client.o common.o cbuffer.o
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
common.o: common.cpp common.h cbuffer.h squeue.h
	$(CC) -c $(CFLAGS) $<	
cbuffer.o: cbuffer.cpp cbuffer.h
	$(CC) -c $(CFLAGS) $<		
clean:
	rm -f $(OBJECTS) $(TARGETS)
