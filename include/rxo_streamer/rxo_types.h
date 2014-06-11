#ifndef RXO_TYPES_H
#define RXO_TYPES_H

#define RXO_WEBM 0
#define RXO_OGG 1

typedef struct rxo_info rxo_info;

struct rxo_info {                     /* settings for the streamer + encoder */

  /* encoder */ 
  int width;                          /* video width */
  int height;                         /* video height */
  int fps_numerator;                  /* fps */
  int fps_denominator;                /* fps */
  int quality;                        /* encoding quality */
  int mode;                           /* what mode: RXO_WEBM or RXO_OGG */

  /* shout */
  int port;                           /* icecast server port */
  const char* host;                   /* icecast IP */
  const char* user;                   /* user that connects to the icecast server, normally you use the "source" user */
  const char* password;               /* password for icecast server */
  const char* mount;                  /* mount point to which we stream, e.g. /example.ogg */
};


#endif
