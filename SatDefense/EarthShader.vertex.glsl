#version 330 core
 
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_texCoord;
out vec3 ex_Normal;
out vec2 ex_texCoord;
out vec4 ex_eye;
out vec3 debugNormal;
 
uniform mat3 NormalInvMatrix;
uniform mat3 matricialInverse;
uniform mat4 mvMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 mvpMatrix;





void main(void)
{
	debugNormal = (in_Normal + 1) * 0.5;
	vec4 p = vec4(in_Position, 1.0);
	gl_Position = mvpMatrix * p;
	vec3 test = NormalInvMatrix*in_Normal;
	ex_Normal = normalize(test);
	ex_texCoord = in_texCoord ;
	ex_eye = -(mvMatrix*p);

}