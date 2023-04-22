#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "dos.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

// buffers for hight map and color map
uint8_t* heightmap = NULL;    // buffer/array to hold heighmap information 1024x1024
uint8_t* colormap = NULL;     // buffer/array to hold color information 1024x1024

int main(int argc, char* args[]) {
  setvideomode(videomode_320x200);

  // - load the gif files
  // - load the colormap and heightmap buffers
  uint8_t palette[256 * 3];
  int map_width, map_height, pal_count;
  colormap = loadgif("maps/gif/map00.color.gif", &map_width, &map_height, &pal_count, palette);
  heightmap = loadgif("maps/gif/map00.height.gif", &map_width, &map_height, NULL, NULL);

  for(int i = 0; i < pal_count; i++) {
    setpal(i, palette[3 * i + 0], palette[3 * i + 1], palette[3 * i + 2]);
  }

  setpal(0, 36, 36, 56);

  setdoublebuffer(1);

  // create a frame buffer.
  // screenbuffer returns a frame buffer in memeory.
  // pointer to a buffer in memory that has all the pixel colors of the screen.
  // each position is 8 bits.
  // 320x200 = 64000 pixels
  uint8_t* framebuffer = screenbuffer();

  // game loop
  while(!shuttingdown()) {
    // wait for a blank frame
    waitvbl();

    // clear the screen
    clearscreen();

    // TODO:
    // - processinput()
    // - update()
    // - render()

    int x = 160;
    int y = 100;
    framebuffer[(SCREEN_WIDTH * y) + x] = 0x50;

    // copy everything from the back buffer to the front buffer
    framebuffer = swapbuffers();

    if(keystate(KEY_ESCAPE)) {
      break;
    }
  }

  // i'm a modern c programmer, i return EXIT_SUCCESS instead of 0
  return EXIT_SUCCESS;
}