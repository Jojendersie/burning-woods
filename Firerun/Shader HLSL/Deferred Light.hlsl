// Konstanten
int SecondDirectionLight = 0;
float3 LightColor;
float3 DirLightDir_ViewSpace;
float3 AmbientLight;

const float3 FogColor = float3(0,0,0.01f); 
const float FogStart = 0.9f;
const float FogFactor = 1.f / (1.f - FogStart);

// GBuffer
sampler2D sampGBuffer0 : register(s0);
sampler2D sampGBuffer1 : register(s1);
sampler2D sampGBuffer2 : register(s2);


//----------------------------------------------------------------------------------------------------------------------------------------------
// helper functions
//----------------------------------------------------------------------------------------------------------------------------------------------
// unpacks the normal
float3 UnpackNormal(float4 AlbedoX, float4 NormalXY)
{
	// z = sqrt(1 - x² - y²)
	return normalize(float3(NormalXY.x, NormalXY.y, sqrt(1.0f - dot(NormalXY.xy, NormalXY.xy)) * (AlbedoX.a * 2 - 1)));
}


//----------------------------------------------------------------------------------------------------------------------------------------------
// Screen-Quad with ray output
//----------------------------------------------------------------------------------------------------------------------------------------------
struct VertexOutput
{
	float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD0;
	float3 ViewDir	: TEXCOORD1;
};
VertexOutput ScreenQuadWithRays_VS(float4 Pos : POSITION, float2 Tex : TEXCOORD0, float3 ViewDir : TEXCOORD1)			 
{
	VertexOutput Out;
	Out.Pos = Pos;
	Out.Tex = Tex;
	Out.ViewDir = ViewDir;
	return Out;
}

//----------------------------------------------------------------------------------------------------------------------------------------------
// Directional Light + Ambient
//----------------------------------------------------------------------------------------------------------------------------------------------
float4 DeferredLight_DirLightAmbient_FP(VertexOutput In) : COLOR0
{
	float4 AlbedoX			= tex2D(sampGBuffer0, In.Tex);
	float4 NormalXY			= tex2D(sampGBuffer1, In.Tex);
	float4 Depth			= tex2D(sampGBuffer2, In.Tex);

	// reconstruct normal
	float3 Normal = UnpackNormal(AlbedoX, NormalXY);
	
	// viewdir in view space
	float3 ViewDir = normalize(In.ViewDir);

	// lighting
	float NDotL = dot(DirLightDir_ViewSpace, Normal);
	float FirstLightDiff = max(0.0f, NDotL);
	float3 Lighting = AmbientLight + LightColor * FirstLightDiff;

	float3 Color = AlbedoX.rgb*Lighting;

	// fogging
	Color = lerp(Color, FogColor, min(1, max(0, Depth.r - FogStart) * FogFactor));

	// Output
	return float4(Color, 1);
}
//----------------------------------------------------------------------------------------------------------------------------------------------
// Pointlight
//----------------------------------------------------------------------------------------------------------------------------------------------
float3 LightPosition_ViewSpace;
float RangeSq;
float4 DeferredLight_PointLight_FP(VertexOutput In) : COLOR0
{
	/*// Sichtrichtung und Texturkoordinaten ermitteln
	In.ProjPos.xy /= In.ProjPos.w;
	float3 ViewDir = normalize(float3(In.ProjPos.xy, 1) * FarCorner);
    float2 Tex = float2(In.ProjPos.x, -In.ProjPos.y) * 0.5f + TextureLookUpBias_AddHalf;
	*/
	// Texture coordinate magic, this compensates for jitter
	//float2 texCoord = float2(projPos[0]/2+0.5, -projPos[1]/2+0.5);
	//oTexCoord = fixUV(uv, float2(vpWidth, vpHeight));

	// GBuffer sampling
	float4 AlbedoX			= tex2D(sampGBuffer0, In.Tex);
	//float4 SpecularValues	= tex2D(sampGBuffer1, In.Tex);
	float4 NormalXY			= tex2D(sampGBuffer1, In.Tex);
	float4 Depth			= tex2D(sampGBuffer2, In.Tex);

		
	// viewdir in view space
	float3 ViewDir = normalize(In.ViewDir);

	// direction to light
	float3 Position = ViewDir * Depth.r;
	float3 ToLightVec = LightPosition_ViewSpace - Position;
	float3 LightDir = normalize(ToLightVec);

	// Attenuation "DistanceFactor"
	float DistanceSq = dot(ToLightVec,ToLightVec);
	float DistanceFactor = saturate(1.0f - DistanceSq / RangeSq);

	// Normale rekonstruieren
	float3 Normal = UnpackNormal(AlbedoX, NormalXY);

	// Lighting
	float NDotL = dot(LightDir, Normal);
	float Lighting = max(0, NDotL);
//	float Specular = ComputeSpecular(ViewDir, Normal, LightDir, NDotL, SpecularValues.r) * SpecularValues.g;

	// Output (additiv)
	return float4((AlbedoX.rgb * Lighting/* + Specular*/) * LightColor * DistanceFactor, 1.0f);
}