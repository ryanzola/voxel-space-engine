#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "dos.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

typedef struct {
  float x;    // x position
  float y;    // y position
  float zfar; // distance of the camera looking forward
} camera_t;

camera_t camera = { 
  .x = 512, 
  .y = 512, 
  .zfar = 400
};

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

    // voxels space magic
    float plx = -camera.zfar;
    float ply = camera.zfar; 

    float prx = camera.zfar;
    float pry = camera.zfar;

    // loop 320 times to find the 320 rays
    // left to right
    for(int i = 0; i < SCREEN_WIDTH; i++) {
      float delta_x = (plx + (prx - plx) / SCREEN_WIDTH * i) / camera.zfar;
      float delta_y = (ply + (pry - ply) / SCREEN_WIDTH * i) / camera.zfar;

      float ray_x = camera.x;
      float ray_y = camera.y;

      float max_height = SCREEN_HEIGHT;

      for(int z = 1; z < camera.zfar; z++) {
        ray_x += delta_x;
        ray_y -= delta_y;

        // find the offset to translate to and fetch the heightmap value
        int mapoffset = (1024 * (int)ray_y) + (int)ray_x;

        // project the height and find the height on the screen
        // perspective divide: projection = height / z
        int heightonscreen = (int)((100.0 - heightmap[mapoffset]) / z * 100.0);

        if(heightonscreen < 0) heightonscreen = 0;
        if(heightonscreen > SCREEN_HEIGHT) heightonscreen = SCREEN_HEIGHT - 1;

        // only render the terrain pixels if the new projected height is taller than the previous max height
        if(heightonscreen < max_height) {
          // draw pixels from previous max height until the new max height
          for(int y = heightonscreen; y < max_height; y++) {
            framebuffer[(SCREEN_WIDTH * y) + i] = (uint8_t)colormap[mapoffset];
          }

          max_height = heightonscreen;
        }
      }
    }

    // copy everything from the back buffer to the front buffer
    framebuffer = swapbuffers();

    if(keystate(KEY_ESCAPE)) {
      break;
    }
  }

  // i'm a modern c programmer, i return EXIT_SUCCESS instead of 0
  return EXIT_SUCCESS;
}