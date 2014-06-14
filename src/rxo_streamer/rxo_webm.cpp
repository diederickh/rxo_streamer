#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <rxo_streamer/rxo_webm.hpp>
#include <webmtools/webm_live_muxer.h>

using namespace webm_tools;

/* ----------------------------------------------- */

static void on_packet(rxo_vpx* vpx, const vpx_codec_cx_pkt_t* pkt, int64_t pts);
static void on_chunk(rxo_webm* webm, uint8_t* data, uint32_t nbytes);

/* ----------------------------------------------- */

int rxo_webm_init(rxo_webm* w, rxo_info* info) {

  if (!w) { return -1; } 
  if (!info) { return -2; } 

  if (rxo_vpx_init(&w->vpx, info) < 0) {
    return -3;
  }

  w->vpx.on_packet = on_packet;
  w->vpx.user = w;
  w->time_started = uv_hrtime();

  /* create webm muxer */
  w->muxer = (void*)new WebMLiveMuxer();
  if (!w->muxer) {
    printf("Error: cannot create webm muxer.\n");
    return -4;
  }

  /* initialize muxer */
  WebMLiveMuxer* wm = (WebMLiveMuxer*)w->muxer;
  if (wm->Init() != WebMLiveMuxer::kSuccess) {
    printf("Error: cannot init live webm muxer.\n");
    delete wm;
    w->muxer = NULL;
    return -5;
  }

  /* add the video track */
  w->tracknum = wm->AddVideoTrack(info->width, info->height); // , "webm");
  if (w->tracknum <= 0) {
    printf("Error: cannot add video track.\n");
    delete wm;
    w->muxer = NULL;
    return -6;
  }

  return 0;
}

int rxo_webm_encode(rxo_webm* w, uint8_t* data, uint32_t nbytes) {

#if !defined(NDEBUG)
  if (!w) { return -1; }
  if (!data) { return -2; } 
  if (!nbytes) { return -3; } 
#endif
  
  uint64_t time_ns = uv_hrtime() - w->time_started;

  return rxo_vpx_encode(&w->vpx, data, nbytes, time_ns);
}

int rxo_webm_encode_planes(rxo_webm* w
                           ,uint8_t* y, int ystride
                           ,uint8_t* u, int ustride
                           ,uint8_t* v, int vstride)
{


  uint64_t time_ns = uv_hrtime() - w->time_started;
  return rxo_vpx_encode_planes(&w->vpx, y, ystride, u, ustride, v, vstride, time_ns);
}

/* ----------------------------------------------- */

void on_packet(rxo_vpx* vpx, const vpx_codec_cx_pkt_t* pkt, int64_t pts) {
  int r;
  int is_key;
  rxo_webm* webm;
  WebMLiveMuxer* muxer;

  is_key = pkt->data.frame.flags & VPX_FRAME_IS_KEY;
  webm = (rxo_webm*)vpx->user;
  muxer = (WebMLiveMuxer*)webm->muxer;

  if (!muxer) {
    return;
  }

  r = muxer->WriteVideoFrame((uint8_t*)pkt->data.frame.buf, 
                             (size_t)pkt->data.frame.sz, 
                             pts,
                             //pkt->data.frame.pts,
                             (bool)is_key);

  if (r != WebMLiveMuxer::kSuccess) {
    printf("Error: cannot write video frame to webm muxer: %d\n", r);
    exit(1);
    return;
  }

  webm->chunk_size = 0;
  
  r = muxer->Finalize();
  while (muxer->ChunkReady(&webm->chunk_size)) {

    if (webm->chunk_size > RXO_WEBM_BUFFER_SIZE) {
      printf("Error: chunk size to big. We need a bigger buffer.\n");
      ::exit(1);
    }

    r = muxer->ReadChunk(RXO_WEBM_BUFFER_SIZE, webm->chunk_buffer);
    if (r != WebMLiveMuxer::kSuccess) {
      printf("Error: reading a webm chunk failed.\n");
      ::exit(1);
    }

    if (webm->on_chunk) {
      webm->on_chunk(webm, webm->chunk_buffer, webm->chunk_size); 
    }
  }
}
