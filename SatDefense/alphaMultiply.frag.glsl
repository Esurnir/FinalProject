#version 330 core
 
in vec2 ex_teC;
out vec4 out_colour;
uniform sampler2D dSampler;

void main(void)
{
	out_colour = vec4(texture(dSampler, ex_teC).rgb * texture(dSampler,ex_teC).a,1);
}