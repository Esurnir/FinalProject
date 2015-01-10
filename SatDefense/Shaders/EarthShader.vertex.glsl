#version 420 core
 
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_texCoord;
out vec3 cs_Normal;//ex_Normal;
out vec2 cs_texCoord;// ex_texCoord;
out vec3 cs_ex_eye;// ex_eye;
out vec3 cs_wPos;
out float fresnelTFactor;

 
uniform mat3 NormalInvMatrix;
uniform mat3 matricialInverse;
uniform mat4 mvMatrix;
uniform mat4 mMatrix;





void main(void)
{

	vec4 p = vec4(in_Position, 1.0);
	//mvpMatrix * p;
	vec3 test = NormalInvMatrix*in_Normal;
	cs_Normal = normalize(test);
	cs_ex_eye = normalize(vec3(-(mvMatrix*p)));
	fresnelTFactor = 0.5+2.5*pow((1 - abs(dot(-cs_ex_eye, cs_Normal))),2); //5 * pow(1 - abs(dot(-cs_ex_eye, cs_Normal)), 1.5);
	cs_texCoord = in_texCoord;
	cs_wPos = vec3(mvMatrix * p);
}