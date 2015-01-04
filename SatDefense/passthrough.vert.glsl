#version 330 core
 
layout(location=0) in vec2 in_Position;
layout(location=1) in vec2 in_texCoord;
out vec2 ex_teC;


void main(void)
{
	gl_Position = vec4(in_Position,0.0,1.0);
	ex_teC = in_texCoord;
}