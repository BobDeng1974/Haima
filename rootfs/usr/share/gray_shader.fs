precision mediump float;
varying vec2 v_texcoord;
uniform sampler2D tex;
void main () {
  vec4 color = texture2D (tex, v_texcoord.xy);
  float y = dot(color.rgb, vec3(0.2125, 0.7154, 0.0721));
  color.r = y;
  color.g = y;
  color.b = y;
  gl_FragColor = color;
}

