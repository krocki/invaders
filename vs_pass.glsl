#version 410
layout (location = 0) in vec3 vp;
layout (location = 1) in vec2 vt;
out vec2 coords;
void main () {
  coords = vt;
  gl_Position = vec4(vp, 1.0);
}
