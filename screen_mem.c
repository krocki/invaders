#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_GLCOREARB
#else
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLFW/glfw3.h>

#define TEX_W 64
#define TEX_H 64
#define SCR_W 512
#define SCR_H 512

#define bind_key(x,y) \
{ if (action == GLFW_PRESS && key == (x)) (y) = 1; if (action == GLFW_RELEASE && key == (x)) (y) = 0; if (y) {printf(#y "\n");} }
#define bind_key_toggle(x,y) \
{ if (action == GLFW_PRESS && key == (x)) (y) = (y); if (action == GLFW_RELEASE && key == (x)) { (y) = 1-(y); printf(#x ", " #y "=%u\n", (y));} }


u8 mode=0;
u8 speed=0;
u8 paused=0;
u8 step=0;
u8 verbose=0;
u8 reset=1;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

  bind_key_toggle(GLFW_KEY_R,     reset);
  bind_key_toggle(GLFW_KEY_M,     mode);
  bind_key_toggle(GLFW_KEY_S,     speed);
  bind_key_toggle(GLFW_KEY_P,     paused);
  bind_key_toggle(GLFW_KEY_SPACE, step);
  bind_key_toggle(GLFW_KEY_V,     verbose);
}

int gl_ok=0;
int gl_pause=0;
double t0;

const char *vs_name = "vs_pass.glsl";
const char *fs_name = "fs_screen.glsl";

char *load_src(const char *file) {
  FILE *f = fopen(file, "r");
  if (!f) {
    fprintf(stderr,
      "couldn't open %s\n", file);
    return NULL;
  }
  fseek(f, 0L, SEEK_END);
  int len = ftell(f);
  rewind(f);

  char *src = malloc(len+1);
  size_t cnt = fread(src, len, 1, f);
  if (0==cnt)
    fprintf(stderr, "fread failed\n");
  fclose(f);
  src[len] = '\0';
  return src;
}

void check_err(const char *m, GLuint *s) {
  GLint res = GL_FALSE;
  int log_len;
  glGetShaderiv(*s, GL_COMPILE_STATUS, &res);
  glGetShaderiv(*s, GL_INFO_LOG_LENGTH, &log_len);
  if (log_len > 0) {
    char *message = malloc(log_len+1);
    glGetShaderInfoLog(*s, log_len, NULL, message);
    printf("%s: %s\n", m, message);
    free(message);
  }
}

void load_shaders(GLuint *v, const char *vf,
                 GLuint *f, const char *ff) {
  char *v_src = load_src(vf);
  char *f_src = load_src(ff);
  *v = glCreateShader(GL_VERTEX_SHADER);
  *f = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(*v, 1, (const char*const*)&v_src, NULL);
  glShaderSource(*f, 1, (const char*const*)&f_src, NULL);
  glCompileShader(*v);
  glCompileShader(*f);
  free(v_src);
  free(f_src);

  /* check */
  check_err("vertex shader", v);
  check_err("fragment shader", f);
}
int display_init(int argc, char **argv) {

  GLuint width = argc > 1 ?
    strtol(argv[1], NULL, 10) : SCR_H;
  GLuint height = argc > 2 ?
    strtol(argv[2], NULL, 10) : SCR_W;

  GLFWwindow *window = NULL;
  const GLubyte *renderer;
  const GLubyte *version;
  GLuint tex_vbo, vbo, vao;
  GLuint vert_shader, frag_shader;
  GLuint shader_prog;

  float size = 0.95f;

  GLfloat vertices[] = {
   -size,  size, 0.0f,
    size,  size, 0.0f,
    size, -size, 0.0f,
    size, -size, 0.0f,
   -size, -size, 0.0f,
   -size,  size, 0.0f };

  GLfloat texcoords[] = {
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f };

  if (!glfwInit()) {
    fprintf(stderr,
      "couldn't initialize glfw3\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
  GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE,
  GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(
    width, height, "GLSL test", NULL, NULL);

  if (!window) {
    fprintf(stderr,
      "couldn't initialize GLFWwindow\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  renderer = glGetString(GL_RENDERER);
  version = glGetString(GL_VERSION);

  fprintf(stdout,
    "GL_RENDERER: %s\n"
    "GL_VERSION: %s\n",
    renderer, version);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  /* allocate gpu's memory for vertices */
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
    18 * sizeof(GLfloat),
    vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &tex_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
  glBufferData(GL_ARRAY_BUFFER,
    12 * sizeof(GLfloat),
    texcoords, GL_STATIC_DRAW);

  /* use the vbo and use 3 float per 'variable' */
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT,
    GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
  glVertexAttribPointer(1, 2, GL_FLOAT,
    GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  load_shaders(&vert_shader, vs_name,
               &frag_shader, fs_name);

  /* link glsl program */
  shader_prog = glCreateProgram();
  glAttachShader(shader_prog, frag_shader);
  glAttachShader(shader_prog, vert_shader);
  glLinkProgram(shader_prog);

  GLuint tex[2];
  glGenTextures( 2, tex );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, tex[0] );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, TEX_W, TEX_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, mem_ptr );
  glGenerateMipmap( GL_TEXTURE_2D );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

  GLuint frag_mode = glGetUniformLocation(shader_prog, "mode");
  gl_ok = 1;

  printf("%9.6f, GL_init OK\n", glfwGetTime()-t0);
  glfwSetKeyCallback(window, key_callback);

  /* main loop */
  while (!glfwWindowShouldClose(window)) {
    /* clear */
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_prog);
    glBindVertexArray(vao);

    glUniform1i(frag_mode, mode);

    /* draw */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEX_W, TEX_H, GL_RGBA, GL_UNSIGNED_BYTE, mem_ptr );
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwPollEvents();
    if (GLFW_PRESS == glfwGetKey(
      window, GLFW_KEY_ESCAPE))
      glfwSetWindowShouldClose(window, 1);
    glfwSwapBuffers(window);
  }

  gl_ok = 0;
  printf("%9.6f, GL terminating\n", glfwGetTime()-t0);
  glfwTerminate();
  return 0;
}

void cpu_step(cpu *c) {

  c->op = mem[PC];
  if (verbose) reg_print(c);
  PC++;
  ((void(*)(cpu*))ops[c->op])(c);
  c->cycl++;

}

void *work(void *args) {

  char *romname = (char*)args;
  printf("romname = %s\n", romname);
  u16 load_at = 0;
  //mem_clear(mem_ptr, MEMSIZE);
  //mem_load(&mem_ptr[load_at], romname);
  //mem_print(mem_ptr, 32);
  //cpu c = {0};
  cpu c = {0};
  ops_init();

  printf("%9.3f, CPU starting...\n", glfwGetTime()-t0);
  printf("%9.3f, reset ok, waiting for GL...\n", glfwGetTime()-t0);

  while (!gl_ok) usleep(10);
  printf("%9.3f, GL OK\n", glfwGetTime() - t0);

  while (gl_ok) {
    if (reset) {
      reset=0;
      mem_clear(mem_ptr, MEMSIZE);
      mem_load(&mem_ptr[load_at], romname);
      c = (cpu){0};
    }
    if (!fail) {
      if (speed) usleep(10000);
      if (!paused || step) {
        step = 0;
        cpu_step(&c);
      }
    } else {
      usleep(10000);
    }
  }

  printf("%9.6f, terminating\n", glfwGetTime()-t0);
  return NULL;
}

int main(int argc, char **argv) {

  pthread_t cpu_thread;
  char *romname = "invaders.rom";
  if(pthread_create(&cpu_thread, NULL, work, (void*)romname)) {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }

  display_init(argc, argv);

  if(pthread_join(cpu_thread, NULL)) {
    fprintf(stderr, "Error joining thread\n");
    return 2;
  }

  return 0;
}
