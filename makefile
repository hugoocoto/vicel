BUILD_DIR = .
BIN_NAME = vicel
OBJ_DIR = ./objs
OUT = $(BUILD_DIR)/$(BIN_NAME)
INC = -I.
LIB = -lm 
HEADERS = $(wildcard src/*.h src/vispel/*.h src/vispel/core/*.h)
SRC = $(wildcard src/*.c src/vispel/*.c src/vispel/core/*.c)
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
CC = gcc
TARGET = 
FLAGS = -ggdb -std=gnu11 -O0 -DDEBUG=1 -fsanitize=address,null -Wall -Wextra -Wno-char-subscripts 

COMP = $(CC) $(TARGET) $(FLAGS)

$(OUT): $(OBJ) $(OBJ_DIR) $(BUILD_DIR) wc.md 
	$(COMP) $(OBJ) $(INC) $(LIB) -o $(OUT)
	rm -f report.log log.txt

$(OBJ_DIR)/%.o: %.c $(HEADERS) makefile
	mkdir -p $(dir $@) && $(COMP) -c $< $(INC) -o $@

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
