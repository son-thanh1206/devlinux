CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -Werror -std=c99
BIN_DIR = bin

all: $(BIN_DIR)/manager $(BIN_DIR)/searcher

$(BIN_DIR)/manager: manager.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/searcher: searcher.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: all
	./$(BIN_DIR)/manager

clean:
	rm -rf $(BIN_DIR)

.PHONY: all run clean