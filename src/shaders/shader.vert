#version 120

attribute vec3 position;
//out vec2 textureCoord;


void main() 
{
//    outPos = position;
    gl_Position = vec4(position, 1.0);
}
