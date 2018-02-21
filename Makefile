CC=gcc
CFLAGS=-c  
LDFLAGS=-g -Wall -lm -lpthread
SOURCES = deamon.c arg.c start.c socket.c thrd.c
OBJECTS = $(SOURCES:.c=.o)
BINARY = binary




all:$(SOURCES) $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	rm -f ./*.o
	./binary

.c.o:
	$(CC) $(CFLAGS)   $< -o $@ 

clean:
	rm -f ./*.o
	rm -f /run/tese.sock
