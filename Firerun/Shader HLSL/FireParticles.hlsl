float4x4 WorldViewProjection;

float PassedTime;
float3 Force = float3(0.0f, 1.0f, 0.0f);

struct VERTEXOUTPUT
{
	float4 Position : POSITION;
};

VERTEXOUTPUT FireVS(float4 Position : POSITION, float2 Texcoord : TEXCOORD0,
					float3 InitialDirection : TEXCOORD1, float2 LifeTimeStartTime : TEXCOORD2)
{
	VERTEXOUTPUT Out;

	// compute time value from 0-LifeTimeStartTime.x
	float currentTime = frac(LifeTimeStartTime.y + PassedTime / LifeTimeStartTime.x) * LifeTimeStartTime.x;

	Position.xyz += InitialDirection * currentTime + Force * currentTime;


	Out.Position = mul(WorldViewProjection, Position);
	return Out;
};

float4 FirePS(VERTEXOUTPUT In) : Color0
{
	return float4(1,0,1,1);
};