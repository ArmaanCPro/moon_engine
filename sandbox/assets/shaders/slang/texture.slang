// -------------- globals -----------------
#define SET_GLOBAL 0
#define BIND_TEXTURES   0
#define BIND_SAMPLER    1
#define BIND_STORAGE_IMAGES 2
#define BIND_YUV_IMAGES 3
#define BIND_ACCEL  4
#define BIND_CAMERA 5 // unused backend binding for UBO

// ------ resources -------
[[vk::binding(BIND_CAMERA, SET_GLOBAL)]]
cbuffer Camera
{
    float4x4 u_VP;
}

struct VSInput
{
    [[vk::location(0)]] float3 position     : POSITION;
    [[vk::location(1)]] float4 color        : COLOR;
    [[vk::location(2)]] float2 tex_coord    : TEXCOORD0;

    [[vk::location(3)]] float tex_index     : TEXCOORD1;
    [[vk::location(4)]] float tiling_factor : TEXCOORD2;
}
struct VSOutput
{
    float4 position : POSITION;
    float4 color    : COLOR;
    float2 tex_coord    : TEXCOORD0;
    nointerpolation uint tex_slot   : TEXCOORD1;
    float tiling_factor : TEXCOORD2;
}

[shader("vertex")]
VSOutput vertexMain(VSInput in)
{
    VSOutput out;
    out.position = mul(u_VP, float4(in.position, 1.0));
    out.color = in.color;
    out.tex_coord = in.tex_coord;
    out.tex_slot = (uint)round(in.tex_index);
    out.tiling_factor = in.tiling_factor;
    return out;
}


// -- FRAGMENT --

// ------- resources ---------
[[vk::binding(BIND_SAMPLER, SET_GLOBAL)]]
SamplerState g_Samplers[];

[[vk::binding(BIND_TEXTURES, SET_GLOBAL)]]
Texture2D<float4> g_Textures[];

// 32-slot indirection table as push constants
struct DrawRemap
{
    uint slot_to_desc[32]; // slot -> descriptor index in g_Textures[]
    // this was omitted because these have the same mapping
    // uint slot_to_sampler[32]; // slot -> descriptor index in g_Samplers[]
}
[[vk::push_constant]]
DrawRemap pc;

[shader("fragment")]
float4 fragmentMain(VSOutput in) : SV_Target
{
    uint slot = min(in.tex_slot, 31u);

    uint desc_index = pc.slot_to_desc[slot];
    uint non_uni = NonUniformResourceIndex(desc_index);

    float2 uv = in.tex_coord * in.tiling_factor;
    float4 texel = g_Textures[non_uni].Sample(g_Samplers[non_uni], uv);

    return texel * in.color;
}
