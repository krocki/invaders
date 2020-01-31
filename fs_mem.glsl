#version 410
in vec2 coords;
uniform sampler2D tx;
uniform int mode;
out vec4 frag_color;
void main () {
  vec2 f_coords = coords.xy; 
  if (mode==1) {
    f_coords = coords.yx;
 };
  frag_color = texture (tx, vec2(f_coords.x, f_coords.y));
}
