uniform mat4 inverseModelView;
uniform vec4 eyePosition;
uniform vec4 lightVector;
uniform mat4 boneMatrices[2];

attribute vec4 weights;
attribute vec4 matrixIndices;
attribute vec4 numBones;

void directionalLight( in int i,
                       in vec3 N,
                       in float shininess,
                       inout vec4 ambientComponent,
                       inout vec4 diffuseComponent,
                       inout vec4 specularComponent )
{
    vec3 L = normalize(gl_LightSource[i].position.xyz);
    vec3 D = normalize(gl_LightSource[i].halfVector.xyz);  

    ambientComponent  *= gl_LightSource[i].ambient;
    diffuseComponent  *= gl_LightSource[i].diffuse * max(dot(N, L), 0.0);
    specularComponent *= gl_LightSource[i].diffuse * pow(max(dot(N, D), 0.0), shininess);
}

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
    //vec3 finalNormal = (normalize( gl_ModelViewMatrixInverse * tempNormal )).xyz;
    vec3 finalNormal = (normalize( inverseModelView * tempNormal )).xyz;

    vec4 ambientComponent  = gl_FrontMaterial.ambient;
    vec4 diffuseComponent  = gl_FrontMaterial.diffuse;
    vec4 specularComponent = gl_FrontMaterial.specular;
    vec4 emmisiveComponent = gl_FrontMaterial.emission;

    directionalLight( 0, finalNormal, gl_FrontMaterial.shininess,
                      ambientComponent, diffuseComponent, specularComponent );

    gl_FrontColor = ambientComponent + diffuseComponent + specularComponent + emmisiveComponent;
}
