#ifndef RXO_STREAMER_H
#define RXO_STREAMER_H

#include <rxo_streamer/rxo_types.h>
#include <rxo_streamer/rxo_theora.h>
#include <rxo_streamer/rxo_webm.hpp>
#include <shout/shout.h>


//#if defined(__cplusplus)
//extern "C" {
//#endif


typedef struct rxo_streamer rxo_streamer;

typedef void(*rxo_streamer_callback)(rxo_streamer* rxo, uint8_t* data, uint32_t nbytes);

struct rxo_streamer {
  rxo_theora theora;                  /* the video encoder */
  rxo_webm webm;                      /* webm muxer */
  shout_t* shout;                     /* our shout handler that sends data to icecast */
  void* user;                         /* gets passed into on_data */
  rxo_streamer_callback on_data;      /* gets called when we have some data */
};

int rxo_streamer_init(rxo_streamer* rxo, rxo_info* info);
int rxo_streamer_add_frame(rxo_streamer* rxo, uint8_t* pixels, uint32_t nbytes);

//#if defined(__cplusplus)
//};
//#endif


#endif
