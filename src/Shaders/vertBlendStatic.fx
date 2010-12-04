////////////////////////////////////////////////////////////////////////////
// 
// File: vertBlendStatic.fx
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Vertex blending vertex shader.  Hardcoded to supports 2 bone 
//       influences per vertex.  
//          
////////////////////////////////////////////////////////////////////////////
// 2007, Modified by Geoyeob Kim


extern float4x4 WorldViewProj;
extern float4x4 FinalTransforms[10]; // bone count
extern texture  Tex;

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

VS_OUTPUT VertexBlend(float4 pos         : POSITION0,
                      float2 texCoord    : TEXCOORD0,
                      float  weight0     : BLENDWEIGHT0,
                      int4   boneIndices : BLENDINDICES0)
{

    VS_OUTPUT output = (VS_OUTPUT)0;

    float weight1 = 1.0f - weight0;
    
    float4 p = 1 * mul(pos, FinalTransforms[boneIndices[0]]);
    p       += 0 * mul(pos, FinalTransforms[boneIndices[1]]);
    p.w = 1.0f;
	
    output.pos      = mul(p, WorldViewProj);
    //output.pos = mul(pos, WorldViewProj);
    output.texCoord = texCoord;
    output.diffuse  = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return output;
}

technique VertexBlendingTech
{
    pass P0
    {
        vertexShader = compile vs_1_1 VertexBlend();
        Sampler[0] = <S0>;
        
        Lighting = false;
    }
}