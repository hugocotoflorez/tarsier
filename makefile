CC = cc -ggdb -std=gnu99
INC = -I.
LIB = -Lraylib/src -lraylib -lm
HEADERS = $(wildcard src/*.h raylib)
SRC = $(wildcard src/*.c)
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
OBJ_DIR = ./objs
BUILD_DIR = ./
OUT = $(BUILD_DIR)/tarsier

$(OUT): $(OBJ) $(OBJ_DIR) $(BUILD_DIR) wc.md
	$(CC) $(OBJ) $(INC) $(LIB) -o $(OUT)

wc.md: $(SRC) $(HEADERS)
	cloc src --by-file --not-match-f='stb_ds\.h' --hide-rate --md > wc.md

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
	chmod +x ~/.local/bin/$(OUT)


raylib:
	sudo pacman --needed -S alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama
	[ -d raylib ] || git clone --depth 1 https://github.com/raysan5/raylib.git raylib
	make PLATFORM=PLATFORM_DESKTOP -C raylib/src/ # To make the static version.


documentation:
	wget https://gist.githubusercontent.com/justinmk/a5102f9a0c1810437885a04a07ef0a91/raw/b66b17a605590afbd725c092b0cdc58e15d461a2/XTerm-Control-Sequences.txt
