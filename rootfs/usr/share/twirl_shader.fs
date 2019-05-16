precision mediump float;
varying vec2 v_texcoord;
uniform sampler2D tex;
void main () {
  vec2 texturecoord = v_texcoord.xy;
  vec2 normcoord;
  normcoord = texturecoord - 0.5;
  float r = length (normcoord);
  // calculate rotation angle: maximum (about pi/2) at the origin and
  // gradually decrease it up to 0.6 of each quadrant
  float phi = (1.0 - smoothstep (0.0, 0.3, r)) * 1.6;
  // precalculate sin phi and cos phi, save some alu
  float s = sin(phi);
  float c = cos(phi);
  // rotate
  normcoord *= mat2(c, s, -s, c);
  texturecoord = normcoord + 0.5;
  vec4 color = texture2D (tex, texturecoord);
  gl_FragColor = color;
}

