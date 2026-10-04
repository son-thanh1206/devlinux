CC      = gcc
CFLAGS  = -Wall -pthread -Wextra -pedantic -Werror -std=c99
BIN_DIR = bin
BIN_NAME = print_queue
TARGET  = $(BIN_DIR)/$(BIN_NAME)
SRCS    = print_queue.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BIN_DIR)