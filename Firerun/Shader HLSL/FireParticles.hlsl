float4x4 WorldViewProjection;

float PassedTime;
static const float3 Force = float3(0.0f, 15.0f, 0.0f);

float4 startColor;
float4 midColor;
float4 endColor;

sampler2D noiseTexture : register(s0);

struct VERTEXOUTPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 Texcoord : TEXCOORD0;
};

VERTEXOUTPUT FireVS(float4 Position : POSITION, float2 Texcoord : TEXCOORD0,
					float3 InitialDirection : TEXCOORD1, float2 LifeTimeStartTime : TEXCOORD2)
{
	VERTEXOUTPUT Out;

	// compute time value from 0-LifeTimeStartTime.x
	float currentTimePercentage = frac(LifeTimeStartTime.y + PassedTime / LifeTimeStartTime.x);
	float currentTime = currentTimePercentage * LifeTimeStartTime.x;

	Position.xyz += InitialDirection * currentTime + Force * currentTime;

	// color
	Out.Color = lerp(midColor, startColor, saturate(1.0f - currentTimePercentage*2));
	Out.Color = lerp(Out.Color, endColor, currentTimePercentage*2-1);
	
	Out.Position = mul(WorldViewProjection, Position);
	Out.Texcoord = Texcoord;
	return Out;
};

float4 FirePS(VERTEXOUTPUT In) : Color0
{
	// noise + radial falloff
	float2 pos = In.Texcoord * 2 - 1.0f;
	float falloff = saturate(1.0f - dot(pos,pos));
	return In.Color * tex2D(noiseTexture, In.Texcoord*10).r * falloff;
};