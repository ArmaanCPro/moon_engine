#include "rootsig.hlsli"

Texture2D<float4> textures[32] : register(t0);
sampler textureSampler : register(s0);

[RootSignature(ROOTSIG)]
void main(
    // in
    in float4 v_Color : COLOR,
    in float2 v_TexCoord : TEXCOORD,
    in float v_TexIndex : TEXINDEX,
    in float v_TilingFactor : TILING_FACTOR,

    // out
    out float4 o_Pixel : SV_TARGET
)
{
    float4 texel = textures[v_TexIndex].Sample(textureSampler, v_TexCoord * v_TilingFactor);
    o_Pixel = float4(texel.rgb * v_Color.rgb, 1.0f);
}
