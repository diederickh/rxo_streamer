#include <stdio.h>
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
    
  //r = shout_set_format(rxo->shout, SHOUT_FORMAT_OGG);
  r = shout_set_format(rxo->shout, SHOUT_FORMAT_WEBM);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: setting shout format: %s\n", shout_get_error(rxo->shout));
    return -10;
  }

  r = shout_open(rxo->shout);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: cannot open shout.\n");
    return -11;
  }

  /* set callbacks */
  rxo->theora.user = rxo;
  rxo->theora.on_data = on_theora_data;
  rxo->webm.user = rxo;
  rxo->webm.on_chunk = on_webm_data;

  /* init theora encoder */
  if (rxo_theora_init(&rxo->theora, info) < 0) {
    printf("Error: cannot init theora.\n");
    return -2;
  }


  if (rxo_webm_init(&rxo->webm, info) < 0) {
    printf("Error: cannot init webm.\n");
    return -3;
  }

  return 0;
}

int rxo_streamer_add_frame(rxo_streamer* rxo, uint8_t* pixels, uint32_t nbytes) {

#if !defined(NDEBUG)  
  if (!rxo)    { return -1; } 
  if (!pixels) { return -2; } 
  if (!nbytes) { return -3; } 
#endif

  //  printf("Add frame: %u\n", nbytes);
  /*
  if (rxo_theora_add_frame(&rxo->theora, pixels, nbytes) < 0) {
    printf("Error: cannot add theora frame.\n");
    return -4;
  };
  */
  /* BEGIN TMP */
  rxo_webm_encode(&rxo->webm, pixels, nbytes);
  /* END TMP */

  return 0;
}

/* ----------------------------------------------- */

static void on_theora_data(rxo_theora* th, uint8_t* data, uint32_t nbytes) {

  int r;
  rxo_streamer* rxo = NULL;

#if !defined(NDEBUG)
  if (!th)     { printf("Error: invalid theora.\n");   exit(1); } 
  if (!data)   { printf("Error: invalid data.\n");     exit(1); } 
  if (!nbytes) { printf("Error: invalid nbytes.\n");   exit(1); } 
#endif

  rxo = (rxo_streamer*)th->user;

#if 1
  /*
  r = shout_send(rxo->shout, data, nbytes);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: sending shout: %s\n", shout_get_error(rxo->shout));
  }
  */
  //  shout_sync(rxo->shout);

#else 
  // printf("Go data: %u\n", nbytes);
  rxo->on_data(rxo, data, nbytes);
#endif
}


static void on_webm_data(rxo_webm* webm, uint8_t* data, uint32_t nbytes) {
  int r;
  rxo_streamer* rxo = NULL;

  rxo = (rxo_streamer*)webm->user;

  printf("Got webmchunk: %d\n", nbytes);
  
  r = shout_send(rxo->shout, data, nbytes);
  if (r != SHOUTERR_SUCCESS) {
    printf("Error: sending shout: %s\n", shout_get_error(rxo->shout));
  }
}
