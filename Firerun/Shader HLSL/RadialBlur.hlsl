sampler2D sampSceneBuf : register(s0);
float2 Step;
float WeightExp;

#define NumSamples 16
static const float maxDistInv = 1.4142135623730950488016887242097f; 	// 1.0 / sqrt(0,5)

float4 FP(float2 Tex	: TEXCOORD0) : COLOR0
{
	float3 Color = tex2Dlod(sampSceneBuf, float4(Tex, 0, 0)).rgb;
	float2 direction = float2(0.5f,0.5f) - Tex;
	float dist = length(direction);
	direction /= dist;
	direction *= Step;
	float weight = pow(maxDistInv * dist, WeightExp);
	for(int i=0; i<NumSamples; ++i)
	{
		Tex += direction;
		Color += tex2Dlod(sampSceneBuf, float4(Tex, 0, 0)).rgb * weight;
	}
	Color /= weight * NumSamples + 1;

	return float4(Color, 1.0f);
}