
 rxo_streamer
 ------------

 Basic theora + ogg streamer. Streams a webcam to an icecast server using 
 the cross platform [videocapture library](https://github.com/roxlu/video_capture)


 Files:
 ````sh
 - rxo_generator.c  - generates a test yuv stream
 - rxo_streamer.c   - uses the rxo_teora encoder to encode video and sends it to an icecast server
 - rxo_theora.c     - wrapper around the theora encoder.
 ````