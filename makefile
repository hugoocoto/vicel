BUILD_DIR = .
BIN_NAME = vicel
OBJ_DIR = ./objs
OUT = $(BUILD_DIR)/$(BIN_NAME)
INC = -I.
LIB = -lm 
HEADERS = $(wildcard src/*.h src/vispel/*.h src/vispel/core/*.h)
SRC = $(wildcard src/*.c src/vispel/*.c src/vispel/core/*.c)
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
PYC := $(shell python3-config --embed --cflags)
PYL := $(shell python3-config --embed --ldflags) 

CC = gcc
TARGET = 
FLAGS = -ggdb -std=gnu11 -O0 -DDEBUG=1 -Wall -Wextra -Wno-char-subscripts 
#-fsanitize=address,null 

COMP = $(CC) $(TARGET) $(FLAGS)


$(OUT): $(OBJ) $(OBJ_DIR) $(BUILD_DIR) wc.md
	$(COMP) $(OBJ) $(INC) $(PYL) $(LIB) -o $(OUT)
	rm -f report.log log.txt

$(OBJ_DIR)/%.o: %.c $(HEADERS) makefile
	mkdir -p $(dir $@) && $(COMP) $(PYC) -c $< $(INC) -o $@ 

wc.md: $(SRC) $(HEADERS)
	cloc src --by-file --hide-rate --md > wc.md

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(OBJ_DIR) $(DEBUG_OUT) $(RELEASE_OUT) 

install: $(OUT) clean
	mv $(OUT) ~/.local/bin/$(BIN_NAME)

uninstall: clean
	rm ~/.local/bin/$(BIN_NAME) -f

release:
	gcc `find . -name "*.c"` -w -o $(OUT) $(LIB) $(INC) $(PYC) $(PYL)

compile_flags:
	$(PYC) | sed "s/ \+/\n/g" > compile_flags.txt

.PHONY: clean install uninstall release
