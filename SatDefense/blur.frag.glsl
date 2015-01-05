#version 330 core
//stolen from https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms because the original sample used assembly for its filter and i can't stand that

in vec2 ex_teC;
out vec4 out_colour;

uniform sampler2D tex0;
uniform vec2 pixelOffset;



void main(void)                                                                          
{                                                                                                                                                                   
    vec3 colOut = vec3( 0, 0, 0 );                                                                                                                                                                                                                                                   
    ////////////////////////////////////////////////;
    // Kernel width 7 x 7
    //
    const int stepCount = 2;
    //
    const float gWeights[stepCount] ={
       0.44908,
       0.05092
    };
    const float gOffsets[stepCount] ={
       0.53805,
       2.06278
    };
    ////////////////////////////////////////////////;
    for( int i = 0; i < stepCount; i++ )                                                                                                                            
    {                                                                                                                                                               
        vec2 texCoordOffset = gOffsets[i] * pixelOffset;    
		vec3 col = texture(tex0, ex_teC + texCoordOffset).xyz + texture(tex0, ex_teC + texCoordOffset).xyz;
        colOut += gWeights[i] * col;                                                                                                                              
    }                                                                                                                                                               
 
    out_colour = vec4(colOut,1);

}                      
