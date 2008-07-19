////////////////////////////////////////////////////////////////////////////
// 
// File: vertblendDynamic.fx
// 
// Author: Frank Luna
//
// Desc: Vertex blending vertex shader.  Supports meshes with 2-4 bone 
//       influences per vertex.  We can dynamically set NumVertInfluences
//       so that the shader knows how many weights it is processing
//       per vertex.  In order to support dynamic loops, we must use
//       at least vertex shader version 2.0.
//          
// 2008, Modified by Geoyeob Kim
//
////////////////////////////////////////////////////////////////////////////



extern float4x4 WorldViewProj;
extern float4x4 FinalTransforms[35];
extern texture  Tex;
extern int NumVertInfluences = 3; // <--- Normally set dynamically.
extern float TestFloatArray[2000];

sampler S0 = sampler_state
{
    Texture = <Tex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


struct VS_OUTPUT
{
    float4 pos      : POSITION0;
    float2 texCoord : TEXCOORD;
    float4 diffuse  : COLOR0;
};

VS_OUTPUT main(float4 pos         : POSITION0,
			  float4 normal		 : NORMAL0,
              float2 texCoord    : TEXCOORD0,
              float4  weights    : BLENDWEIGHT0,
              int4   boneIndices : BLENDINDICES0)
{
    VS_OUTPUT output = (VS_OUTPUT)0;


    float4 p = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float p0 = pos[0];
    float p1 = pos[1];
    float p2 = pos[2];
    float p3 = pos[3];

    float lastWeight = 0.0f;
    //int n = NumVertInfluences;
    int i = 0;

    int idx0 = boneIndices[0] + p0 - p0;
    int idx1 = boneIndices[1] + p1 - p1;
    int idx2 = boneIndices[2] + p2 - p2;
    int idx3 = boneIndices[3] + p3 - p3;


    float weights0 = weights[0];
    float weights1 = weights[1];
    float weights2 = weights[2];
    float weights3 = weights[3];
    

    float weightsTotal = weights0 + weights1 + weights2 + weights3;


	/*
    for(i = 0; i < 3; i++)
    {
        lastWeight += weights[i];
		p += weights[i] * mul(pos, FinalTransforms[boneIndices[i]]);
    }

    lastWeight = 1.0f - lastWeight;
	p += lastWeight * mul(pos, FinalTransforms[boneIndices[NumVertInfluences]]);
	*/


    p += weights0 * mul( pos, FinalTransforms[idx0] );
    p += weights1 * mul( pos, FinalTransforms[idx1] );
    p += weights2 * mul( pos, FinalTransforms[idx2] );

    lastWeight = 1.0f - weights0 - weights1 - weights2 - weights3 + weights3;

    p += lastWeight * mul( pos, FinalTransforms[idx3] );
    //p += weights[3] * mul( pos, FinalTransforms[boneIndices[3]] );

    p.w = 1.0f;

    output.pos      = mul(p, WorldViewProj);
    output.texCoord = texCoord;
    output.diffuse  = float4(1.0f, 1.0f, 5.0f, 1.0f);// * mul(normal, float4(1.0f, 1.0f, 1.0f, 1.0f));

    return output;
}


technique VertexBlendingTech
{
    pass P0
    {
        vertexShader = compile vs_2_0 main();
        Sampler[0] = <S0>;
        
        //Lighting = true;
    }
}

