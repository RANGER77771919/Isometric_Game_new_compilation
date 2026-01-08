// Fragment shader para tiles isométricos
#version 330

// Input del vertex shader
in vec2 v_texcoord;
in vec4 v_color;

// Output
out vec4 fragColor;

void main()
{
    // Por ahora solo color sólido (sin texturas)
    fragColor = v_color;
}
