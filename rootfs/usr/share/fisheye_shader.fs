precision mediump float;
varying vec2 v_texcoord;
uniform sampler2D tex;
void main () {
  vec2 texturecoord = v_texcoord.xy;
  vec2 normcoord;
  normcoord = texturecoord - 0.5;
  float r = length (normcoord);
  normcoord *= r * 1.414;
  texturecoord = normcoord + 0.5;
  vec4 color = texture2D (tex, texturecoord);
  gl_FragColor = color;
}

