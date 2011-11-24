struct GBufferOutput
{
	float4 AlbedoX	: COLOR0;
	float4 Normal	: COLOR1;
	float4 Depth	: COLOR2;
};

// Vertexshader for the Terrain
sampler2D sampTerrainHeights : register(s0);
float4x4 ViewProjection;
float3x3 View;
float3 TerrainTranslation;
float3 ViewPosition;

static const float HeightScale = 100.0f;
static const float TerrainTextureWorldSize = 1024.0f;
static const float TerrainTextureOffsetPerUnit = 1.0f/TerrainTextureWorldSize;
static const float TextureIteration = 0.05f;

struct VERTEXOUTPUT
{
	float4 Position 		: POSITION;
	float NormHeight 		: TEXCOORD0;	// 0-1
	float Depth				: TEXCOORD1;
	float3 NormalViewSpace	: TEXCOORD2;
	float2 Texcoord 		: TEXCOORD3;
	float2 TexcoordNoise 	: TEXCOORD4;
};

VERTEXOUTPUT TerrainVS(float4 InPosition : POSITION)
{
	VERTEXOUTPUT Out;
	
	// Translate
	float4 TransPosition;
	TransPosition.xyz = InPosition.xyz + TerrainTranslation;
	TransPosition.w = 1.0f;

	// Texcoord
	Out.Texcoord = TransPosition.xz * TextureIteration;
	Out.TexcoordNoise = TransPosition.zx * TerrainTextureOffsetPerUnit * 10;

	// height
	float4 TexCoord = float4(TransPosition.xz * TerrainTextureOffsetPerUnit, 0, 0);
	Out.NormHeight = tex2Dlod(sampTerrainHeights, TexCoord).r;
	TransPosition.y += Out.NormHeight * HeightScale;

	// Depth
	Out.Depth = length(TransPosition.xyz - ViewPosition);

	// Transform
	Out.Position = mul(ViewProjection, TransPosition);

	// Compute Normal

	// approximation formula from ovgu cg
	// y(x – 1,z) – y(x + 1,z)
	// 2
	// y(x,z – 1) – y(x,z + 1)
	float4 TexCoord0 = TexCoord; TexCoord0.x -= TerrainTextureOffsetPerUnit;
	float HeightX0 = tex2Dlod(sampTerrainHeights, TexCoord0).r;
	TexCoord0.x += TerrainTextureOffsetPerUnit*2;
	float HeightX1 = tex2Dlod(sampTerrainHeights, TexCoord0).r;

	TexCoord0 = TexCoord; TexCoord0.y -= TerrainTextureOffsetPerUnit;
	float HeightZ0 = tex2Dlod(sampTerrainHeights, TexCoord0).r;
	TexCoord0.y += TerrainTextureOffsetPerUnit*2;
	float HeightZ1 = tex2Dlod(sampTerrainHeights, TexCoord0).r;

	Out.NormalViewSpace = float3(HeightX0 - HeightX1, 1, HeightZ0 - HeightZ1);
	Out.NormalViewSpace.xz *= HeightScale;
	Out.NormalViewSpace = normalize(mul(View, Out.NormalViewSpace));

	return Out;
};


// Pixelshader for the Terrain
sampler2D sampGras : register(s0);
sampler2D sampRock : register(s1);
sampler2D sampWoods : register(s2);
sampler2D sampNoise : register(s3);
static const float RockGrasSmoothnes = 6.0f;
static const float GrasFactor = 1.25f;
static const float WoodNoiseFactor = 4.f;

float3 YUpViewSpace;	// 0 1 0 in Viewspace

GBufferOutput TerrainPS(VERTEXOUTPUT In)
{
	GBufferOutput Output = (GBufferOutput)0;

	// Normal
	float3 Normal = normalize(In.NormalViewSpace);
	Output.Normal.xy = Normal.xy;
	Output.AlbedoX.a = sign(Normal.z);

	// Texturing
	float Noise = tex2D(sampNoise, In.TexcoordNoise).r*WoodNoiseFactor;
	float3 Gras = tex2D(sampGras, In.Texcoord).rgb;
	float3 Rock = tex2D(sampRock, In.Texcoord).rgb;
	float3 Woods = tex2D(sampWoods, In.Texcoord).rgb;
	float GrassAmount = saturate(pow(dot(Normal, YUpViewSpace)*1.3f, RockGrasSmoothnes));

	Output.AlbedoX.rgb = lerp(Gras, Woods, saturate(Noise));
	Output.AlbedoX.rgb = lerp(Rock, Output.AlbedoX.rgb, GrassAmount);

	// Depth
	Output.Depth = In.Depth;

	return Output;
};