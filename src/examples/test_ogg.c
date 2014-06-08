#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <rxo_streamer/rxo_streamer.h>
#include <rxo_streamer/rxo_generator.h>

#define WIDTH 640
#define HEIGHT 480
#define FPS 25

FILE* fp;
rxo_info info;
rxo_streamer rxo;
rxo_generator gen;

static void sighandler(int s);
static void on_data(rxo_streamer* rxo, uint8_t* data, uint32_t nbytes);


int main() {
  //  printf("\n\ntest_ogg\n\n");

  signal(SIGINT, sighandler);

  fp = fopen("out.ogv", "wb");
  if (!fp) {
    printf("Error: cannot open output file.\n");
    exit(1);
  }

  /* initialize generator */
  if (rxo_generator_init(&gen, WIDTH, HEIGHT, FPS) < 0) {
    exit(1);
  }

  rxo.on_data = on_data;

  /* initialize rxo */
  info.width = WIDTH;
  info.height = HEIGHT;
  info.fps_numerator = FPS;
  info.fps_denominator = 1;
  info.quality = 50;
  info.port = 8000;
  info.host = "127.0.0.1";
  info.user = "source";
  info.password = "hackme";
  info.mount = "/example.ogg";

  if (rxo_streamer_init(&rxo, &info) < 0) {
    exit(1);
  }

  while (1) {
    //  printf("....\n");
    rxo_generator_update(&gen);
    rxo_streamer_add_frame(&rxo, gen.y, gen.nbytes);
    usleep(10000);
  }

//printf("\n");
  return 0;
}

void sighandler(int s) {
  printf("Handling signal.\n");
  fclose(fp);
  exit(1);
}

static void on_data(rxo_streamer* rxo, uint8_t* data, uint32_t nbytes) {
  //printf(">> WRITE: %u\n", nbytes);
  //fwrite((void*)data, nbytes, 1, fp);
  fwrite((void*)data, nbytes, 1, stdout);
  fflush(stdout);
}
