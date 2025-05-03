@echo off

dxc -T vs_6_9 -E main -Fo texture.vert.cso texture.vert.hlsl
dxc -T ps_6_9 -E main -Fo texture.frag.cso texture.frag.hlsl
dxc -DCOMPILE_ROOTSIG -T rootsig_1_1 -Fo rootsig.cso rootsig.hlsli
:: fxc /T rootsig_1_1 .\rootsig.hlsli /E ROOTSIG /Fo rootsig.fxo

pause
