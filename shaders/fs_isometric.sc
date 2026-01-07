$input v_texcoord0, v_color0, v_worldPos

#include "bgfx_shader.sh"

void main()
{
    // Muestrear la textura
    vec4 texColor = texture2D(s_texColor, v_texcoord0);

    // Multiplicar por el color del vértice
    gl_FragColor = texColor * v_color0;
}
