CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -Werror -std=c99 -D_DEFAULT_SOURCE
BIN_DIR = bin
LOG     = system.log
N       = 20

FLOCK   = $(BIN_DIR)/writer_flock
FCNTL   = $(BIN_DIR)/writer_fcntl

.PHONY: all clean test_flock test_fcntl

all: $(FLOCK) $(FCNTL)

$(BIN_DIR)/%: %.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Spawn N writers concurrently, then check every line landed intact
define run_test
	rm -f $(LOG)
	for i in $$(seq 1 $(N)); do ./$(1) "$(2) message $$i" & done; wait
	cat $(LOG)
	@lines=$$(wc -l < $(LOG)); \
	bad=$$(grep -cvE '^\[PID:[0-9]+\] \[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\] \[INFO\] .+$$' $(LOG)); \
	echo "lines=$$lines expected=$(N) malformed=$$bad"; \
	[ "$$lines" -eq $(N) ] && [ "$$bad" -eq 0 ] && echo "PASS" || { echo "FAIL"; exit 1; }
endef

test_flock: $(FLOCK)
	$(call run_test,$(FLOCK),flock)

test_fcntl: $(FCNTL)
	$(call run_test,$(FCNTL),fcntl)

clean:
	rm -rf $(BIN_DIR) $(LOG)
