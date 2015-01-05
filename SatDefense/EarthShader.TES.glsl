#version 410 core                                                                               
                                                                                                
layout(triangles, equal_spacing, ccw) in;                                                       
                                                                                                
uniform mat4 gVP;                                                                               
uniform sampler2D gDisplacementMap;                                                             
uniform vec3 gDispFactor;
                                                                                                

in vec3 es_Normal[];
in vec2 es_texCoord[];
in vec3 es_ex_eye[];
in vec3 es_wPos[];                                                                          
                           

out vec3 ex_Normal;
out vec2 ex_texCoord;
out vec3 ex_eye;						   
                                                                        
                                                                                                
vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)                                                   
{                                                                                               
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;   
}                                                                                               
                                                                                                
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)                                                   
{                                                                                               
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;   
}                                                                                               
                                                                                                
void main()                                                                                     
{                                                                                               
    // Interpolate the attributes of the output vertex using the barycentric coordinates        
    ex_texCoord = interpolate2D(es_texCoord[0], es_texCoord[1], es_texCoord[2]);    
    ex_Normal = interpolate3D(es_Normal[0], es_Normal[1], es_Normal[2]);            
    ex_Normal = normalize(ex_Normal);                                      
	ex_eye = interpolate3D(es_ex_eye[0], es_ex_eye[1], es_ex_eye[2]);
    vec3 position = interpolate3D(es_wPos[0], es_wPos[1], es_wPos[2]);    
                                                                                                
    // Displace the vertex along the normal                                                     
    float Displacement = 2*(texture(gDisplacementMap, ex_texCoord.xy).x-0.5);                        
    position += ex_Normal * Displacement * gDispFactor;                                
    gl_Position = gVP * vec4(position, 1.0);                                              
}                                                                                               
