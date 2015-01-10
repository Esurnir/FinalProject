#version 420 core
//stolen from https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms because the original sample used assembly for its filter and i can't stand that

in vec2 ex_teC;
out vec4 out_colour;

uniform sampler2D tex0;
uniform vec2 pixelOffset;



void main(void)                                                                          
{                                                                                                                                                                   
    vec3 colOut = vec3( 0, 0, 0 );                                                                                                                                                                                                                                                   
    ////////////////////////////////////////////////;
    // Kernel width 35 x 35
    //
    const int stepCount = 9;
    //
	const float gWeights[stepCount] = float[stepCount](
       0.10855,
       0.13135,
       0.10406,
       0.07216,
       0.04380,
       0.02328,
       0.01083,
       0.00441,
       0.00157
    );
    const float gOffsets[stepCount] = float[stepCount](
       0.66293,
       2.47904,
       4.46232,
       6.44568,
       8.42917,
       10.41281,
       12.39664,
       14.38070,
       16.36501
    );
    ////////////////////////////////////////////////;
    for( int i = 0; i < stepCount; i++ )                                                                                                                            
    {                                                                                                                                                               
        vec2 texCoordOffset = gOffsets[i] * pixelOffset;    
		vec3 col = texture(tex0, ex_teC + texCoordOffset).xyz + texture(tex0, ex_teC - texCoordOffset).xyz;
        colOut += gWeights[i] * col;                                                                                                                              
    }                                                                                                                                                               
 
    out_colour = vec4(colOut,1);

}                      
