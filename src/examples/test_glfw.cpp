/*
 
  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.
 
*/
#include <stdlib.h>
#include <stdio.h>
 
#if defined(__linux) || defined(_WIN32)
#  include <GLXW/glxw.h>
#endif

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

#define RXO_OPENGL_IMPLEMENTATION
extern "C" {
#  include <rxo_streamer/rxo_gl.h>
#  include <rxo_streamer/rxo_streamer.h>
}

rxo_gl yuv;
rxo_streamer streamer;
 
void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);
 
int main() {
 
  glfwSetErrorCallback(error_callback);
 
  if(!glfwInit()) {
    printf("Error: cannot setup glfw.\n");
    exit(EXIT_FAILURE);
  }
 
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  GLFWwindow* win = NULL;
  int w = 640;
  int h = 480;
 
  win = glfwCreateWindow(w, h, "GLFW", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
 
  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetCharCallback(win, char_callback);
  glfwSetCursorPosCallback(win, cursor_callback);
  glfwSetMouseButtonCallback(win, button_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);
 
#if defined(__linux) || defined(_WIN32)
  if(glxwInit() != 0) {
    printf("Error: cannot initialize glxw.\n");
    exit(EXIT_FAILURE);
  }
#endif
 
  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------
  Painter paint;
  paint.clear();


  rxo_gl_info info;
  info.width = w * 0.25;
  info.height = h * 0.25;

  if (rxo_gl_init(&yuv, &info) < 0) {
    printf("Error: cannot init converter.\n");
    exit(1);
  }

  rxo_info cfg;
  cfg.width = w * 0.25;
  cfg.height = h * 0.25;
  cfg.fps_numerator = 30;
  cfg.fps_denominator = 1;
  cfg.quality = 75;
  cfg.port = 8000;
  cfg.host = "127.0.0.1";
  cfg.user = "source";
  cfg.password = "hackme";
  cfg.mode = RXO_WEBM;
  //  cfg.mode = RXO_OGG;

  if (cfg.mode == RXO_WEBM) {
    cfg.mount = "/example.webm";
  }
  else if (cfg.mode == RXO_OGG) {
    cfg.mount = "/example.ogg";
  }

  if (rxo_streamer_init(&streamer, &cfg) < 0) {
    printf("Error: cannot initialize the streamer.\n");
    exit(1);
  }

  double t = 0.01;
  double r = 0.0;

  while(!glfwWindowShouldClose(win)) {
    glViewport(0, 0, w, h);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    r =  double(w * 0.3) * (0.5 + sin(t * 3.141519) * 0.5);

    paint.clear();
    paint.fill();
    paint.circle(w * 0.5, h * 0.5, r);  
    t += 0.01;

    rxo_gl_begin(&yuv);
    paint.draw();
    rxo_gl_end(&yuv);

    rxo_gl_draw(&yuv);
    rxo_gl_download(&yuv);
    rxo_streamer_add_planes(&streamer, 
                            yuv.planes[0].ptr, yuv.planes[0].stride,
                            yuv.planes[1].ptr, yuv.planes[1].stride,
                            yuv.planes[2].ptr, yuv.planes[2].stride);
 
    glfwSwapBuffers(win);
    glfwPollEvents();
  }
 
 
  glfwTerminate();
 
  return EXIT_SUCCESS;
}

void char_callback(GLFWwindow* win, unsigned int key) {
}
 
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
  
  if(action != GLFW_PRESS) {
    return;
  }
 
  switch(key) {
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
  };
}
 
void resize_callback(GLFWwindow* window, int width, int height) {
}
 
void cursor_callback(GLFWwindow* win, double x, double y) {
}
 
void button_callback(GLFWwindow* win, int bt, int action, int mods) {
  /*
    if(action == GLFW_PRESS) { 
    }
  */
}
 
void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}
