precision mediump float;
uniform sampler2D tex;
uniform float width;
uniform float height;
varying vec2 v_texcoord;
void main()
{
    vec2 topcoord, botcoord;
    vec4 cur_color, top_color, bot_color;
    topcoord.x = v_texcoord.x;
    botcoord.x = v_texcoord.x;
    if (v_texcoord.y == 0.0 || v_texcoord.y == 1.0) {
        topcoord.y = v_texcoord.y ;
        botcoord.y = v_texcoord.y ;
    }
    else {
        topcoord.y = v_texcoord.y - 1.0/height;
        botcoord.y = v_texcoord.y + 1.0/height;
    }

    cur_color = texture2D(tex, v_texcoord);
    top_color = texture2D(tex, topcoord);
    bot_color = texture2D(tex, botcoord);

    gl_FragColor = 0.5*cur_color + 0.25*top_color + 0.25*bot_color; 
}

