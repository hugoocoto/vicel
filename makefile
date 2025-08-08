BUILD_DIR = .
BIN_NAME = vicel
OBJ_DIR = ./objs
OUT = $(BUILD_DIR)/$(BIN_NAME)
INC = -I.
LIB = -lm
HEADERS = $(wildcard src/*.h)
SRC = $(wildcard src/*.c)
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
CC  = gcc -ggdb -std=gnu11 -O0 -DDEBUG=1 -Wall -Wextra -Wno-char-subscripts

$(OUT): $(OBJ) $(OBJ_DIR) $(BUILD_DIR) wc.md gen
	$(CC) $(OBJ) $(INC) $(LIB) -o $(OUT)
	rm -f report.log

$(OBJ_DIR)/%.o: %.c $(HEADERS) makefile
	mkdir -p $(dir $@) && $(CC) -c $< $(INC) -o $@

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

gen: 
	./gen_release_version.sh
