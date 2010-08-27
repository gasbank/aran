#version 150
uniform sampler2D ShadowMap;
out vec4 gl_FragColor;

in vec4 ShadowCoord;
//in vec4 gl_Color;
in vec4 out_Color;

void main()
{	
	vec4 shadowCoordinateWdivide = ShadowCoord / ShadowCoord.w ;
	
	// Used to lower moiré pattern and self-shadowing
	shadowCoordinateWdivide.z += 0.0001;
	
	
	float distanceFromLight = texture2D(ShadowMap,shadowCoordinateWdivide.st).z;
	
	
 	float shadow = 1.0;
 	if (ShadowCoord.w > 0.0)
 		shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0 ;
  	
	vec4 tempcol = vec4(0,0,0,0);
    gl_FragColor = shadow * out_Color;
  	//gl_FragColor = tempcol + out_Color + 1e-10*(shadow * out_Color);
    //gl_FragColor = vec4(1,0,0,1);
  
}

