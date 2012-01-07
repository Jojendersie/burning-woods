sampler2D sampWhiteNoise : register(s0);
sampler2D sampPerlinNoise : register(s1);

static const float3 ColorLuminanceWeight = float3(0.2125f, 0.7154f, 0.0721f);

// Fragmentprogram
float4 FP(float3 ViewDir	: TEXCOORD1) : COLOR0
{
	ViewDir = normalize(ViewDir);
	float2 tex = ViewDir.xz / (ViewDir.y+1);
	float perlin0 = max(sqrt(tex2D(sampPerlinNoise, tex*0.8).r), 0.2f);
	float4 whitenoise = tex2D(sampWhiteNoise, tex*6);
	float4 whitenoise2 = tex2D(sampWhiteNoise, tex.yx*3);
	whitenoise.rgb *= whitenoise.gbr * whitenoise2.rgb;

	float4 stars = pow(whitenoise, 6) * perlin0 * 20;
	float nebula = pow(perlin0*0.3f, 1.3f)*0.25f;
	
	// desaturate stars
	float starIntensity = dot(ColorLuminanceWeight, stars.rgb);
	stars.rgb = lerp(starIntensity.xxx, stars.rgb, 0.1f);

	return stars + nebula;
}