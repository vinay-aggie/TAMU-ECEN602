# Compiler and flags
CC = gcc
CFLAGS = -Wall -O2

# Directories for binaries and object files
BIN_DIR = bin
OBJ_DIR = obj

# Define the targets
TARGET1 = $(BIN_DIR)/echo
TARGET2 = $(BIN_DIR)/echos

# Source and object files
SRC_CLIENT = client.c
SRC_SERVER = server.c
SRC_SOCKET_UTILS = socketUtils.c

OBJ_CLIENT = $(OBJ_DIR)/client.o
OBJ_SERVER = $(OBJ_DIR)/server.o
OBJ_SOCKET_UTILS = $(OBJ_DIR)/socketUtils.o

# All target
.PHONY: all
all: $(TARGET1) $(TARGET2)

# Ensure bin and obj directories exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $(BIN_DIR) $(OBJ_DIR)

# Link object files into the two executables
$(TARGET1): $(OBJ_CLIENT) $(OBJ_SOCKET_UTILS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJ_CLIENT) $(OBJ_SOCKET_UTILS)

$(TARGET2): $(OBJ_SERVER) $(OBJ_SOCKET_UTILS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJ_SERVER) $(OBJ_SOCKET_UTILS)

# Compile the individual source files
$(OBJ_CLIENT): $(SRC_CLIENT) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC_CLIENT) -o $(OBJ_CLIENT)

$(OBJ_SERVER): $(SRC_SERVER) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC_SERVER) -o $(OBJ_SERVER)

$(OBJ_SOCKET_UTILS): $(SRC_SOCKET_UTILS) socketUtils.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC_SOCKET_UTILS) -o $(OBJ_SOCKET_UTILS)

# Clean target to remove binaries and object files
.PHONY: clean
clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR)
	@echo "Cleaned up all binaries and object files."

# Optional debug target to build with debugging symbols
.PHONY: debug
debug: CFLAGS += -g
debug: clean all
