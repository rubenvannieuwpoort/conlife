.PHONY: clean

main: main.c image.c image.h display.c display.h
	gcc main.c image.c display.c -o main -O3 -lSDL2 -lm

clean:
	rm -rf main

