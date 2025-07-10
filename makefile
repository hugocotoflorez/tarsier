tarsier: raylib tarsier.o term.o
	gcc tarsier.o term.o -Lraylib/src/ -lraylib -Wall -Wextra -o tarsier -lm -ggdb

tarsier.o: tarsier.c term.h ascii.h
	gcc -c tarsier.c -ggdb

term.o: term.c term.h ascii.h
	gcc -c term.c -ggdb

raylib:
	sudo pacman --needed -S alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama
	[ -d raylib ] || git clone --depth 1 https://github.com/raysan5/raylib.git raylib
	make PLATFORM=PLATFORM_DESKTOP -C raylib/src/ # To make the static version.


documentation:
	wget https://gist.githubusercontent.com/justinmk/a5102f9a0c1810437885a04a07ef0a91/raw/b66b17a605590afbd725c092b0cdc58e15d461a2/XTerm-Control-Sequences.txt
