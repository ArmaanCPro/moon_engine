#ifndef ROOTSIG_HLSLI
#define ROOTSIG_HLSLI

/*
 *   Root Signature Layout (CPU)
 *   -> 0        descriptor table "ViewProjection" and "Textures"
 *
 *   Root Signature Layout (GPU)
 *   -> b0       float4x4 "ViewProj"
 *   -> t0...    Texture2D<float4> "Textures"
 *   -> s0       Sampler for "Textures"
 */

#define ROOTSIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"DescriptorTable(" \
    "CBV(b0)," \
    "SRV(t0, numDescriptors = 32)" \
")," \
"StaticSampler(s0)"

#ifdef COMPILE_ROOTSIG
RootSignature( ROOTSIG );
#endif

#endif
