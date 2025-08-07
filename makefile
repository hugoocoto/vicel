BUILD_DIR = .
BIN_NAME = vicel
OBJ_DIR = ./objs

INC = -I.
LIB = -lm

HEADERS = $(wildcard src/*.h)
SRC = $(wildcard src/*.c)
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))

.DEFAULT_GOAL := debug

DEBUG_CC  = cc -ggdb -std=gnu11 -O0 -DDEBUG=1 -Wall -Wextra -Wno-char-subscripts
DEBUG_OUT = $(BUILD_DIR)/$(BIN_NAME)_debug

RELEASE_CC  = cc -std=gnu11 -O3 -DDEBUG=0 -w
RELEASE_OUT = $(BUILD_DIR)/$(BIN_NAME)

ifeq ($(MAKECMDGOALS),release)
	CC := $(RELEASE_CC)
	OUT := $(RELEASE_OUT)
else
	CC := $(DEBUG_CC)
	OUT := $(DEBUG_OUT)
endif

debug: $(OUT)
	gdb -ex "run data.csv" $(OUT) 

release: $(OUT)

$(OUT): $(OBJ) $(OBJ_DIR) $(BUILD_DIR) wc.md
	$(CC) $(OBJ) $(INC) $(LIB) -o $(OUT)

$(OBJ_DIR)/%.o: %.c $(HEADERS) makefile
	mkdir -p $(dir $@)
	$(CC) -c $< $(INC) -o $@

wc.md: $(SRC) $(HEADERS)
	cloc src --by-file --hide-rate --md > wc.md

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(OBJ_DIR) $(DEBUG_OUT) $(RELEASE_OUT) wc.md

install: release clean
	mv $(RELEASE_OUT) ~/.local/bin/$(BIN_NAME)

gen: debug
	./gen_release_version.sh

