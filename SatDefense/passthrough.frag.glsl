#version 330 core
 
in vec2 ex_teC;
out vec4 out_colour;
uniform sampler2D ptSampler;

void main(void)
{
	out_colour = texture(ptSampler, ex_teC);
}