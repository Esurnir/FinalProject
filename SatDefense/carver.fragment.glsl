#version 420 core

in vec3 ex_Normal;
in vec2 ex_TexCoord;
in vec3 ex_Eye;
//in vec3 debugNormal;
layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_specular;

uniform vec3 lDir;
uniform sampler2D texImage;
uniform sampler2D specImage;
uniform bool specbloom;
uniform bool lighting;

struct material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};
material mymaterial = material(
	vec4(0.2, 0.2, 0.2, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(.7, .7, .7, 1.0),
	50.0
	);

void main(void)
{
	if (dot(ex_Eye, ex_Normal) < 0) discard;
	// set the specular term to black
	vec4 spec = vec4(0.0);

	// normalize both input vectors
	vec3 n = normalize(ex_Normal);
	vec3 e = normalize(ex_Eye);
	vec3 light = normalize(lDir);

	float intensity = max(dot(n, light), 0.0);
	
	

	vec3 h = normalize(light + e);
	// compute the specular term into spec
	float intSpec = max(dot(h, n), 0.0);
	float specfactor = pow(intSpec, mymaterial.shininess);
	spec = vec4(texture(specImage, ex_TexCoord).rgb * specfactor,1);


	vec4 texColor = texture(texImage, ex_TexCoord);
	vec4 diffColor = intensity * mymaterial.diffuse * texColor;
	vec4 ambientColor = mymaterial.ambient * texColor;
	
	out_Color = lighting? max(diffColor , ambientColor) + spec : texColor;
	out_specular = (specbloom && lighting ? spec:vec4(0));

}
