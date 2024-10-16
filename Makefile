# Compiler options.
CC = gcc
CFLAGS = -Wall

# Define the target.
TARGET1 = tftpServer

# Rules are applicable for all targets.
all: $(TARGET1)

# Link object file to the executable.
$(TARGET1): server.o
	$(CC) $(CFLAGS) -o $(TARGET1) server.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

# clean using 'make clean'
clean:
	rm -f $(TARGET1) *.o
