#include <stdio.h>
#include <rxo_streamer/rxo_theora.h>

int rxo_theora_init(rxo_theora* th, rxo_info* info) {
  int r;
  int w, h;

  if (!th){ return -1; } 

  th_info_init(&th->info);

  //  th->frame_num = 0;

  /* setup info */
  w = info->width;
  h = info->height;
  th->info.frame_width = w;
  th->info.frame_height = h;

  th->info.pic_width = w;
  th->info.pic_height = h;
  th->info.pic_x = 0;
  th->info.pic_y = 0;
  th->info.fps_numerator = info->fps_numerator;
  th->info.fps_denominator = info->fps_denominator;
  th->info.aspect_numerator = 1;
  th->info.aspect_denominator = 1;
  th->info.colorspace = TH_CS_UNSPECIFIED;
  th->info.quality = info->quality;     /* @todo - give meaningful value */
  th->info.pixel_fmt = TH_PF_420;       /* @todo - let user pass pixel format */

  /* allocate encoder context */
  th->ctx = th_encode_alloc(&th->info);

  th_info_clear(&th->info);

  if (!th->ctx) {
    printf("Error: cannot create encoder.\n");
    return -2;
  }

  /* init ogg stream */
  r = ogg_stream_init(&th->ostream, rand());
  if (r < 0) {
    printf("Error: cannot init ogg stream.\n");
    /* @todo - leaking here, need to cleeanup. */
    return -3;
  }

  /* @todo - set keyframe frequency */
  th_comment_init(&th->comment);
  th_comment_add(&th->comment, (char *)"roxlu");
  th->comment.vendor = (char *)"roxlu";


  /* write out headers */
  for (;;) {
    r = th_encode_flushheader(th->ctx, &th->comment, &th->opacket); // &oggpacket);
    if (r < 0) { 
      printf("Error: cannot flush header.\n");
      return -5; 
    }
    if (r == 0) { 
      break;
    }
    
    /* should I use ogg_stream_packetin() here? */
    ogg_stream_packetin(&th->ostream, &th->opacket); 
    while (ogg_stream_pageout(&th->ostream, &th->opage)) { 
      th->on_data(th, th->opage.header, th->opage.header_len);
      th->on_data(th, th->opage.body, th->opage.body_len);
    }
  }

  /* flush out headers */
  while (ogg_stream_flush(&th->ostream, &th->opage) > 0) {
      th->on_data(th, th->opage.header, th->opage.header_len);
      th->on_data(th, th->opage.body, th->opage.body_len);
  }

  /* init image buffer */
  /* @todo - handle different yuv formats */
  th->yuv[0].width  = w;
  th->yuv[0].height = h;
  th->yuv[0].stride = w;
  th->yuv[1].width  = w / 2;
  th->yuv[1].height = h / 2;
  th->yuv[1].stride = w / 2;
  th->yuv[2].width  = w / 2;
  th->yuv[2].height = h / 2;
  th->yuv[2].stride = w / 2;

  return 0;
}

int rxo_theora_add_frame(rxo_theora* th, uint8_t* data, uint32_t nbytes) {

  int r;

#if !defined(NDEBUG)
  if (!th)     { return -1; } 
  if (!data)   { return -2; } 
  if (!nbytes) { return -3; } 
#endif

  /* @todo - handle different pixel formats */

  /* set pointers */
  th->yuv[0].data = data;
  th->yuv[1].data = th->yuv[0].data + (th->yuv[0].width * th->yuv[0].height);
  th->yuv[2].data = th->yuv[1].data + (th->yuv[1].width * th->yuv[1].height);

  /* encode the yuv */
  r = th_encode_ycbcr_in(th->ctx, th->yuv);
  if (r != 0) {
    printf("Error: cannot encode frame.\n");
    return -4;
  }

  while (th_encode_packetout(th->ctx, 0, &th->opacket) != 0) {
    ogg_stream_packetin(&th->ostream, &th->opacket);
    while(ogg_stream_pageout(&th->ostream, &th->opage)) {
      th->on_data(th, th->opage.header, th->opage.header_len);
      th->on_data(th, th->opage.body, th->opage.body_len);
    }
  }
  return 0;
}


