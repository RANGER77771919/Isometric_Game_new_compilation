// Vertex shader con instancing para tiles isométricos
#version 330

// Input del quad base (4 vértices)
in vec2 a_position;      // Posición del quad (0-1)
in vec2 a_texcoord;      // UV coordinates

// Input de instancias (por tile)
in vec3 i_position;      // Posición mundial del tile
in vec4 i_color;         // Color del tile (ABGR)

// Output al fragment shader
out vec2 v_texcoord;
out vec4 v_color;

// Uniforms (set by BGFX)
uniform mat4 u_modelViewProj;

void main()
{
    // Calcular posición mundial del vértice
    vec3 worldPos = vec3(a_position.x, a_position.y, 0.0) + i_position;

    // Transformar a clip space
    gl_Position = u_modelViewProj * vec4(worldPos, 1.0);

    // Pasar datos al fragment shader
    v_texcoord = a_texcoord;
    v_color = i_color;
}
