uniform mat4 inverseModelView;
uniform vec4 eyePosition;
uniform vec4 lightVector;
uniform mat4 boneMatrices[32];

attribute vec4 weights;
attribute vec4 matrixIndices;
attribute vec4 numBones;

void main( void )
{
    vec4 index  = matrixIndices;
    vec4 weight = weights;

    vec4 normal      = vec4( gl_Normal.xyz, 0.0 );
    vec4 position    = vec4( 0.0, 0.0, 0.0, 0.0 );
    vec4 tempNormal  = vec4( 0.0, 0.0, 0.0, 0.0 );

    for( float i = 0.0; i < numBones.x; i += 1.0 )
    {
        // Apply influence of bone i
        position = position + weight.x * (boneMatrices[int(index.x)] * gl_Vertex);
        
        // Transform normal by bone i
        tempNormal = tempNormal + weight.x * (boneMatrices[int(index.x)] * normal);

        // shift over the index/weight variables, this moves the index and 
        // weight for the current bone into the .x component of the index 
        // and weight variables
        index  = index.yzwx;
        weight = weight.yzwx;
    }

    gl_Position = gl_ModelViewProjectionMatrix * position;

    //
    // Compute N·L directional lighting with a standard material model...
    //

    // gl_ModelViewMatrixInverse only works on nVIDIA cards so we'll need to 
    // pass our own inverse model-view matrix until ATI adds support for it.
    vec3 finalNormal = (normalize( gl_ModelViewMatrixInverse * tempNormal )).xyz;
    //vec3 finalNormal = (normalize( inverseModelView * tempNormal )).xyz;

	vec3 lightDir = normalize( lightVector.xyz );
	vec3 halfVec = normalize( eyePosition.xyz - lightDir );
	float specularPower = 0.0;

    //vec4 coeffs = lit( dot(finalNormal,-lightDir), dot(finalNormal,halfVec), specularPower );
    // The lit function above only works on nVIDIA cards, so we'll need to use
    // the code below, which does the same thing, so we can run on ATI cards.
    float diffuse  = dot( finalNormal, -lightDir );
    float specular = dot( finalNormal, halfVec );
    vec4 coeffs;
    coeffs.x = 1.0;
    coeffs.y = max( diffuse, 0.0 );
    coeffs.z = min( diffuse, specular );
    if( coeffs.z < 0.0 )
        coeffs.z = 0.0;
    else
        coeffs.z = pow( specular, specularPower );
    coeffs.w = 1.0;

	vec4 AmbLight  = vec4( 0.0, 0.0, 0.0, 1.0 );
	vec4 DiffLight = vec4( 1.0, 1.0, 1.0, 1.0 );
	vec4 SpecLight = vec4( 1.0, 1.0, 1.0, 1.0 );

	vec4 AmbMat  = vec4( 0.2f, 0.2f, 0.2f, 1.0 );
	vec4 DiffMat = gl_Color; // Use the vertex's color for the diffuse material.
	vec4 SpecMat = vec4( 0.0, 0.0, 0.0, 1.0 );
	vec4 EmisMat = vec4( 0.0, 0.0, 0.0, 1.0 );

	// Compute the standard shading model
	vec4 outCol = AmbLight  * AmbMat +              //ambient term
		          DiffLight * DiffMat * coeffs.y +  //diffuse term
		          SpecLight * SpecMat * coeffs.z;   //specular term

	// Add the emmisive material last...
	gl_FrontColor = outCol + EmisMat;
	gl_FrontColor.a = 0.5;
	
	//gl_Position = gl_ModelViewProjectionMatrix * boneMatrices[0] * gl_Vertex;
	//gl_Position = gl_ModelViewProjectionMatrix * boneMatrices[1] * gl_Vertex;
	//gl_Position = gl_ModelViewProjectionMatrix * boneMatrices[2] * gl_Vertex;
	//gl_FrontColor = vec4(1, 0, 0, 0.5);
}
