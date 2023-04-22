#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "dos.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

int main(int argc, char* args[]) {
  setvideomode(videomode_320x200);

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

    // copy everything from the back buffer to the front buffer
    framebuffer = swapbuffers();

    if(keystate(KEY_ESCAPE)) {
      break;
    }
  }

  // i'm a modern c programmer, i return EXIT_SUCCESS instead of 0
  return EXIT_SUCCESS;
}