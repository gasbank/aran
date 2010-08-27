#version 120

precision highp float;
in  vec3 in_Position;
in  vec3 in_Color;
in  vec3 in_Normal;
varying vec4 out_Color;
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat4 ShadowTexMatrix;

/* transpose of the inverse of the
 * upper leftmost 3x3 of gl_ModelViewMatrix */
uniform mat3 normal_matrix; /* Note mat3, not mat4 */

// Used for shadow lookup
varying vec4 ShadowCoord;

void main() {
    vec3 normal, lightDir;
    vec4 diffuse;
    vec4 lightCol;
    float NdotL;
    
    lightCol = vec4(1,1,1,1);
    
    ShadowCoord = ShadowTexMatrix * vec4(in_Position,1);
    
    /* first transform the normal into eye space and normalize the result */
    normal = normalize(gl_NormalMatrix * in_Normal);
    //normal = normalize(normal_matrix * vec3(0,0,1));
    //normal = normalize(gl_NormalMatrix * vec3(0,0,1));
    
    /* now normalize the light's direction. Note that according to the
    OpenGL specification, the light is stored in eye space. Also since 
    we're talking about a directional light, the position field is actually 
    direction */
    //lightDir = normalize(vec3(gl_LightSource[0].position));
    lightDir = normalize(vec3(0, 0, 1));
    
    /* compute the cos of the angle between the normal and lights direction. 
    The light is directional so the direction is constant for every vertex.
    Since these two are normalized the cosine is the dot product. We also 
    need to clamp the result to the [0,1] range. */
    NdotL = max(dot(normal, lightDir), 0.0);
    
    /* Compute the diffuse term */
    diffuse = vec4(in_Color,1) * lightCol;
    //diffuse = vec4(1,0,0,1) * lightCol;
    
    gl_FrontColor =  NdotL * diffuse;
    
    //gl_Position = projection_matrix * modelview_matrix * gl_Vertex;
    gl_Position = projection_matrix * modelview_matrix * vec4(in_Position, 1);
    //gl_Position = projection_matrix * vec4(in_Position, 1);
    //gl_Position = vec4(in_Position);
    
    out_Color = NdotL * diffuse;
}
