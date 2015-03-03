CC 		= g++
CFLAGS 	+= -g
LDFLAGS +=  -lrt
OBJECTS	= main.o server.o client.o common.o cbuffer.o connection.o Ticks.o 
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
connection.o: connection.cpp connection.h common.h cbuffer.h squeue.h Ticks.h
	$(CC) -c $(CFLAGS) $<	
cbuffer.o: cbuffer.cpp cbuffer.h
	$(CC) -c $(CFLAGS) $<		
Ticks.o: Ticks.cpp Ticks.h ticks_os.h clock.h
	$(CC) -c $(CFLAGS) $<		
clean:
	rm -f $(OBJECTS) $(TARGETS)
