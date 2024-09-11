# Compiler options.
CC = gcc
CFLAGS = -Wall

# Define the 2 targets. We have a client
# and a server.
TARGET1 = echo
TARGET2 = echos

# Rules are applicable for all targets.
all: $(TARGET1) $(TARGET2)

# Link object files into the two executables.
$(TARGET1): client.o
	$(CC) $(CFLAGS) -o $(TARGET1) client.o

$(TARGET2): server.o
	$(CC) $(CFLAGS) -o $(TARGET2) server.o

# Compile the individual files but do not link.
client.o: client.c socketUtils.o
	$(CC) $(CFLAGS) -c socketUtils.o client.c

server.o: server.c socketUtils.o
	$(CC) $(CFLAGS) -c socketUtils.o server.c

socketUtils.o: socketUtils.c socketUtils.h
	$(CC) $(CFLAGS) -c socketUtils.c

# clean using 'make clean'
clean:
	rm -f $(TARGET1) $(TARGET2) *.o
