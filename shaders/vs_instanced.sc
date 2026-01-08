$input a_position, a_texcoord0, a_color0
$input i_texcoord1, i_color1  // Instance data: position (xyz+w), color
$output v_texcoord0, v_color0

#include <bgfx_shader.sh>

void main()
{
    // Calcular posición mundial del vértice usando datos de instancing
    vec3 worldPos = a_position + i_texcoord1.xyz;

    gl_Position = mul(u_modelViewProj, vec4(worldPos, 1.0));
    v_texcoord0 = a_texcoord0;
    v_color0 = i_color1;
}
