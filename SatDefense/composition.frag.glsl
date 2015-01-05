#version 330 core
 
in vec2 ex_teC;
out vec4 out_colour;
uniform sampler2D sceneSampler;
uniform sampler2D blurSampler;

vec4 radial(sampler2D tex, vec2 texcoord, int samples, float startScale , float scaleMul ) {
	vec4 c = vec4(0);
	float scale = startScale;
	for (int i=0; i < samples; i++) {
		vec2 uv = ((texcoord-vec2(0.5))*scale)+vec2(0.5);
		vec4 s = texture(tex,uv);
		c +=s;
		scale *=scaleMul;
	}
	c/=samples;
	return c;
}

// float4 radial(sampler2D tex,
              // float2 texcoord,
              // int samples,
              // float startScale = 1.0,
              // float scaleMul = 0.9)
// {
    // float4 c = 0;
    // float scale = startScale;
    // for(int i=0; i<samples; i++) {
        // float2 uv = ((texcoord-0.5)*scale)+0.5;
        // float4 s = tex2D(tex, uv);
        // c += s;
        // scale *= scaleMul;
    // }
    // c /= samples;
    // return c;
// }

void main(void)
{
	float blurAmount = 0.5;
	float effectAmount = 1;
	vec4 scene = texture(sceneSampler,ex_teC);
	vec4 blurred = texture(blurSampler,ex_teC);
	vec4 effect = radial(blurSampler,ex_teC,30,1.0,0.95);
	
	vec4 c = scene + blurred;
	
	c+= effect*effectAmount;
	
	out_colour = c;
}



// float4 main(float2 texcoord : TEXCOORD0,
            // uniform sampler2D   sceneTex    : TEXUNIT0,
            // uniform sampler2D   blurTex     : TEXUNIT1,
            // uniform sampler2D   starTex     : TEXUNIT2,
            // uniform sampler2D   starTex1    : TEXUNIT3,
            // uniform sampler2D   starTex2    : TEXUNIT4,
			// uniform float       blurAmount,
			// uniform float       effectAmount = 0.1,
            // uniform float       exposure,
            // uniform float       gamma = 1.0 / 2.0,
            // uniform float2      windowSize
            // ) : COLOR
// {
    //sum original and blurred image
    // float4 scene = tex2D(sceneTex, texcoord);
    // float4 blurred = tex2D(blurTex, texcoord);
	// float4 effect = radial(blurTex, texcoord, 30, 1.0, 0.95);

    // float4 c = lerp(scene, blurred, blurAmount);

	// c += effect*effectAmount;

    //exposure
    // c = c * exposure;
    
    //vignette effect
    // c *= vignette(texcoord*2-1, 0.7, 1.5);

    //gamma correction
    // c.rgb = pow(c.rgb, gamma);

    // return c;
// }