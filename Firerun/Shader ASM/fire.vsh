//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float PassedTime;
//   float4x4 WorldViewProjection;
//   float4 endColor;
//   float endScale;
//   float4 midColor;
//   float4 startColor;
//   float startScale;
//
//
// Registers:
//
//   Name                Reg   Size
//   ------------------- ----- ----
//   WorldViewProjection c0       4
//   PassedTime          c4       1
//   startColor          c5       1
//   midColor            c6       1
//   endColor            c7       1
//   startScale          c8       1
//   endScale            c9       1
//

    vs_3_0
    def c10, 1, 0, 8, 2
    def c11, 2, -1, 0, 0
    dcl_position v0
    dcl_texcoord v1
    dcl_texcoord1 v2
    dcl_texcoord2 v3
    dcl_position o0
    dcl_color o1
    dcl_texcoord o2.xy
    mov r0, c6
    add r0, -r0, c5
    rcp r1.x, v3.x
    mad r1.x, c4.x, r1.x, v3.y
    frc r1.x, r1.x
    mad r1.y, r1.x, -c10.w, c10.x
    max r1.y, r1.y, c10.y
    mad r0, r1.y, r0, c6
    add r2, -r0, c7
    mad r1.y, r1.x, c11.x, c11.y
    mad o1, r1.y, r2, r0
    dp3 r0.x, v2, v2
    rsq r0.x, r0.x
    rcp r0.x, r0.x
    add r0.x, r0.x, c10.x
    rcp r0.x, r0.x
    mul r0.y, r1.x, v3.x
    mul r0.x, r0.x, r0.y
    rsq r0.y, r0.y
    rcp r0.y, r0.y
    mul r0.xzw, r0.x, c10.yyzy
    mad r0.xyz, v2, r0.y, r0.xzww
    mov r2.x, c8.x
    add r0.w, -r2.x, c9.x
    mad r0.w, r1.x, r0.w, c8.x
    mad r0.xyz, v0, r0.w, r0
    mul r1, r0.y, c1
    mad r1, c0, r0.x, r1
    mad r0, c2, r0.z, r1
    mad o0, c3, v0.w, r0
    mov o2.xy, v1

// approximately 31 instruction slots used
