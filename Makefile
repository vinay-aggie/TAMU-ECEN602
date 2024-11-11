# Compiler options.
CC = g++
CFLAGS = --std=c++11 -Wall 

# Define the 2 targets. We have a client
# and a server.
TARGET1 = client
TARGET2 = proxy

# Rules are applicable for all targets.
all: $(TARGET1) $(TARGET2)

# Link object files into the two executables.
$(TARGET1): client.o
	$(CC) $(CFLAGS) -o $(TARGET1) client.o

$(TARGET2): server.o
	$(CC) $(CFLAGS) -o $(TARGET2) server.o

# Compile the individual files but do not link.
client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp

server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp

# clean using 'make clean'
clean:
	rm -f $(TARGET1) $(TARGET2) *.o