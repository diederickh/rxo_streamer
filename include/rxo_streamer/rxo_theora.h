/*

  rxo_theora
  ----------

  Basic theora encoder.

 */

#ifndef RXO_THEORA_H
#define RXO_THEORA_H

#include <rxo_streamer/rxo_types.h>
#include <theora/theoraenc.h>
#include <stdint.h>

typedef struct rxo_theora rxo_theora;
typedef void(*rxo_theora_callback)(rxo_theora* th, uint8_t* data, uint32_t nbytes);

struct rxo_theora {

  /* theora */
  th_info info;
  th_comment comment;
  th_enc_ctx* ctx;
  th_ycbcr_buffer yuv;

  /* ogg */
  ogg_stream_state ostream;
  ogg_page opage;
  ogg_packet opacket;

  /* callback */
  void* user;
  rxo_theora_callback on_data;
};

int rxo_theora_init(rxo_theora* th, rxo_info* info);
int rxo_theora_add_frame(rxo_theora* th, uint8_t* data, uint32_t nbytes);

#endif
