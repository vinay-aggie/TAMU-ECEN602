# Variables
CC = gcc
CFLAGS = -Wall

TARGET = echos

all: $(TARGET)

$(TARGET): server.o
	$(CC) $(CFLAGS) -o $(TARGET) server.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f $(TARGET) *.o
