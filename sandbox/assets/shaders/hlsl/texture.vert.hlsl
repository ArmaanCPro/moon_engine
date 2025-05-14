#include "rootsig.hlsli"

float4x4 u_VP : register(b0);

[RootSignature(ROOTSIG)]
void main(
    // IN
    in float3 a_Position : POSITION,
    in float4 a_Color : COLOR,
    in float2 a_TexCoord : TEXCOORD,
    in float a_TexIndex : TEXINDEX,
    in float a_TilingFactor : TILINGFACTOR,

    // OUT
    out float4 o_Pos : SV_POSITION,
    out float2 v_TexCoord : TEXCOORD,
    out float4 v_Color : COLOR,
    out float v_TexIndex : TEXINDEX,
    out float v_TilingFactor : TILINGFACTOR
)
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TexIndex = a_TexIndex;
    v_TilingFactor = a_TilingFactor;
    o_Pos = mul(u_VP, float4(a_Position, 1.0));
}
