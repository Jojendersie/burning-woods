//----------------------------------------------------------------------------------------------------------------------------------------------
// Vertex- & Fragmentprogram for the stars (vertexpoints)
//----------------------------------------------------------------------------------------------------------------------------------------------

float4x4 WorldViewProjection;

sampler2D sampWhiteNoise : register(s0);
sampler2D sampPerlinNoise : register(s1);

static const float3 ColorLuminanceWeight = float3(0.2125f, 0.7154f, 0.0721f);

// Vertexprogram
struct VertexOutput
{
	float4 Pos 		: POSITION;
	float4 Color	: TEXCOORD0;
};
VertexOutput VP(float4 inPos : POSITION, float3 Color : TEXCOORD0)
{
	VertexOutput Out;
	Out.Pos = mul(WorldViewProjection, inPos);
	Out.Pos.xy /= Out.Pos.w;
	Out.Pos.w = Out.Pos.z = 1.0f;
	Out.Color = float4(Color, 1);
	return Out;
}

// Fragmentprogram
float4 FP(float4 ViewDir	: TEXCOORD1) : COLOR0
{
	ViewDir = normalize(ViewDir);
	float2 tex = ViewDir.xz / (ViewDir.y+1);
	float perlin0 = sqrt(tex2D(sampPerlinNoise, tex).r);
	//float perlin1 = sqrt(tex2D(sampPerlinNoise,  tex * 0.25f).r);
	float4 whitenoise = tex2D(sampWhiteNoise, tex*10);
	float4 whitenoise2 = tex2D(sampWhiteNoise, tex.yx*4);
	whitenoise.rgb *= whitenoise.gbr * whitenoise2.rgb;

	float4 stars = pow(whitenoise, 8) * perlin0 * 20;
	float nebula = pow(perlin0*0.3f, 1.5f)*0.5f;
	
	// desaturate stars
	float starIntensity = dot(ColorLuminanceWeight, stars.rgb);
	stars.rgb = lerp(starIntensity.xxx, stars.rgb, 0.1f);

	return stars + nebula;// * perlin1 + float4(perlin0, perlin0, perlin0+perlin1, 1);
}