tarsier: raylib objs/tarsier.o objs/term.o
	gcc objs/tarsier.o objs/term.o -Lraylib/src/ -lraylib -Wall -Wextra -o tarsier -lm -ggdb

objs/tarsier.o: src/tarsier.c src/term.h src/ascii.h objs
	gcc -c src/tarsier.c -ggdb -o objs/tarsier.o

objs/term.o: src/term.c src/term.h src/ascii.h objs
	gcc -c src/term.c -ggdb -o objs/term.o

objs: 
	mkdir objs

raylib:
	sudo pacman --needed -S alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama
	[ -d raylib ] || git clone --depth 1 https://github.com/raysan5/raylib.git raylib
	make PLATFORM=PLATFORM_DESKTOP -C raylib/src/ # To make the static version.


documentation:
	wget https://gist.githubusercontent.com/justinmk/a5102f9a0c1810437885a04a07ef0a91/raw/b66b17a605590afbd725c092b0cdc58e15d461a2/XTerm-Control-Sequences.txt
