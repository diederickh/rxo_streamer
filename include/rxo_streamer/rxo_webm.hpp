#ifndef RXO_WEBM_H
#define RXO_WEBM_H

#include <rxo_streamer/rxo_types.h>
#include <rxo_streamer/rxo_vpx.h>
#include <stdint.h>

#define vpx_cx_interface (vpx_codec_vp8_cx())
#define RXO_WEBM_BUFFER_SIZE (1024 * 1024 * 4)

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct rxo_webm rxo_webm;
typedef void(*rxo_webm_callback)(rxo_webm* w, uint8_t* data, uint32_t nbytes);

struct rxo_webm {
  rxo_vpx vpx;

  void* muxer;
  int tracknum;
  uint64_t time_started;
  int32_t chunk_size;
  uint8_t chunk_buffer[RXO_WEBM_BUFFER_SIZE];

  void* user;
  rxo_webm_callback on_chunk; /* gets called when a webm chunk has been created; the passed data will contain a webm chunk */
};

int rxo_webm_init(rxo_webm* w, rxo_info* info);
int rxo_webm_encode(rxo_webm* w, uint8_t* data, uint32_t nbytes);

#if defined(__cplusplus)
};
#endif

#endif
