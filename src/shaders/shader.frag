#version 330 core

in vec3 outPos;

out vec4 outColor;

void main()
{
    outColor = vec4(outPos, 1.0);
}
