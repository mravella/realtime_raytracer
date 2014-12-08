#version 400 core

out vec4 outColor;
uniform sampler2D render;
uniform sampler2D noise;
uniform float height;
uniform float width;
uniform float noiseWidth;
uniform float noiseHeight;

#define NUMSAMPLES 8

void main() 
{
	vec4 renderColor = texture2D(render, gl_FragCoord.xy / vec2(width, height));

	outColor = renderColor;
	return;

	float zValue = 1.0 - renderColor.a;

	float ao = 0.0;


	for (int i = 0; i < NUMSAMPLES; i++)
	{
		vec2 off = -1.0 + 2.0 * texture2D(noise, (gl_FragCoord.xy + 23.71 * float(i)) / vec2(noiseWidth, noiseHeight)).xy;

		float z = 1.0 - texture2D(render, (gl_FragCoord.xy + floor(off * 16.0)) / vec2(width, height)).a;

		ao += clamp((zValue - z) / 0.1, 0.0, 1.0);
	}
	ao = clamp(1.0 - ao / 8.0, 0.0, 1.0);

	vec3 col = vec3(ao) * renderColor.xyz;

	outColor = vec4(col, 1.0);
}