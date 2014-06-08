
#include <videocapture/Capture.h>

extern "C" {
#  include <stdlib.h>
#  include <stdio.h>
#  include <signal.h>
#  include <rxo_streamer/rxo_streamer.h>
#  include <libyuv.h>
}

#define DEVICE 0
#define WIDTH 640
#define HEIGHT 480
#define FPS 30

using namespace ca; 

static void on_webcam_frame(void* pixels, int nbytes, void* user);
static int init_webcam(int device, int w, int h);
static int init_yuv();
static int shutdown_webcam();
static int get_secondary_cap_fmt(int curr);
static void sighandler(int s);

rxo_info info;
rxo_streamer streamer;
Capture* cap = NULL;
uint8_t* yuv = NULL;
uint8_t* u   = NULL;
uint8_t* v   = NULL;
int cap_id   = 0;
int cap_fmt  = CA_UYVY422;

int main() {

  printf("\n\ntest_webcam\n");

  signal(SIGINT, sighandler);

  if (init_yuv() < 0) {
    printf("Error: cannot init yuv.\n");
    exit(1);
  }

  /* initialize the streamer */
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

  if (rxo_streamer_init(&streamer, &info) < 0) {
    printf("Error: cannot initialize the streamer.\n");
    exit(1);
  }

  /* initialize the webcam. */
  if (init_webcam(DEVICE, WIDTH, HEIGHT) < 0) {
    printf("Error: cannot init webcam device: %d  at: %dx%d\n", DEVICE, WIDTH, HEIGHT);
    exit(1);
  }

  while (1) {
    cap->update();
  }

  /* shutdown webcam */
  if (shutdown_webcam() < 0) {
    printf("Error: cannot cleanly shutdown the webcam.\n");
    exit(1);
  }
 
  /* @todo > create + call rxo_streamer_clear() */

  if (yuv) {
    free(yuv);
    yuv = NULL;
  }
  
  return 0;
}

static int init_webcam(int device, int w, int h) {
  
  if (device < 0) { return -1; } 
  if (!w)         { return -2; } 
  if (!h)         { return -3; } 
  if (cap)        { return -4; } 

  cap = new Capture(on_webcam_frame, NULL);
  if (!cap) {
    return -5;
  }

  if (cap->listDevices() <= 0) {
    return -6;
  }

  cap_id = cap->findCapability(device, w, h, cap_fmt);
  if (cap_id < 0) {

    /* test other format */
    cap_fmt = get_secondary_cap_fmt(cap_fmt);
    if (cap_fmt < 0) {
      return -7;
    }
    
    cap_id = cap->findCapability(device, w, h, cap_fmt);
    if (cap_id < 0) {
      return -8;
    }
  }

  /* open device */
  Settings set;
  set.device = device;
  set.capability = cap_id;
  set.format = -1;

  if (cap->open(set) < 0) {
    return -9;
  }

  if (cap->start() < 0) {
    return -10;
  }

  return 0;
}

static int shutdown_webcam() {
  if (!cap) { return -1; }
  cap->stop();
  cap->close();
}


/* get "the other" format */
static int get_secondary_cap_fmt(int curr) {
  if (curr == CA_UYVY422) {
    return CA_YUYV422;
  }
  else if (curr == CA_YUYV422) {
    return CA_UYVY422;
  }
  else {
    return -1;
  }
}

static void on_webcam_frame(void* pixels, int nbytes, void* user) {

  int r = 0;

  /* convert to 420p */
  if (cap_fmt == CA_UYVY422) {
    r = libyuv::UYVYToI420((const uint8*)pixels,  WIDTH * 2, 
                           (uint8*)yuv, WIDTH, 
                           (uint8*)u,   WIDTH / 2, 
                           (uint8*)v,   WIDTH / 2, 
                           WIDTH, HEIGHT);
    if (r != 0) {
      printf("Error: cannot convert to I420.\n");
      exit(1);
    }

    rxo_streamer_add_frame(&streamer, yuv, nbytes);

  }
  else if (cap_fmt == CA_YUYV422) {
    r = libyuv::YUY2ToI420((const uint8*)pixels,  WIDTH * 2, 
                           (uint8*)yuv, WIDTH, 
                           (uint8*)u,   WIDTH / 2, 
                           (uint8*)v,   WIDTH / 2, 
                           WIDTH, HEIGHT);
    if (r != 0) {
      printf("Error: cannot convert to I420.\n");
      exit(1);
    }

    rxo_streamer_add_frame(&streamer, yuv, nbytes);
  }
  else {
    printf("Error: unsupported pixel format from webcam.\n");
    exit(1);
  }
}

static void sighandler(int s) {
  shutdown_webcam();
  exit(1);
}

static int init_yuv() {

  yuv = (uint8_t*)malloc(WIDTH * HEIGHT * 2);
  if (!yuv) {
    return -1;
  }

  u = yuv + WIDTH * HEIGHT;
  v = u + (WIDTH * HEIGHT) / 4;

  return 0;
}
