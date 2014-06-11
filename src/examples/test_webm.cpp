x#include <stdio.h>
#include <stdlib.h>
#include <webmtools/webm_live_muxer.h>

using namespace webm_tools;

int main() {
  int r, tracknum;

  printf("\n\ntest_webm\n\n");

  WebMLiveMuxer muxer;
  if (muxer.Init() != WebMLiveMuxer::kSuccess) {
    printf("Error: cannot init muxer.\n");
    exit(1);
  }

  tracknum = muxer.AddVideoTrack(640, 480, "webm");
  if (r < 0) {
    printf("Error: cannot add video track: %d\n", tracknum);
    exit(1);
  }

  if (muxer.Finalize() != WebMLiveMuxer::kSuccess) {
    printf("Error: cannot finalize.\n");
  }

  return 0;
}
