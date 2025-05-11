CC ?= gcc

SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := bin-int

PREFIX ?= /usr/local

CFLAGS_COMMON := -Wall -Wextra -I./include
CFLAGS_DEBUG := -g -O0 -DZK_ENABLE_ASSERTS
CFLAGS_RELEASE := -O3 -DNDEBUG

LDFLAGS = -L /usr/local/lib -lcjson -llua5.4

SRC := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/$(CFG)/%.o,$(SRC))
TARGET := zenkai

.PHONY: all debug release build install clean

all: debug

debug:
	$(MAKE) build CFG=debug CFLAGS="$(CFLAGS_COMMON) $(CFLAGS_DEBUG)"

release:
	$(MAKE) build CFG=release CFLAGS="$(CFLAGS_COMMON) $(CFLAGS_RELEASE)"

build: $(BIN_DIR)/$(CFG)/$(TARGET)

$(BIN_DIR)/$(CFG)/$(TARGET): $(OBJ)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/$(CFG)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

install: release
	sudo install -m 755 $(BIN_DIR)/release/$(TARGET) $(PREFIX)/bin/
	sudo mkdir -p $(PREFIX)/share/$(TARGET)
	sudo cp -r rules $(PREFIX)/share/$(TARGET)

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
