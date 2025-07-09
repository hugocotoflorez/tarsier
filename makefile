aa: term.c
	gcc term.c -o aa -ggdb

tarsier: raylib tarsier.o
	gcc tarsier.o -Lraylib/src/ -lraylib -Wall -Wextra -o tarsier -lm

tarsier.o: tarsier.c
	gcc -c tarsier.c

raylib:
	sudo pacman --needed -S alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama
	[ -d raylib ] || git clone --depth 1 https://github.com/raysan5/raylib.git raylib
	make PLATFORM=PLATFORM_DESKTOP -C raylib/src/ # To make the static version.
