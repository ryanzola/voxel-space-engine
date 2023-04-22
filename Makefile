CFLAGS = -Wall -Wfatal-errors -std=c99 -I/usr/local/include -I/opt/homebrew/Cellar/glew/2.2.0_1/include
LDFLAGS = -L/opt/homebrew/Cellar/glew/2.2.0_1/lib -lGLEW -framework OpenGL

.PHONY: build run clean

build:
	gcc $(CFLAGS) *.c `sdl2-config --libs --cflags` $(LDFLAGS) -lm -lpthread -o voxel

run:
	./voxel

clean:
	rm voxel
