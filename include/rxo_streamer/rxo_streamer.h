#ifndef RXO_STREAMER_H
#define RXO_STREAMER_H

#include <rxo_streamer/rxo_types.h>
#include <rxo_streamer/rxo_theora.h>
#include <rxo_streamer/rxo_webm.hpp>
#include <shout/shout.h>
#include <stdint.h>

typedef struct rxo_streamer rxo_streamer;

typedef void(*rxo_streamer_callback)(rxo_streamer* rxo, uint8_t* data, uint32_t nbytes);

struct rxo_streamer {
  int mode;                           /* RXO_WEBM or RXO_OGG mode */
  rxo_theora theora;                  /* the video encoder */
  rxo_webm webm;                      /* webm muxer */
  shout_t* shout;                     /* our shout handler that sends data to icecast */
  void* user;                         /* gets passed into on_data */
  rxo_streamer_callback on_data;      /* gets called when we have some data */
  uint64_t bytes_sent;                /* used to display the avarage bitrate */
  uint64_t time_started;              /* when we received the first data to sent */
};

int rxo_streamer_init(rxo_streamer* rxo, rxo_info* info);
int rxo_streamer_add_frame(rxo_streamer* rxo, uint8_t* pixels, uint32_t nbytes);
int rxo_streamer_add_planes(rxo_streamer* rxo, uint8_t* y, int ystride, uint8_t* u, int ustride, uint8_t* v, int vstride);

#endif
