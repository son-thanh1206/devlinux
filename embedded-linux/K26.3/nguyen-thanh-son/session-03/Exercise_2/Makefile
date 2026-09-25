CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -Werror -std=c99
BIN_DIR = bin
BIN_NAME = product_management
TARGET  = $(BIN_DIR)/$(BIN_NAME)
SRCS    = main.c

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
	rm -rf *.dat