#include <stdio.h>
#include <rxo_streamer/rxo_vpx.h>

/* ----------------------------------------------------------------- */

static int vpx_encode(rxo_vpx* vpx, int64_t pts);

/* ----------------------------------------------------------------- */

int rxo_vpx_init(rxo_vpx* enc, rxo_info* info) {

  vpx_codec_err_t err;

  if (!enc)    { return -1; } 
  if (!info) { return -2; } 


  err = vpx_codec_enc_config_default(vpx_cx_interface, &enc->cfg, 0);
  if (err) {
    printf("Error: cannot init the encoder.\n");
    return -3;
  }

  printf("w: %d\n", info->width);
  enc->cfg.rc_target_bitrate = 300;
  enc->cfg.g_w = info->width;
  enc->cfg.g_h = info->height;
  enc->cfg.g_timebase.num = 1; 
  enc->cfg.g_timebase.den = 1000;
  enc->cfg.g_pass = VPX_RC_ONE_PASS;
  enc->cfg.g_error_resilient = 1;
  enc->cfg.kf_mode = VPX_KF_AUTO;
  enc->cfg.g_lag_in_frames = 0;
  enc->cfg.rc_dropframe_thresh = 1;
  enc->cfg.rc_end_usage = VPX_CBR;

  err = vpx_codec_enc_init(&enc->ctx, vpx_cx_interface, &enc->cfg, 0);
  if (err) {
    printf("Error: cannot init VPX encoder: %d\n", err);
    return -4;
  }

  enc->width = info->width;
  enc->height = info->height;
  enc->flags = 0;
  enc->frame_duration = ((double) 1.0 / info->fps_denominator) / ((double) enc->cfg.g_timebase.num / enc->cfg.g_timebase.den);
  enc->num_frames = 0;

  return 0;
}

int rxo_vpx_encode(rxo_vpx* w, unsigned char* pixels, uint32_t nbytes, int64_t pts) {

  vpx_codec_err_t err;
  vpx_image_t* img = NULL;
  vpx_codec_iter_t iter = NULL;
  const vpx_codec_cx_pkt_t* pkt;

#if !defined(NDEBUG)
  if (!w)      { return -1; } 
  if (!pixels) { return -2; }
  if (!nbytes) { return -3; } 
#endif  

  img = vpx_img_wrap(&w->img, VPX_IMG_FMT_I420, w->width, w->height, 1, pixels);
  if (!img) {
    printf("Error: cannot wrap the image.\n");
    return -3;
  }

  if (w->num_frames % 25 == 0) {
    w->flags = VPX_EFLAG_FORCE_KF;
  }


  /* - tmp -  */
#if 0
  printf("img.w: %d\n", img->w);
  printf("img.h: %d\n", img->h);
  printf("img.d_w: %d\n", img->d_w);
  printf("img.d_h: %d\n", img->d_h);
  printf("img.bps: %d\n", img->bps);
  printf("img.x_chroma_shift: %d\n", img->x_chroma_shift);
  printf("img.y_chroma_shift: %d\n", img->y_chroma_shift);
  printf("img.self_allocd: %d\n", img->self_allocd);
  printf("img.stride[0]: %d\n", img->stride[0]);
  printf("img.stride[1]: %d\n", img->stride[1]);
  printf("img.stride[2]: %d\n", img->stride[2]);
#endif
  /* - tmp - */


  err = vpx_codec_encode(&w->ctx, img, pts, w->frame_duration, w->flags, VPX_DL_REALTIME);
  if (err) {
    printf("Error: while encoding.\n");
    return -4;
  }
  
  while ( (pkt = vpx_codec_get_cx_data(&w->ctx, &iter)) ) {
    if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
      w->on_packet(w, pkt, pts);
    }
  }

  w->flags = 0;
  w->num_frames++;

  return 0;
}

int rxo_vpx_encode_planes(rxo_vpx* w
                          ,uint8_t* y, int ystride
                          ,uint8_t* u, int ustride
                          ,uint8_t* v, int vstride
                          ,int64_t pts)
{

#if !defined(NDEBUG)
  if (!w) { return -1; } 
  if (!y) { return -2; } 
  if (!u) { return -3; } 
  if (!v) { return -4; } 
#endif

  w->img.w              = w->width;
  w->img.h              = w->height;
  w->img.d_w            = w->width;
  w->img.d_h            = w->height;
  w->img.fmt            = VPX_IMG_FMT_I420;
  w->img.planes[0]      = y;
  w->img.stride[0]      = ystride;
  w->img.planes[1]      = u;
  w->img.stride[1]      = ustride;
  w->img.planes[2]      = v;
  w->img.stride[2]      = vstride;
  w->img.x_chroma_shift = 1;
  w->img.y_chroma_shift = 1;
  w->img.bps            = 12;
  
  return vpx_encode(w, pts);
}

/* ----------------------------------------------------------------- */

static int vpx_encode(rxo_vpx* vpx, int64_t pts) {
  int r = 0;
  vpx_codec_err_t err;
  vpx_codec_iter_t iter = NULL;
  const vpx_codec_cx_pkt_t* pkt;

  if (vpx->num_frames % 25 == 0) {
    vpx->flags = VPX_EFLAG_FORCE_KF;
  }

  err = vpx_codec_encode(&vpx->ctx, &vpx->img, pts, vpx->frame_duration, vpx->flags, VPX_DL_REALTIME);
  if (err) {
    printf("Error: while encoding.\n");
    return -4;
  }
  
  while ( (pkt = vpx_codec_get_cx_data(&vpx->ctx, &iter)) ) {
    if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
      vpx->on_packet(vpx, pkt, pts);
    }
  }

  vpx->flags = 0;
  vpx->num_frames++;

  return 0;
}
