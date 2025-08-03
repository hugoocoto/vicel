CC = cc -ggdb -std=gnu11 -O0 -fsanitize=address,null
INC = -I.
LIB = -lm
HEADERS = $(wildcard src/*.h)
SRC = $(wildcard src/*.c)
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
OBJ_DIR = ./objs
BUILD_DIR = .
OUT = $(BUILD_DIR)/00-vicel

gen: test 
	./gen_release_version.sh

test: $(OUT)
	rm report.log -f
	./$(OUT) untitled
# gdb -ex run $(OUT) 

$(OUT): $(OBJ) $(OBJ_DIR) $(BUILD_DIR) wc.md 
	$(CC) $(OBJ) $(INC) $(LIB) -o $(OUT)

wc.md: $(SRC) $(HEADERS)
	cloc src --by-file --hide-rate --md > wc.md

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR)/%.o: %.c $(HEADERS)
	mkdir -p $(dir $@) && $(CC) -c $< $(INC) -o $@

clean:
	rm -rf $(OBJ_DIR)

install: $(OUT) clean
	mv $(OUT) ~/.local/bin/$(OUT)
