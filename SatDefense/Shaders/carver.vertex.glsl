#version 420 core
 
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_TexCoord;
out vec3 ex_Normal;//ex_Normal;
out vec2 ex_TexCoord;// ex_texCoord;
out vec3 ex_Eye;// ex_eye;

 
uniform mat3 normalMatrix;
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;




void main(void)
{

	vec4 p = vec4(in_Position, 1.0);
	gl_Position = mvpMatrix * p;
	ex_Normal = normalize(normalMatrix*in_Normal);
	ex_Eye = normalize(vec3(-(mvMatrix*p)));
	ex_TexCoord = in_TexCoord;
}