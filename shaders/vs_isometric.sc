$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_color0, v_worldPos

#include "bgfx_shader.sh"

void main()
{
    // Transformar posición usando matriz de modelo-vista-proyección
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

    // Pasar coordenadas de textura al fragment shader
    v_texcoord0 = a_texcoord0;

    // Pasar color al fragment shader
    v_color0 = a_color0;

    // Pasar posición mundial para efectos en fragment shader
    v_worldPos = a_position;
}
