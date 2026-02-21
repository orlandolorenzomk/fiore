CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -Wno-gnu-zero-variadic-macro-arguments -I include
LDFLAGS =

TARGET  = supervisor
BIN     = bin
BUILD   = build
SRC     = src

SRCS    = $(SRC)/main.c \
          $(SRC)/logger.c \
          $(SRC)/process_table.c \
          $(SRC)/supervisor.c

OBJS    = $(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(SRCS))

.PHONY: all clean install

all: $(BIN)/$(TARGET)

$(BIN)/$(TARGET): $(OBJS) | $(BIN)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD):
	mkdir -p $(BUILD)

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -rf $(BUILD) $(BIN)

install: $(BIN)/$(TARGET)
	install -m 755 $(BIN)/$(TARGET) /usr/local/bin/$(TARGET)
