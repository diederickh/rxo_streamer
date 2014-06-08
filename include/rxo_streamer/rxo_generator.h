/*

  rxo_generator
  -------------

  Creates a test video signal with 7 colors vertical bars
  and one moving horizontal. First initialize the generator using
  `rxo_generator_init()` then call `rxo_generator_update()`. Each time you
  call `rxo_generator_update()` it will update the frame and y, u, v planes.
  When ready clean memory using `rxo_generator_clear()`.
  
  You can write the frames into a file and use avconv to encode it to some format:

  ````sh
  ./avconv -f rawvideo -pix_fmt yuv420p -s 640x480 -i output.yuv -vcodec h264 -r 25 -y out.mov
  ````

  TODO
  -----
  - There are many things we could optimise.
  
  Example: 
  --------
  <example>

     fp = fopen("output.yuv", "wb");

     rxo_generator gen;

     if (rxo_generator_init(&gen, WIDTH, HEIGHT, FPS) < 0) {
       printf("Error: cannot initialize the generator.\n");
       exit(1);
     }

     while(1) {

        printf("Frame: %llu\n", gen.frame);

        rxo_generator_update(&gen);

        // write planes to a file
        fwrite((char*)gen.y, gen.ybytes,1,  fp);
        fwrite((char*)gen.u, gen.ubytes,1, fp);
        fwrite((char*)gen.v, gen.vbytes,1, fp);

        if (gen.frame > 250) { 
          break;
        }

        usleep(gen.fps);
     }

    fclose(fp);

    rxo_generator_clear(&gen);

  </example>
 */

#ifndef RXO_GENERATOR_H
#define RXO_GENERATOR_H

#define RXS_MAX_CHARS 11
#include <stdint.h>

typedef struct rxo_generator rxo_generator;
typedef struct rxo_generator_char rxo_generator_char;

struct rxo_generator_char {
  int id;
  int x;
  int y;
  int width;
  int height;
  int xoffset;
  int yoffset;
  int xadvance;
};

struct rxo_generator {
  uint64_t frame;
  uint8_t* y;
  uint8_t* u;
  uint8_t* v;
  uint32_t width;
  uint32_t height;
  uint32_t ybytes;
  uint32_t ubytes;
  uint32_t vbytes;
  uint32_t nbytes;
  int fps_num;
  int fps_den;
  double fps;                               /* in microseconds, 1 fps == 1.000.000 us */
  double step;                              /* used to create the moving bar */
  double perc;                              /* position of the moving bar */
  rxo_generator_char chars[RXS_MAX_CHARS];  /*  bitmap characters, `0-9` and `:` */
  int font_w;
  int font_h;
  int font_line_height;
};

int rxo_generator_init(rxo_generator* g, int width, int height, int fps);
int rxo_generator_update(rxo_generator* g);
int rxo_generator_clear(rxo_generator* g);

#endif
