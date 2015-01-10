#version 420 core
 
in vec2 ex_teC;
out vec4 out_colour;
uniform sampler2D dSampler;
uniform sampler2D specularSampler;

void main(void)
{
	vec2 texCoordinate = ex_teC; //((ex_teC - vec2(0.5))*0.95) + vec2(0.5);

	out_colour = vec4(vec3(0.5, 0.5, 1)* texture(dSampler, texCoordinate).a, 1) + texture(dSampler, texCoordinate)*texture(specularSampler,texCoordinate).r;
}