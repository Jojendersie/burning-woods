struct GBufferOutput
{
	float4 AlbedoX	: COLOR0;
	float4 Normal	: COLOR1;
	float4 Depth	: COLOR2;
};
// Vertexshader
float4x4 WorldViewProjection;
float4x4 WorldView;
float3 ObjSpaceViewPos;

struct VERTEXOUTPUT
{
	float4 Position : POSITION;
	float2 Texcoord	: TEXCOORD0;
	float3 Normal	: TEXCOORD2;
	float Depth : TEXCOORD3;
};

VERTEXOUTPUT StoneVS(float4 Position : POSITION, float3 Normal : NORMAL, float2 Texcoord : TEXCOORD0)
{
	VERTEXOUTPUT Out;
	
	Out.Position = mul(WorldViewProjection, Position);
	Out.Normal = normalize(mul(WorldView, Normal));
	Out.Texcoord = Texcoord;
	Out.Depth = length(Position.xyz - ObjSpaceViewPos);

	return Out;
};

// Pixelshader
sampler2D sampTexture : register(s0);

GBufferOutput StonePS(VERTEXOUTPUT In)
{
	GBufferOutput Output = (GBufferOutput)0;
	In.Normal = normalize(In.Normal);
	Output.AlbedoX.rgb = tex2D(sampTexture, In.Texcoord);
	Output.AlbedoX.a = sign(In.Normal.z);
	Output.Normal.xy = In.Normal.xy;
	Output.Depth = In.Depth;

	return Output;
};