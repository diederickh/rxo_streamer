cmake_minimum_required(VERSION 2.8)

project(rxo_streamer)

include(${CMAKE_CURRENT_LIST_DIR}/Triplet.cmake)

set(app app)
set(sd ${CMAKE_CURRENT_LIST_DIR}/../src/)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(app "${app}_debug")
endif()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../src
  ${CMAKE_CURRENT_LIST_DIR}/../include
  ${extern_include_dir}
  ${CMAKE_CURRENT_LIST_DIR}/../extern/tinylib/src
)

set(rxo_streamer_sources
  ${sd}/rxo_streamer/rxo_streamer.c
  ${sd}/rxo_streamer/rxo_theora.c
  ${sd}/rxo_streamer/rxo_generator.c
  ${sd}/rxo_streamer/rxo_webm.cpp
  ${sd}/rxo_streamer/rxo_vpx.c
)

#set_source_files_properties(  ${sd}/rxo_streamer/rxo_webm.cpp PROPERTIES LANGUAGE CXX )
# if(UNIX AND NOT APPLE)
#   list(APPEND app_sources ${extern_source_dir}/GLXW/glxw.c)
#   message(STATUS "Adding glxw.c for GL-function loading.")
#   message(${app_sources})
# endif()

if(APPLE)

#   find_library(fr_corefoundation CoreFoundation)
#   find_library(fr_avfoundation AVFoundation)
#   find_library(fr_coremedia CoreMedia)
#   find_library(fr_cocoa Cocoa)
#   find_library(fr_opengl OpenGL)
#   find_library(fr_iokit IOKit)
#   find_library(fr_corevideo CoreVideo)
#   find_library(fr_opencl OpenCL)
#   find_library(fr_audio AudioUnit)
#   find_library(fr_caudio CoreAudio)
#   find_library(fr_audio_toolbox AudioToolbox)
# 
#   set(app_libs
#     ${extern_lib_dir}/libvpx.a
#     ${extern_lib_dir}/libuv.a
#     ${extern_lib_dir}/libpng.a
#     ${extern_lib_dir}/libz.a
#     ${extern_lib_dir}/libglfw3.a
#     ${extern_lib_dir}/libvideocapture.a
#     ${extern_lib_dir}/libyuv.a
#     ${fr_corefoundation}  # uv
#     ${fr_cocoa}           # uv
#     ${fr_iokit}           # uv
#     ${fr_opengl}          # glfw
#     ${fr_corevideo}       # glfw
#     ${fr_avfoundation}    # videocapture
#     ${fr_coremedia}       # videocapture
# 
#     )

elseif(UNIX)

  set(debug_flag "")

  if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(debug_flag "d")
  endif()

  set(app_libs
    ${extern_lib_dir}/libuv.a
    ${extern_lib_dir}/libyuv.a  # for webcam conversion
    ${extern_lib_dir}/libpng.a
    ${extern_lib_dir}/libshout.a
    ${extern_lib_dir}/libvorbis.a
    ${extern_lib_dir}/libtheora.a
    ${extern_lib_dir}/libtheoraenc.a
    ${extern_lib_dir}/libogg.a
    ${extern_lib_dir}/libspeex.a
    ${extern_lib_dir}/libvideocapture.a
    ${extern_lib_dir}/libvpx.a
    ${extern_lib_dir}/libwebmtools.a
    ${extern_lib_dir}/libwebm.a
    ${extern_lib_dir}/libuv.a       # timing for webm
    udev         # libvideocapture
    GL
    X11
    Xxf86vm
    Xrandr
    pthread
    dl
    Xi
    asound
    z
    m
    )
endif()

# rxs_streamer library
add_library(rxo_streamer ${rxo_streamer_sources})
install(TARGETS rxo_streamer ARCHIVE DESTINATION lib)

# test ogg streamer
add_executable(test_ogg ${sd}/examples/test_ogg.cpp ${app_sources})
target_link_libraries(test_ogg rxo_streamer ${app_libs})
install(TARGETS test_ogg DESTINATION bin)

# test shout sender
add_executable(test_shout ${sd}/examples/test_shout.cpp ${app_sources})
target_link_libraries(test_shout rxo_streamer ${app_libs})
install(TARGETS test_shout DESTINATION bin)

# test webcam sender
add_executable(test_webcam ${sd}/examples/test_webcam.cpp ${app_sources})
target_link_libraries(test_webcam rxo_streamer ${app_libs})
install(TARGETS test_webcam DESTINATION bin)

# test webm muxer
add_executable(test_webm ${sd}/examples/test_webm.cpp ${app_sources})
target_link_libraries(test_webm rxo_streamer ${app_libs})
install(TARGETS test_webm DESTINATION bin)










