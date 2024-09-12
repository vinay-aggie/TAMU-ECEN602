# Compiler and flags
CC = gcc
CFLAGS = -Wall -O2

# Directories for binaries and object files
BIN_DIR = bin
OBJ_DIR = obj

# Define the targets
TARGET1 = $(BIN_DIR)/echo
TARGET2 = $(BIN_DIR)/echos

# Source files
SRC_CLIENT = client.c
SRC_SERVER = server.c

# Object files
OBJ_CLIENT = $(OBJ_DIR)/client.o
OBJ_SERVER = $(OBJ_DIR)/server.o

# All target
.PHONY: all
all: $(TARGET1) $(TARGET2)

# Ensure bin and obj directories exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $(BIN_DIR) $(OBJ_DIR)

# Link object files into the two executables
$(TARGET1): $(OBJ_CLIENT) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJ_CLIENT)

$(TARGET2): $(OBJ_SERVER) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJ_SERVER)

# Compile the individual source files
$(OBJ_CLIENT): $(SRC_CLIENT) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC_CLIENT) -o $(OBJ_CLIENT)

$(OBJ_SERVER): $(SRC_SERVER) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC_SERVER) -o $(OBJ_SERVER)

# Clean target to remove binaries and object files
.PHONY: clean
clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR)
	@echo "Cleaned up all binaries and object files."

# Optional debug target to build with debugging symbols
.PHONY: debug
debug: CFLAGS += -g
debug: clean all
