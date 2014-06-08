
     
  rxo_streamer
  ------------
  -  Add rxo_streamer_clear to cleanup memory

  rxo_theora
  ----------  

  -  We need to make sure the frame_width, frame_height, pic_width
     and pic_height have correct values. See the theora encoder example
     for some info: 

        th->info.frame_width  = ((w + 15) >> 4) << 4;  
        th->info.frame_height = ((h + 15) >> 4) << 4;

  -  Add option to either use quality control or bitrate control: 

        th->info.target_bitrate = 3000;   

  -  Set granule shift.

        th->info.keyframe_granule_shift = 4;
        
  -  Add rxo_theora_clear() to cleanup any used memory.
