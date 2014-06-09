//#include <uv.h>
#include <stdio.h>
#include <rxo_streamer/rxo_webm.hpp>

/* ----------------------------------------------- */

static void on_packet(rxo_vpx* vpx, const vpx_codec_cx_pkt_t* pkt, int64_t pts);

/* ----------------------------------------------- */

int rxo_webm_init(rxo_webm* w, rxo_info* info) {

  if (!w) { return -1; } 
  if (!info) { return -2; } 

  if (rxo_vpx_init(&w->vpx, info) < 0) {
    return -3;
  }

  w->vpx.on_packet = on_packet;
  w->vpx.user = w;
  //  w->time_started = uv_hrtime();

  return 0;
}



int rxo_webm_add_packet(rxo_webm* w, uint8_t* data, uint32_t nbytes) {

 uint64_t time_ns;

  if (!w) { return -1; }
  if (!data) { return -2; } 
  if (!nbytes) { return -3; } 

  time_ns = uv_hrtime() - w->time_started;
  printf("%lld\n", time_ns);
  
  return rxo_vpx_encode(&w->vpx, data, nbytes, time_ns);
}

/* ----------------------------------------------- */

void on_packet(rxo_vpx* vpx, const vpx_codec_cx_pkt_t* pkt, int64_t pts) {


  printf("Got packet\n");
}
