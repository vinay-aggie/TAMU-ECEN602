# Makefile for TCP Echo Server and Client

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Target executables
TARGETS = server client

# Source files
SRCS = server.c client.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default rule
all: $(TARGETS)

# Rule to build the server
server: server.o
	$(CC) $(CFLAGS) -o server server.o

# Rule to build the client
client: client.o
	$(CC) $(CFLAGS) -o client client.o

# Rule to compile server source file
server.o: server.c
	$(CC) $(CFLAGS) -c server.c

# Rule to compile client source file
client.o: client.c
	$(CC) $(CFLAGS) -c client.c

# Rule to clean up build files
clean:
	rm -f $(OBJS) $(TARGETS)

# Phony targets
.PHONY: all clean
