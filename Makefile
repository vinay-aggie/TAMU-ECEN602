# Compiler options.
CC = gcc
CFLAGS = -Wall

# Define the 2 targets. We have a client
# and a server.
TARGET1 = client
TARGET2 = server

# Rules are applicable for all targets.
all: $(TARGET1) $(TARGET2)

# Link object files into the two executables.
$(TARGET1): client.o socketUtils.o
	$(CC) $(CFLAGS) -o $(TARGET1) client.o socketUtils.o

$(TARGET2): server.o socketUtils.o
	$(CC) $(CFLAGS) -o $(TARGET2) server.o socketUtils.o

# Compile the individual files but do not link.
client.o: client.c
	$(CC) $(CFLAGS) -c client.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

socketUtils.o: socketUtils.c socketUtils.h
	$(CC) $(CFLAGS) -c socketUtils.c

# clean using 'make clean'
clean:
	rm -f $(TARGET1) $(TARGET2) *.o
