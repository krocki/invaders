#version 410
in vec2 coords;
uniform sampler2D tx;
uniform int mode;
uniform vec2 mem_offset;
uniform vec2 mem_scale;
out vec4 frag_color;

void main () {

  vec2 f_coords;
  float ratio = 224.0f/256.0f;
  if (mode==0) {
    frag_color = texture (tx,
    vec2(
    mem_scale.x * (mem_offset.x + coords.x),
    mem_scale.y * (mem_offset.y + coords.y)
    ));
  }

  if (mode==1) {
    frag_color = texture (tx, vec2(1.0-coords.y, coords.x*ratio));
  }
}
