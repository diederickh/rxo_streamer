#include <stdio.h>
#include <uv.h>
#include <rxo_streamer/rxo_streamer.h>

/* ----------------------------------------------- */

static void on_theora_data(rxo_theora* th, uint8_t* data, uint32_t nbytes);
static void on_webm_data(rxo_webm* webm, uint8_t* data, uint32_t nbytes);

/* ----------------------------------------------- */

int rxo_streamer_init(rxo_streamer* rxo, rxo_info* info) {

  int r;

  if (!rxo)  { return -1; } 
  if (!info) { return -2; } 

  /* init shout */
  shout_init();

  rxo->shout = shout_new();
  if (!rxo->shout) { 
    printf("Error: cannot init shout.\n");
    /* @todo should clean shout */
    return -3;
  }

  r = shout_set_host(rxo->shout, info->host);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: setting hostname: %s\n", shout_get_error(rxo->shout));
    /* @todo should clean shout */
    return -4;
  }

  r = shout_set_protocol(rxo->shout, SHOUT_PROTOCOL_HTTP);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: setting shout protocol: %s\n", shout_get_error(rxo->shout));
    return -5;
  }

  r = shout_set_port(rxo->shout, info->port);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: setting shout port: %s\n", shout_get_error(rxo->shout));
    return -6;
  }

  r = shout_set_password(rxo->shout, info->password);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: setting shout password: %s\n", shout_get_error(rxo->shout));
    return -7;
  }

  r = shout_set_mount(rxo->shout, info->mount);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: setting shout mount: %s\n", shout_get_error(rxo->shout));
    return -9;
  }

  r = shout_set_user(rxo->shout, info->user);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: setting shout user: %s\n", shout_get_error(rxo->shout));
    return -8;
  }
    
  r = shout_set_format(rxo->shout, (info->mode == RXO_WEBM) ? SHOUT_FORMAT_WEBM : SHOUT_FORMAT_OGG);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: setting shout format: %s\n", shout_get_error(rxo->shout));
    return -10;
  }

  r = shout_open(rxo->shout);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: cannot open shout.\n");
    return -11;
  }

  rxo->mode = info->mode;
  rxo->bytes_sent = 0;
  rxo->time_started = uv_hrtime();

  /* init correct encoder */
  if (info->mode == RXO_OGG) { 

    /* init theora encoder */
    rxo->theora.user = rxo;
    rxo->theora.on_data = on_theora_data;

    if (rxo_theora_init(&rxo->theora, info) < 0) {
      printf("Error: cannot init theora.\n");
      return -2;
    }
  }
  else if(info->mode == RXO_WEBM) {

    /* init webm */
    rxo->webm.user = rxo;
    rxo->webm.on_chunk = on_webm_data;

    if (rxo_webm_init(&rxo->webm, info) < 0) {
      printf("Error: cannot init webm.\n");
      return -3;
    }
  }

  
  return 0;
}

int rxo_streamer_add_frame(rxo_streamer* rxo, uint8_t* pixels, uint32_t nbytes) {

#if !defined(NDEBUG)  
  if (!rxo)    { return -1; } 
  if (!pixels) { return -2; } 
  if (!nbytes) { return -3; } 
#endif

  if (rxo->mode == RXO_OGG) { 
    if (rxo_theora_add_frame(&rxo->theora, pixels, nbytes) < 0) {
      printf("Error: cannot add theora frame.\n");
      return -4;
    };
  }
  else if (rxo->mode == RXO_WEBM) {
    rxo_webm_encode(&rxo->webm, pixels, nbytes);
  }
  else {
    printf("(Error: invalid rxo mode.\n");
    return -4;
  }

  return 0;
}

int rxo_streamer_add_planes(rxo_streamer* rxo 
                            ,uint8_t* y, int ystride 
                            ,uint8_t* u, int ustride
                            ,uint8_t* v, int vstride)
{
  if (rxo->mode == RXO_OGG) { 
    if (rxo_theora_add_planes(&rxo->theora, y, ystride, u, ustride, v, vstride) < 0) {
      printf("Error: cannot add theora frame.\n");
      return -4;
    };
  }
  else if (rxo->mode == RXO_WEBM) {
    if (rxo_webm_encode_planes(&rxo->webm, y, ystride, u, ustride, v, vstride) < 0) {
      printf("Error: cannot encode vpx plane.\n");
    }
  }
  else {
    printf("(Error: invalid rxo mode.\n");
    return -4;
  }
}


/* ----------------------------------------------- */

static void on_theora_data(rxo_theora* th, uint8_t* data, uint32_t nbytes) {

  int r;
  rxo_streamer* rxo = NULL;
  double runtime = 0;

#if !defined(NDEBUG)
  if (!th)     { printf("Error: invalid theora.\n");   exit(1); } 
  if (!data)   { printf("Error: invalid data.\n");     exit(1); } 
  if (!nbytes) { printf("Error: invalid nbytes.\n");   exit(1); } 
#endif

  rxo = (rxo_streamer*)th->user;

  r = shout_send(rxo->shout, data, nbytes);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: sending shout: %s\n", shout_get_error(rxo->shout));
  }

  rxo->bytes_sent += nbytes;

  runtime = (uv_hrtime() - rxo->time_started);
  runtime = runtime / (1000.0 * 1000.0 * 1000.0);
  printf("duration: %2.2f, avarage per sec: %2.2f kB, total sent: %llu kB\n", runtime, (rxo->bytes_sent / runtime) / 1024.0, rxo->bytes_sent / 1024) ;
}


static void on_webm_data(rxo_webm* webm, uint8_t* data, uint32_t nbytes) {
  int r;
  rxo_streamer* rxo = NULL;
  double runtime = 0;

  rxo = (rxo_streamer*)webm->user;

  r = shout_send(rxo->shout, data, nbytes);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: sending shout: %s\n", shout_get_error(rxo->shout));
  }

  rxo->bytes_sent += nbytes;

  runtime = (uv_hrtime() - rxo->time_started);
  runtime = runtime / (1000.0 * 1000.0 * 1000.0);
  printf("duration: %2.2f, avarage per sec: %2.2f kB, total sent: %llu kB\n", runtime, (rxo->bytes_sent / runtime) / 1024.0, rxo->bytes_sent / 1024) ;
}
