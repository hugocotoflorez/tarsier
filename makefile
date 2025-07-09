tarsier: raylib tarsier.o term.o
	gcc tarsier.o term.o -Lraylib/src/ -lraylib -Wall -Wextra -o tarsier -lm

tarsier.o: tarsier.c term.h ascii.h
	gcc -c tarsier.c

term.o: term.c term.h ascii.h
	gcc -c term.c

raylib:
	sudo pacman --needed -S alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama
	[ -d raylib ] || git clone --depth 1 https://github.com/raysan5/raylib.git raylib
	make PLATFORM=PLATFORM_DESKTOP -C raylib/src/ # To make the static version.
