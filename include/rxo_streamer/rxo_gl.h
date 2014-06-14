
/*


 rxo_gl
 --------------------------------------------------

 rxo_gl grabs your GL scene and convert the framebuffer 
 into a YUV420P buffer that you can use to feed it directly
 into a common video encoder like theora / webm / h264.

 It will create one texture into which it write the Y, U
 and V planes. You can use the member rxo_gl.planes, to access
 the y,u and v plane and the necesary info to feed into 
 an encode. 

 See th test_glfw.cpp example.

 Usage:
 ------
 
 1. call rxo_gl_init() to initialize the struct 
 2. call rxo_gl_begin(), then draw something, then call rxo_gl_end()
 3. call rxo_gl_download() to download the captured scene into the .planes member.
 4. when ready, free memory using rxo_gl_clear().

 See test_glfw.cpp for a complete example.

 */

#ifndef RXO_OPENGL_H
#define RXO_OPENGL_H

// vertex shader that is shared with all progs
static const char* YUV420P_YUV_VS = ""
  "#version 150\n"
  "const vec2 vert_data[4] = vec2[]("
  "  vec2(-1.0, 1.0),"
  "  vec2(-1.0, -1.0),"
  "  vec2(1.0, 1.0),"
  "  vec2(1.0, -1.0)"
  ");"

  "const vec2 tex_data[4] = vec2[]("
  "  vec2(0.0, 1.0), "
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)  "
  ");"

  "out vec2 v_tex; "

  "void main() { "
  "  gl_Position = vec4(vert_data[gl_VertexID], 0.0, 1.0);"
  "  v_tex = tex_data[gl_VertexID]; "
  "}"
  ;

// conversion shaders
static const char* YUV420P_Y_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcol;"
  "void main() {"
  "  vec3 tc = texture(u_tex, v_tex).rgb; "
  "  fragcol.rgba = vec4(1.0);"           // we need to set all channels
  "  fragcol.r = (tc.r * 0.257) + (tc.g * 0.504) + (tc.b * 0.098) + 0.0625;"
  "}"
  ;

static const char* YUV420P_U_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 u_channel;"
  "void main() {"
  "  vec3 tc = texture(u_tex, v_tex).rgb; "
  "  u_channel = vec4(1.0);"
  "  u_channel.r = -(tc.r * 0.148) - (tc.g * 0.291) + (tc.b * 0.439) + 0.5; "
  "}"
  ;

static const char* YUV420P_V_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 v_channel;"
  "void main() {"
    "  vec3 tc = texture(u_tex, v_tex).rgb; "
    "  v_channel = vec4(1.0);"
    "  v_channel.r = (tc.r * 0.439) - (tc.g * 0.368) - (tc.b * 0.071) + 0.5; "
  "}"
  ;

// pass through shader
static const char* YUV420P_PT_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;\n"
  "in vec2 v_tex; " 
  "out vec4 fragcol; "
  "void main() {"
  " fragcol = vec4(texture(u_tex, v_tex).rgb, 1.0);"
  "}"
  ;

typedef struct rxo_gl_pass rxo_gl_pass;
typedef struct rxo_gl_info rxo_gl_info;
typedef struct rxo_gl_img rxo_gl_img;
typedef struct rxo_gl rxo_gl;

struct rxo_gl_info {
  int width;
  int height;
};

struct rxo_gl_pass {
  GLuint prog;
  GLuint vert;
  GLuint frag;
};

struct rxo_gl_img {
  uint8_t* ptr;
  int w;
  int h;
  int stride;
};

struct rxo_gl {
  GLuint vao;
  GLuint input_tex;
  GLuint input_fbo;
  GLuint input_depth;
  GLuint output_tex;
  GLuint output_fbo;
  rxo_gl_pass pass_y;
  rxo_gl_pass pass_u;
  rxo_gl_pass pass_v;
  rxo_gl_pass pass_draw;
  int w;                                       /* width of the video / default viewport */
  int h;                                       /* height of the video / default viewport */
  int viewport[4];                             /* default viewport size */
  rxo_gl_img planes[3];
  unsigned char* pixels;
  GLuint pbos[2];
  int pbo_dx;
};

int rxo_gl_init(rxo_gl* gl, rxo_gl_info* info);
int rxo_gl_begin(rxo_gl* gl);
int rxo_gl_end(rxo_gl* gl);
int rxo_gl_clear(rxo_gl* gl);
int rxo_gl_draw(rxo_gl* gl);  /* draws the captured scene */
int rxo_gl_download(rxo_gl* gl);

#endif


/*
 rxo_opengl.c
 --------------------------------------------------
 */

#if defined(RXO_OPENGL_IMPLEMENTATION)

/* -------------------------------------------------------------------------- */

static GLuint gl_create_shader(GLenum shader, const char* ptr);
static GLuint gl_create_program(GLuint vert, GLuint frag);
static GLuint gl_create_texture(GLenum internalFormat, GLenum format, int w, int h);
static int convert_input_to_yuv(); /* converts the input scene into YUV420P */

/* -------------------------------------------------------------------------- */

int rxo_gl_init(rxo_gl* gl, rxo_gl_info* info) {
  
  if (!gl)           { return -1; } 
  if (!info)         { return -2; } 
  if (!info->width)  { return -3; } 
  if (!info->height) { return -4; } 

  /* copy private members */
  gl->w = info->width;
  gl->h = info->height;
  gl->pbo_dx = 0;

  /* create image buffers */
  gl->pixels = (unsigned char*) malloc( (gl->w * 2) * gl->h );
  gl->planes[0].w      = gl->w;
  gl->planes[0].h      = gl->h;
  gl->planes[0].stride = gl->w * 2;
  gl->planes[0].ptr    = gl->pixels;
  gl->planes[1].w      = gl->w * 0.5;
  gl->planes[1].h      = gl->h * 0.5;
  gl->planes[1].stride = gl->w * 2;
  gl->planes[1].ptr    = gl->pixels + gl->w;  
  gl->planes[2].w      = gl->w * 0.5;
  gl->planes[2].h      = gl->h * 0.5;
  gl->planes[2].stride = gl->w * 2;
  gl->planes[2].ptr    = gl->planes[1].ptr + (gl->w / 2);

  /* get the default viewport */
  glGetIntegerv(GL_VIEWPORT, gl->viewport);

  /* create textures for input and output FBO */
  gl->output_tex = gl_create_texture(GL_R8, GL_RED, gl->w * 2, gl->h);
  gl->input_tex = gl_create_texture(GL_RGBA, GL_RGBA, gl->viewport[2], gl->viewport[3]);

  /* input fbo */
  {
    glGenRenderbuffers(1, &gl->input_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, gl->input_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, gl->viewport[2], gl->viewport[3]);

    glGenFramebuffers(1, &gl->input_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gl->input_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl->input_tex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gl->input_depth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      return -5; /* @todo - should clear everything */
    }
  }

  /* output fbo */
  {
    glGenFramebuffers(1, &gl->output_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gl->output_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl->output_tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      return -6;  /* @todo - should clear everything */
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  /* Y pass */
  {
    gl->pass_y.vert = gl_create_shader(GL_VERTEX_SHADER, YUV420P_YUV_VS);
    gl->pass_y.frag = gl_create_shader(GL_FRAGMENT_SHADER, YUV420P_Y_FS);
    gl->pass_y.prog = gl_create_program(gl->pass_y.vert, gl->pass_y.frag);

    glUseProgram(gl->pass_y.prog);
    glUniform1i(glGetUniformLocation(gl->pass_y.prog, "u_tex"), 0);
    glUseProgram(0);
  }

  /* U pass */
  {
    gl->pass_u.vert = gl->pass_y.vert;
    gl->pass_u.frag = gl_create_shader(GL_FRAGMENT_SHADER, YUV420P_U_FS);
    gl->pass_u.prog = gl_create_program(gl->pass_u.vert, gl->pass_u.frag);

    glUseProgram(gl->pass_u.prog);
    glUniform1i(glGetUniformLocation(gl->pass_u.prog, "u_tex"), 0);
    glUseProgram(0);
  }

  /* V pass */
  {
    gl->pass_v.vert = gl->pass_y.vert;
    gl->pass_v.frag = gl_create_shader(GL_FRAGMENT_SHADER, YUV420P_V_FS);
    gl->pass_v.prog = gl_create_program(gl->pass_v.vert, gl->pass_v.frag);

    glUseProgram(gl->pass_v.prog);
    glUniform1i(glGetUniformLocation(gl->pass_v.prog, "u_tex"), 0);
    glUseProgram(0);
  }

  /* draw pass */
  {
    gl->pass_draw.vert = gl->pass_y.vert;
    gl->pass_draw.frag = gl_create_shader(GL_FRAGMENT_SHADER, YUV420P_PT_FS);
    gl->pass_draw.prog = glCreateProgram();

    glAttachShader(gl->pass_draw.prog, gl->pass_draw.vert);
    glAttachShader(gl->pass_draw.prog, gl->pass_draw.frag);
    glBindFragDataLocation(gl->pass_draw.prog, 0, "fragcol");
    glLinkProgram(gl->pass_draw.prog);

    glUseProgram(gl->pass_draw.prog);
    glUniform1i(glGetUniformLocation(gl->pass_draw.prog, "u_tex"), 0);
    glUseProgram(0);
  }

  /* we need a VAO, even for attributeless rendering. */
  glGenVertexArrays(1, &gl->vao);

  /* pixel buffers */
  glGenBuffers(2, gl->pbos);
  glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->pbos[0]);
  glBufferData(GL_PIXEL_PACK_BUFFER, (gl->w * 2) * gl->h, NULL, GL_STREAM_COPY);
  glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->pbos[1]);
  glBufferData(GL_PIXEL_PACK_BUFFER, (gl->w * 2) * gl->h, NULL, GL_STREAM_COPY);
  glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

  return 0;
}

int rxo_gl_clear(rxo_gl* gl) {
  if (!gl) { return -1; } 

  if (gl->pixels) {
    free(gl->pixels);
    gl->pixels = NULL;
  }

  /* @todo - free GL objects */

  return 0;
}

int rxo_gl_begin(rxo_gl* gl) {

  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 };

  if (!gl) { return -1; } 
  
  glBindFramebuffer(GL_FRAMEBUFFER, gl->input_fbo);
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(gl->viewport[0], gl->viewport[1], gl->viewport[2], gl->viewport[3]);

  return 0;
}

int rxo_gl_end(rxo_gl* gl) {

  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 };

  if (!gl) { return -1; } 

  glBindFramebuffer(GL_FRAMEBUFFER, gl->output_fbo);
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gl->input_tex);

  /* y-pass */
  glViewport(0, 0, gl->w, gl->h);
  glUseProgram(gl->pass_y.prog); 
  glBindVertexArray(gl->vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  
  /* u-pass */
  glViewport(gl->w, 0, gl->planes[1].w, gl->planes[1].h);
  glUseProgram(gl->pass_u.prog);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  /* v-pass */
  glUseProgram(gl->pass_v.prog);
  glViewport(gl->w + gl->planes[1].w , 0, gl->planes[1].w, gl->planes[1].h);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(0);
  glViewport(gl->viewport[0], gl->viewport[1], gl->viewport[2], gl->viewport[3]);

  return 0;
}

int rxo_gl_draw(rxo_gl* gl) {
  if (!gl) { return -1; } 

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gl->input_tex);

  glBindVertexArray(gl->vao);
  glUseProgram(gl->pass_draw.prog);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  return 0;
}

int rxo_gl_download(rxo_gl* gl) {

  unsigned char* ptr;

  if (!gl) { return -1; } 

  /* write to pbo 'a'  (pbo ping pong) */
  {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->output_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->pbos[gl->pbo_dx]);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ROW_LENGTH, gl->w * 2);
    glReadPixels(0, 0, gl->w * 2, gl->h, GL_RED, GL_UNSIGNED_BYTE, NULL);
  }

  /* read from pbo 'b' */
  {
    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->pbos[1 - gl->pbo_dx]);
    ptr = (unsigned char*) glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY); 
    if (ptr) {
      memcpy(gl->pixels, ptr, (gl->w * 2) * gl->h);
    }
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
  }

  gl->pbo_dx = 1 - gl->pbo_dx;

  glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return 0;
}

/* -------------------------------------------------------------------------- */

static int convert_input_to_yuv(rxo_gl* gl) {
  return 0;
}

static GLuint gl_create_program(GLuint vert, GLuint frag) {
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glLinkProgram(prog);
  return prog;
}

static GLuint gl_create_texture(GLenum internalFormat, GLenum format, int w, int h) {
  GLuint tex; 
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  return tex;
}

static GLuint gl_create_shader(GLenum type, const char* ptr) {

  GLuint sh;
  GLint status;

  if (!ptr) { 
    printf("Error: invalid shader pointer given to rxo_gl_create_shader.\n");
    exit(1);
  }

  sh = glCreateShader(type);
  glShaderSource(sh, 1, &ptr, NULL);
  glCompileShader(sh);
  glGetShaderiv(sh, GL_COMPILE_STATUS, &status);

  if (status == GL_FALSE) {
    printf("Error: couldn't compile the following shader:\n\n---\n%s\n---\n\n", ptr);
    exit(1);
  }
  
  return sh;
}

#endif
