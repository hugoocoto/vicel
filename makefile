BUILD_DIR = .
BIN_NAME = vicel
OBJ_DIR = ./objs
OUT = $(BUILD_DIR)/$(BIN_NAME)
INC = -I.
LIB = -lm 
HEADERS = $(wildcard src/*.h src/vispel/*.h src/vispel/core/*.h)
SRC = $(wildcard src/*.c src/vispel/*.c src/vispel/core/*.c)
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
LUA ?= lua5.4
LUA_CFLAGS := $(shell pkg-config --cflags $(LUA) 2>/dev/null || pkg-config --cflags lua 2>/dev/null)
LUA_LIBS := $(shell pkg-config --libs $(LUA) 2>/dev/null || pkg-config --libs lua 2>/dev/null || echo '-l$(LUA)')

CC = gcc
FLAGS = -ggdb -std=gnu11 -O0 -DDEBUG=1 -Wall -Wextra -Wno-char-subscripts -fsanitize=address,null 

COMP = $(CC) $(FLAGS)


$(OUT): $(OBJ) $(OBJ_DIR) $(BUILD_DIR) wc
	$(COMP) $(OBJ) $(INC) $(LUA_LIBS) $(LIB) -o $(OUT)
	rm -f report.log log.txt

$(OBJ_DIR)/%.o: %.c $(HEADERS) makefile
	mkdir -p $(dir $@) 
	$(COMP) -c $< $(INC) $(LUA_CFLAGS) -o $@

wc: $(SRC) $(HEADERS)
	wc `find src -name "*.[ch]"` > wc

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(OBJ_DIR) $(DEBUG_OUT) $(RELEASE_OUT) 

install: clean release
	mv $(OUT) ~/.local/bin/$(BIN_NAME)

uninstall: clean
	rm ~/.local/bin/$(BIN_NAME) -f

release:
	gcc `find src -name "*.c"` -w -o $(OUT) $(LIB) $(INC) $(LUA_CFLAGS) $(LUA_LIBS)

compile_flags:
	printf '%s\n' "$(LUA_CFLAGS)" | sed "s/ \+/\n/g" > compile_flags.txt

.PHONY: clean install uninstall release
