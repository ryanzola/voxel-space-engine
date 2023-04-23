#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "dos.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCALE_FACTOR 100
#define MAX_VELOCITY 4.0

typedef struct {
  float x;      // x position
  float y;      // y position
  float height; // height of the camera
  float angle;  // angle of the camera (radians, clockwise)
  float horizon; // offset the horizon position (look up/down)
  float velocity; // velocity of the camera
  float tilt;   // tilt the camera left/right
  float zfar;   // distance of the camera looking forward
} camera_t;

camera_t camera = { 
  .x = 512.0, 
  .y = 512.0,
  .height = 150.0,
  .angle   = 1.5 * 3.141592, // (= 270 deg)
  .horizon = 100.0,
  .velocity = 0.0,
  .tilt = 0.0,
  .zfar = 600.0
};

// buffers for hight map and color map
uint8_t* heightmap = NULL;    // buffer/array to hold heighmap information 1024x1024
uint8_t* colormap = NULL;     // buffer/array to hold color information 1024x1024

// lerp function
// a = start value
// b = end value
// t = time
float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

void process_input() {
  if(keystate(KEY_UP)) {
    // increment velocity
    camera.velocity = lerp(camera.velocity, MAX_VELOCITY, 0.01);

    // cap the velocity
    if(camera.velocity > MAX_VELOCITY) {
      camera.velocity = MAX_VELOCITY;
    }
  }
  if(keystate(KEY_DOWN)) {
    // increment velocity
    camera.velocity = lerp(camera.velocity, -MAX_VELOCITY, 0.01);

    // cap the velocity
    if(camera.velocity < -MAX_VELOCITY) {
      camera.velocity = -MAX_VELOCITY;
    }
  }
  if(keystate(KEY_LEFT)) {
    camera.angle -= 0.01;
    if(camera.tilt < 1.0) {
      // lerp from 0 to 1.0
      camera.tilt = lerp(camera.tilt, 1.0, 0.1);
    }
  }
  if(keystate(KEY_RIGHT)) {
    camera.angle += 0.01;
    if(camera.tilt > -1.0) {
      // lerp from 0 to -1.0
      camera.tilt = lerp(camera.tilt, -1.0, 0.1);
    }
  }
  if(keystate(KEY_E)) {
    camera.height++;
  }
  if(keystate(KEY_D)) {
    camera.height--;
  }
  if(keystate(KEY_Q)) {
    camera.horizon += 1.5f;
  }
  if(keystate(KEY_W)) {
    camera.horizon -= 1.5f;
  }

    // apply velocity to camera position
    camera.x += cos(camera.angle) * camera.velocity;
    camera.y += sin(camera.angle) * camera.velocity;

  // tilt the camera back to 0
  if(!keystate(KEY_LEFT) && !keystate(KEY_RIGHT)) {
    camera.tilt = lerp(camera.tilt, 0.0, 0.1);
  }

  // apply friction to the velocity
  if(!keystate(KEY_UP) && !keystate(KEY_DOWN)) {
      if(camera.velocity == 0.0) {
        return;
      }

      // slow down the camera velocity until it reaches 0
      camera.velocity = lerp(camera.velocity, 0.0, 0.001);
  }
}

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

    // process input
    process_input();

    float sinangle = sin(camera.angle);
    float cosangle = cos(camera.angle);

    // voxels space magic
    float plx = cosangle * camera.zfar + sinangle * camera.zfar;
    float ply = sinangle * camera.zfar - cosangle * camera.zfar; 

    float prx = cosangle * camera.zfar - sinangle * camera.zfar;
    float pry = sinangle * camera.zfar + cosangle * camera.zfar;

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
        ray_y += delta_y;

        // find the offset to translate to and fetch the heightmap value
        int mapoffset = ((1024 * ((int)ray_y & 1023)) + ((int)ray_x & 1023));

        // project the height and find the height on the screen
        // perspective divide: projection = height / z
        int proj_height = (int)((camera.height - heightmap[mapoffset]) / z * SCALE_FACTOR + camera.horizon);

        if(proj_height < 0) proj_height = 0;
        if(proj_height > SCREEN_HEIGHT) proj_height = SCREEN_HEIGHT - 1;

        // only render the terrain pixels if the new projected height is taller than the previous max height
        if(proj_height < max_height) {
          // compute a "lean" offset to simulate rolling the camera left and right
          float lean = (camera.tilt * (i / (float)SCREEN_WIDTH - 0.5) + 0.5) * SCREEN_HEIGHT / 6;

          // draw pixels from previous max height until the new max height
          for(int y = (proj_height + lean); y < (max_height + lean); y++) {
            if(y >= 0) {
              framebuffer[(SCREEN_WIDTH * y) + i] = (uint8_t)colormap[mapoffset];
            }
          }

          max_height = proj_height;
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