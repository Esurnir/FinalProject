#version 330 core

in vec3 ex_Normal;
in vec2 ex_texCoord;
in vec4 ex_eye;
in vec3 debugNormal;
out vec4 out_Color;

uniform vec3 lDir;
uniform sampler2D texImage;
uniform sampler2D nightImage;
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
	vec4(1.0, 1.0, 1.0, 1.0),
	50.0
	);
 
void main(void)
{
	// set the specular term to black
	vec4 spec = vec4(0.0);
	vec4 nightColor = vec4(0.0);

	// normalize both input vectors
	vec3 n = normalize(ex_Normal);
	vec3 e = normalize(vec3(ex_eye));
	vec3 light = normalize(lDir);

	float intensity = max(dot(n, light), 0.0);
	
	// if the vertex is lit compute the specular color
	if (intensity > 0.0) {
		// compute the half vector
		vec3 h = normalize(light + e);
		// compute the specular term into spec
		float intSpec = max(dot(h, n), 0.0);
		spec = mymaterial.specular * pow(intSpec, mymaterial.shininess);
	}

	if (intensity < 0.2) {
		float fudgeFactor = 1-max(intensity*5,0);
		nightColor = fudgeFactor * texture(nightImage,ex_texCoord);
	}
	vec4 texColor = texture(texImage, vec2(ex_texCoord.s, ex_texCoord.t));
	vec4 diffColor = intensity * mymaterial.diffuse * texColor;
	

	out_Color = max(diffColor + spec + nightColor, nightColor);

}
