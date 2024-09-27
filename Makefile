# Makefile for building client and server programs

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGETS = client server
SRC_CLIENT = client.c socketUtils.c
SRC_SERVER = server.c socketUtils.c
OBJ_CLIENT = $(SRC_CLIENT:.c=.o)
OBJ_SERVER = $(SRC_SERVER:.c=.o)

# Build all targets (client and server)
all: $(TARGETS)

# Build client
client: $(OBJ_CLIENT)
	$(CC) $(CFLAGS) -o client $(OBJ_CLIENT)

# Build server
server: $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o server $(OBJ_SERVER)

# Clean up
clean:
	rm -f $(OBJ_CLIENT) $(OBJ_SERVER) $(TARGETS)

# Rebuild from scratch
rebuild: clean all
