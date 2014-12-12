#version 400 core

out vec4 outColor;
uniform sampler2D tex;
uniform float width;
uniform float height;



void main(void)
{	
	outColor = texture2D(tex, vec2(gl_FragCoord.x, height - gl_FragCoord.y) / vec2(width, height)).bgra;

}
