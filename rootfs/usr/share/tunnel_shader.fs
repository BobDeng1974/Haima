precision mediump float;
varying vec2 v_texcoord;
uniform sampler2D tex;
void main () {
  vec2 texturecoord = v_texcoord.xy;
  vec2 normcoord;
  normcoord = (texturecoord - 0.5);
  float r = length(normcoord);
  normcoord *= clamp (r, 0.0, 0.275) / r;
  texturecoord = normcoord + 0.5;
  vec4 color = texture2D (tex, texturecoord); 
  gl_FragColor = color;
}

