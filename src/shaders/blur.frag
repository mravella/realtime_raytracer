#version 400 core

out vec4 outColor;
uniform sampler2D tex;
uniform float width;
uniform float height;
uniform float focalDepth;



void main(void)
{
	vec4 textureColor = texture2D(tex, gl_FragCoord.xy / vec2(width, height));
	float depth = textureColor.a;
	vec3 c = textureColor.rgb;

	const int mSize = 15;
	const int kSize = (mSize-1)/2;
	float kernel[mSize];

	vec3 col = vec3(0.0);
	
	float sigma = 7.0;
	float Z = 0.0;
	for (int j = 0; j <= kSize; ++j)
	{
		kernel[kSize+j] = kernel[kSize-j] = exp(-0.5*float(j)*float(j)/(sigma*sigma))/sigma;
	}
	
        int usingSize = int(pow(abs(focalDepth - depth), 0.3) * 20.0);
	int kUsingSize = (usingSize - 1) / 2;
	
	for (int i=-kUsingSize; i <= kUsingSize; ++i)
	{
		for (int j=-kUsingSize; j <= kUsingSize; ++j)
		{
			col += kernel[kUsingSize+j]*kernel[kUsingSize+i]*texture2D(tex, (gl_FragCoord.xy+vec2(float(i),float(j))) / vec2(width, height)).rgb;
			Z += kernel[kUsingSize+j]*kernel[kUsingSize+i];
		}
	}
	
	
	outColor = vec4(col/(Z), 1.0);

}
