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

#define W 64
#define H 64
#define SCR_W 256
#define SCR_H 256

GLbyte data[W*H*4];
int gl_ok=0;
int gl_pause=0;
double t0;

const char *vertex_shader =
  "#version 410\n"
  "layout (location = 0) in vec3 vp;"
  "layout (location = 1) in vec2 vt;"
  "out vec2 coords;"
  "void main () {"
  "  coords = vt;"
  "  gl_Position = vec4(vp, 1.0);"
  "}";

const char *fragment_shader =
  "#version 410\n"
  "in vec2 coords;"
  "uniform sampler2D tx;"
  "out vec4 frag_color;"
  "void main () {"
  "  frag_color = texture (tx, vec2(coords.x, coords.y));"
  "}";

void rand_data(GLbyte *data, int x, int y, int w, int h) {
  for (int i=x; i<x+w; i++)
  for (int j=y; j<y+h; j++)
  for (int k=0; k<4; k++) {
    data[j*W*4 + (i*4) + k] = rand() & 0xff;
    data[j*W*4 + (i*4) + k] = rand() & 0xff;
    data[j*W*4 + (i*4) + k] = rand() & 0xff;
    data[j*W*4 + (i*4) + k] = rand() & 0xff;
  }
}

void load_data(char *fname, GLbyte *data, int x, int y) {
  FILE *f = fopen(fname, "rb");
  if (f) {
    fread(data, 4 * x * y, 1, f);
    fclose(f);
  } else {
    fprintf(stderr, "couldn't open %s\n", fname);
  }
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

  GLfloat texcoords[] = { 0.0f, 0.0f, 1.0f,
                          0.0f, 1.0f, 1.0f,
                          1.0f, 1.0f, 0.0f,
                          1.0f, 0.0f, 0.0f };

  load_data("space.bin", data, W, H);

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

  /* vertex shader */
  vert_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_shader, 1,
    &vertex_shader, NULL);
  glCompileShader(vert_shader);

  /* fragment shader */
  frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_shader, 1,
    &fragment_shader, NULL);
  glCompileShader(frag_shader);

  /* link glsl program */
  shader_prog = glCreateProgram();
  glAttachShader(shader_prog, frag_shader);
  glAttachShader(shader_prog, vert_shader);
  glLinkProgram(shader_prog);

  GLuint tex;
  glGenTextures( 1, &tex );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, tex );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
  glGenerateMipmap( GL_TEXTURE_2D );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

  gl_ok = 1;

  printf("%9.6f, GL_init OK\n", glfwGetTime()-t0);
  /* main loop */
  while (!glfwWindowShouldClose(window)) {
    /* clear */
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_prog);
    glBindVertexArray(vao);

    /* draw */
    //rand_data(data, 12, 12, 24, 24);
    //add_noise(data);
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, data );
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

void *cpu(void *args) {

  void *ptr = args;
  printf("ptr = %p\n", ptr);

  printf("%9.3f, GB starting...\n", glfwGetTime()-t0);
  printf("%9.3f, reset ok, waiting for GL...\n", glfwGetTime()-t0);

  while (!gl_ok) usleep(10);
  printf("%9.3f, GL OK\n", glfwGetTime() - t0);

  while (gl_ok) {
    if (0==gl_pause) {
      for (int i=0; i<H; i++)
      for (int j=0; j<W; j++)
      for (int c=0; c<4; c++)
        data[i*W*4 + j*4 + c] = rand() & 0xff;
        usleep(100000);
    }
  }

  printf("%9.6f, terminating\n", glfwGetTime()-t0);
  return NULL;
}

int main(int argc, char **argv) {

  pthread_t cpu_thread;
  if(pthread_create(&cpu_thread, NULL, cpu, argv[1])) {
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
