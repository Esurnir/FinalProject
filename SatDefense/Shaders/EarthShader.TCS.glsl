#version 420 core                                                                               
                                                                                                
// define the number of CPs in the output patch                                                 
layout (vertices = 3) out;                                                                      
                                                                                                                                                                     
                                                                                                
// attributes of the input CPs                                                                  
     
in vec3 cs_Normal[];//ex_Normal;
in vec2 cs_texCoord[];// ex_texCoord;
in vec3 cs_ex_eye[];// ex_eye;               
in vec3 cs_wPos[];
in float fresnelTFactor[]; 
uniform bool tess;
                                                                                                
// attributes of the output CPs      
out vec3 es_Normal[];
out vec2 es_texCoord[];
out vec3 es_ex_eye[];
out vec3 es_wPos[];                                                                           
                                                                                                
// float GetTessLevel(float Distance0, float Distance1)                                            
// {                                                                                               
    // float AvgDistance = (Distance0 + Distance1) / 2.0;                                          
                                                                                                
    // if (AvgDistance <= 2.0) {                                                                   
        // return 10.0;                                                                            
    // }                                                                                           
    // else if (AvgDistance <= 5.0) {                                                              
        // return 7.0;                                                                             
    // }                                                                                           
    // else {                                                                                      
        // return 3.0;                                                                             
    // }                                                                                           
// }                                                                                               
                                                                                                
void main()                                                                                     
{                                                                                               
    // Set the control points of the output patch   
	
	es_Normal[gl_InvocationID] = cs_Normal[gl_InvocationID];
	es_texCoord[gl_InvocationID] = cs_texCoord[gl_InvocationID];
	es_ex_eye[gl_InvocationID] = cs_ex_eye[gl_InvocationID];
	es_wPos[gl_InvocationID] = cs_wPos[gl_InvocationID];

	float normaldot[3];
	normaldot[0] = dot(es_Normal[0], vec3(0, 0, 1));
	normaldot[1] = dot(es_Normal[1], vec3(0, 0, 1));
	normaldot[2] = dot(es_Normal[2], vec3(0, 0, 1));
	if (tess){
		if (normaldot[0] < 0 && normaldot[1] < 0 && normaldot[2] < 0) {
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;
			gl_TessLevelInner[0] = 0;
		}
		else {
			gl_TessLevelOuter[0] = ((fresnelTFactor[1] + fresnelTFactor[2]) / 2);
			gl_TessLevelOuter[1] = ((fresnelTFactor[0] + fresnelTFactor[2]) / 2);
			gl_TessLevelOuter[2] = ((fresnelTFactor[0] + fresnelTFactor[1]) / 2);
			gl_TessLevelInner[0] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[1] + gl_TessLevelOuter[2]) / 3;
		}
		//gl_TessLevelOuter[0] = normaldot[1] < 0 || normaldot[2] < 0 ? 0 : gl_TessLevelOuter[0];
		//gl_TessLevelOuter[1] = normaldot[0] < 0 || normaldot[2] < 0 ? 0 : gl_TessLevelOuter[0];
		//gl_TessLevelOuter[2] = normaldot[1] < 0 || normaldot[0] < 0 ? 0 : gl_TessLevelOuter[0];

	}
	else {
		gl_TessLevelOuter[0] = 1;
		gl_TessLevelOuter[1] = 1;
		gl_TessLevelOuter[2] = 1;
		gl_TessLevelInner[0] = 1;
	}
	
	                         
                                                                                                
    //Calculate the distance from the camera to the three control points                       
    // float EyeToVertexDistance0 = distance(gEyeWorldPos, WorldPos_ES_in[0]);                     
    // float EyeToVertexDistance1 = distance(gEyeWorldPos, WorldPos_ES_in[1]);                     
    // float EyeToVertexDistance2 = distance(gEyeWorldPos, WorldPos_ES_in[2]);                     
                                                                                                
    ////Calculate the tessellation levels                                                        
    // gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);            
    // gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance2, EyeToVertexDistance0);            
    // gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance0, EyeToVertexDistance1);            
    // gl_TessLevelInner[0] = gl_TessLevelOuter[2];                                                
}                                                                                               