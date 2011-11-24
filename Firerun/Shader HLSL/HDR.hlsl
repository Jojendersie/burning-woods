//----------------------------------------------------------------------------------------------------------------------------------------------
// Shader die das HDR Rendering, bzw. Tonemapping betreffen
//----------------------------------------------------------------------------------------------------------------------------------------------

// #define LAST_LUM_DOWNSAMPLE

// Gewichtung der Farbkanäle für Luminance
static const float3 ColorLuminanceWeight = float3(0.2125f, 0.7154f, 0.0721f);
// Konstanten für Tonemapping
static const float Exposure = 0.08f;

// SampleOffsets fürs Downsampling
float4 SampleOffsets[4];	// je 2 float2

// Helligkeitslimiterung für BrightPass
static const float BrightLimiter = 0.02f;
static const float BloomScale = 1.15f;

// Gaussian Blur
	// Kernel Size 24, sigma 
float4 GaussianBlurOffsets[12];
static const float GaussianBlurWeights[13] = 
{ 0.00401739f,
 0.00651127f,
 0.01011929f,
 0.01507989f,
 0.02154813f,
 0.02952462f,
 0.03879024f,
 0.04886793f,
 0.05903217f,
 0.06837810f,
 0.07594664f,
 0.08088417f, 0.08260034f}; // mittelsample zum schluss, ansonsten von fern nach nah


// Intensität des Blooms
//static const float BloomScale = 1.25f;

// Geschwindigkeit mit der die Luminance angepasst wird
static const float LuminanceAdaptionSpeed = 10.0f;

float FrameTime;


//----------------------------------------------------------------------------------------------------------------------------------------------
// Hilfsfunktion für Tonemapping
float3 toneMap(float3 Color, float Luminance)
{
	// exposure
	float keyValue = Exposure;
	float linearExposure = keyValue / Luminance;
//	float exposure = max(linearExposure, 0.0001f);
    Color *= linearExposure;

	// filmic tonemapping
    Color = max(0, Color - 0.004f);
    Color = (Color * (6.2f * Color + 0.5f)) / (Color * (6.2f * Color + 1.7f)+ 0.06f);
    // result has 1/2.2 baked in
    return pow(Color, 2.2f);

	// reinhard's basic tonemapping
	/*float pixelLuminance = dot(Color, ColorLuminanceWeight);    
    float toneMappedLuminance = pixelLuminance / (pixelLuminance + 1);
	return toneMappedLuminance * pow(Color / pixelLuminance, 1); */

	/*// drago
	float pixelLuminance = dot(Color, ColorLuminanceWeight);
    float toneMappedLuminance = log10(1 + pixelLuminance);
	toneMappedLuminance /= log10(1 + 5);
	toneMappedLuminance /= log10(2 + 8 * ((pixelLuminance / 5) * log10(0.5f) / log10(0.5f)));
	return toneMappedLuminance * pow(Color / pixelLuminance, 1); */
}

//----------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------------------------------
// Luminance Extraktion und Downsampling mit 2x2 Filter vom ScreenBuffer aus
float4 LuminanceInitial(float4 Tex : TEXCOORD0, sampler2D sampScreenBuffer : register(s0)) : COLOR0
{
	float3 Sample = float3(0,0,0);
    [unroll] for(int i = 0; i < 2; i++)
    {
		float4 TexOffseted = Tex + SampleOffsets[i];
        Sample += log(dot(tex2D(sampScreenBuffer, TexOffseted.xy).rgb, ColorLuminanceWeight) + 1e-4);
		Sample += log(dot(tex2D(sampScreenBuffer, TexOffseted.zw).rgb, ColorLuminanceWeight) + 1e-4);
	}
	float Sum = Sample * 0.25f;
	return Sum;
}

//----------------------------------------------------------------------------------------------------------------------------------------------
// Luminance Downsampling mit 3x3 Filter, wahlweise unter Berücksichtigung alter Lumance
float4 LuminanceDownsample(float2 Tex : TEXCOORD0, sampler2D sampLuminance : register(s0)
	#ifdef LAST_LUM_DOWNSAMPLE
		, sampler2D sampLuminanceOld : register(s1)
	#endif
		)	: COLOR0
{
    float Luminance = tex2D(sampLuminance, Tex.xy).r;
	[unroll] for(int i = 0; i < 4; i++)
	{
		float4 TexOffseted = Tex.xyxy + SampleOffsets[i];
        Luminance += tex2D(sampLuminance, TexOffseted.xy).r;
		Luminance += tex2D(sampLuminance, TexOffseted.zw).r;
	}
	Luminance *= 0.11111111111f;

#ifdef LAST_LUM_DOWNSAMPLE
	Luminance = exp(Luminance);
	float OldLuminance = tex2D(sampLuminanceOld, float2(0.5f, 0.5f)).r;
    Luminance = OldLuminance + (Luminance - OldLuminance) * (1 - pow(0.98f, LuminanceAdaptionSpeed * FrameTime));
#endif
    return Luminance;
}

//----------------------------------------------------------------------------------------------------------------------------------------------
// Helligkeitsfilterung mit 3x3 Sampling
float4 BrightPass(float2 Tex : TEXCOORD0, sampler2D sampScreenBuffer : register(s0), sampler2D sampLum : register(s1)) : COLOR0
{
	float3 Color =  tex2D(sampScreenBuffer, Tex.xy).rgb;
    [unroll] for(int i = 0; i < 4; i++)
	{
		float4 TexOffseted = Tex.xyxy + SampleOffsets[i];
        Color += tex2D(sampScreenBuffer, TexOffseted.xy).rgb;
		Color += tex2D(sampScreenBuffer, TexOffseted.zw).rgb;
	}

	// Durchschnitt
	Color *= 0.1111111111111111f;

	// treshhold and tonemap
	float Luminance = tex2D(sampLum, float2(0.5f, 0.5f)).r;
	Color.rgb = max(float3(0.0f, 0.0f, 0.0f), Color.rgb - BrightLimiter);
	return float4(toneMap(Color, Luminance), 1.0f);
}

//----------------------------------------------------------------------------------------------------------------------------------------------
// Gaussian Blur 15 samples
float4 GaussianBlur(float2 Tex : TEXCOORD0, sampler2D sampTex : register(s0)) : COLOR0
{
    float4 Color = tex2D(sampTex, Tex.xy) * GaussianBlurWeights[12];
	[unroll] for(int i = 0; i < 12; i++)
	{
		float4 TexOffseted = Tex.xyxy + GaussianBlurOffsets[i];
        Color += (tex2D(sampTex, TexOffseted.xy) + tex2D(sampTex, TexOffseted.zw)) * GaussianBlurWeights[i];
	}

    return Color;
}

//----------------------------------------------------------------------------------------------------------------------------------------------
// Tonemapping Ende
float4 ToneMappingFinish(float2 Tex : TEXCOORD0, sampler2D sampScreenBuffer : register(s0), sampler2D sampLum : register(s1), sampler2D sampBloom : register(s2)) : COLOR0
{
    float3 sceneCol = tex2D(sampScreenBuffer, Tex).rgb;
	float Luminance = tex2D(sampLum, float2(0.5f, 0.5f)).r;

	// tonemapping
	float3 toneMappedSceneCol = toneMap(sceneCol, Luminance);
	
	// Bloom
	float3 Bloom = tex2D(sampBloom, Tex).rgb*BloomScale;
	
	// sceneblend
	return float4(toneMappedSceneCol * (BloomScale-Bloom) + Bloom, 0.0f);
}
