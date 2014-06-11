#ifndef RXO_VPX_H
#define RXO_VPX_H

#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>
#include <rxo_streamer/rxo_types.h>
#include <stdint.h>
#define vpx_cx_interface (vpx_codec_vp8_cx())

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct rxo_vpx rxo_vpx;
typedef void(*rxo_vpx_callback)(rxo_vpx* w, const vpx_codec_cx_pkt_t* pkt, int64_t pts);

struct rxo_vpx {
  vpx_image_t img;
  vpx_codec_enc_cfg_t cfg;
  vpx_codec_ctx_t ctx;
  unsigned long frame_duration;
  int flags;
  int width;
  int height;
  uint64_t num_frames;

  void* user;
  rxo_vpx_callback on_packet;
};

int rxo_vpx_init(rxo_vpx* w, rxo_info* info);
int rxo_vpx_encode(rxo_vpx* w, unsigned char* pixels, uint32_t nbytes, int64_t pts);

#if defined(__cplusplus)
};
#endif

#endif
