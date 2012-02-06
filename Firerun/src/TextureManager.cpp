#include "stdafx.h"
#include "Renderer.h"
#include "Texturemanager.h"

#ifdef TEXTURE_TEST
int g_TexIndex=5;
#endif

const DWORD QuadVertex::FVF = D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(1);

// the screen aligned quad
QuadVertex g_aQuad[4] =
    {
        { -0.5f,	-0.5f,	1.0f, 1.0f, 0.0f, 0.0f, D3DXVECTOR3(0,0,0) },
		{ 255.5f,	-0.5f,	1.0f, 1.0f, 1.0f, 0.0f, D3DXVECTOR3(0,0,0) },
		{ -0.5f,	255.5f,	1.0f, 1.0f, 0.0f, 1.0f, D3DXVECTOR3(0,0,0) },
		{ 255.5f,	255.5f,	1.0f, 1.0f, 1.0f, 1.0f, D3DXVECTOR3(0,0,0) }
    };


// **************************************************************************************************************************************************** //
// Sets this texture as rendertarget and aligns the quad
void Texture::SetAsTarget(unsigned int _Index)
{
	// Realign screenquad
	D3DSURFACE_DESC Desc;
	m_pTex->GetLevelDesc(0, &Desc);
	g_aQuad[1].x = Desc.Width - 0.5f;
										g_aQuad[2].y = Desc.Height - 0.5f;
	g_aQuad[3].x = Desc.Width - 0.5f;	g_aQuad[3].y = Desc.Height - 0.5f;

	// set surface
	LPDIRECT3DSURFACE9 pTarget;
	m_pTex->GetSurfaceLevel(0, &pTarget);
	Renderer::Get().m_pD3DDevice->SetRenderTarget(_Index, pTarget);
	pTarget->Release();
}

// **************************************************************************************************************************************************** //
// Draws screen aligned quad with this texture
void Texture::Draw()
{
#ifdef TEXTURE_TEST
	g_aQuad[1].x = 511.5f;
							g_aQuad[2].y = 511.5f;
	g_aQuad[3].x = 511.5f;	g_aQuad[3].y = 511.5f;

	Renderer::Get().m_pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	Renderer::Get().m_pD3DDevice->SetRenderState(D3DRS_ALPHAREF, 125);
	Renderer::Get().m_pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
#endif

	Renderer::Get().m_pD3DDevice->SetTexture(0, m_pTex);
	Renderer::Get().m_pD3DDevice->SetFVF(QuadVertex::FVF);
	Renderer::Get().m_pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof( QuadVertex ) );
}

// **************************************************************************************************************************************************** //
// Predefined Textures
// **************************************************************************************************************************************************** //
// Bark - Birch
const GeneratorGEN BirchGEN = {BARK_TEXTURE_SIZE,BARK_TEXTURE_SIZE,
	1.0f, 1.0f, 3, 7, true,				// Noise1
	0.125f, 2.0f, 6, 7, false,			// Noise2
	-0.17f,// -0.169f, -0.169f,			// Zero-crossing test
	0.0f, 1.0f, 0.0f,					// Less-case Basicnoise
	245.0f, 40.0f,						// Less-case Red
	245.0f, 40.0f,						// Less-case Green
	245.0f, 40.0f,						// Less-case Blue
	2.0f, 0.0f, 0.7f,					// Greater-case Basicnoise
	9.0f, 17.0f,						// Greater-case Red
	8.0f, 16.0f,						// Greater-case Green
	6.3f, 11.0f							// Greater-case Blue
};

// Bark - Maple
const GeneratorGEN MapleGEN = {BARK_TEXTURE_SIZE,BARK_TEXTURE_SIZE,
	4.0f, 1.0f, 3, 7, true,				// Noise1
	2.0f, 1.0f, 4, 6, true,				// Noise2
	-0.2f,// -0.2f, -0.2f,				// Zero-crossing test
	1.0f, -1.0f, 0.4f,					// Less-case Basicnoise
	100.0f, 70.0f,						// Less-case Red
	110.0f, 80.0f,						// Less-case Green
	60.0f, 40.0f,						// Less-case Blue
	-1.0f, 1.0f, 0.6f,					// Greater-case Basicnoise
	55.0f, 27.0f,						// Greater-case Red
	53.0f, 26.0f,						// Greater-case Green
	27.3f, 11.0f						// Greater-case Blue
};

// Bark - Fir
const GeneratorGEN FirGEN = {BARK_TEXTURE_SIZE,BARK_TEXTURE_SIZE,
	4.0f, 1.0f, 3, 7, true,				// Noise1
	2.0f, 1.0f, 4, 6, true,				// Noise2
	-0.2f,// -0.2f, -0.2f,				// Zero-crossing test
	1.0f, -1.0f, 0.4f,					// Less-case Basicnoise
	25.0f, 16.0f,						// Less-case Red
	30.0f, 17.0f,						// Less-case Green
	15.0f, 12.0f,						// Less-case Blue
	-1.0f, 1.0f, 0.6f,					// Greater-case Basicnoise
	55.0f, 27.0f,						// Greater-case Red
	53.0f, 26.0f,						// Greater-case Green
	27.3f, 11.0f						// Greater-case Blue
};

// Terrain - GrayBrownStoneGEN
const GeneratorGEN GrayBrownStoneGEN = {GROUND_TEXTURE_SIZE,GROUND_TEXTURE_SIZE,
	1.0f, 1.0f, 3, 7, false,			// Noise1
	2.0f, 2.0f, 2, 6, false,			// Noise2
	0.0f,// 0.0f, 0.0f,					// Zero-crossing test
	0.0f, 1.0f, -0.4f,					// Less-case Basicnoise
	60.0f, 40.0f,						// Less-case Red
	60.0f, 40.0f,						// Less-case Green
	35.0f, 20.0f,						// Less-case Blue
	1.0f, 0.0f, 0.4f,					// Greater-case Basicnoise
	60.0f, 40.0f,						// Greater-case Red
	60.0f, 40.0f,						// Greater-case Green
	35.0f, 20.0f						// Greater-case Blue
};

// Terrain - Grass
const GeneratorGEN GrassGEN = {GROUND_TEXTURE_SIZE,GROUND_TEXTURE_SIZE,
	16.0f, 2.0f, 4, 7, false,			// Noise1
	2.0f, 16.0f, 3, 6, false,			// Noise2
	0.0f,// 0.0f, 0.0f,					// Zero-crossing test
	0.0f, 1.0f, -0.8f,					// Less-case Basicnoise
	10.0f, 6.0f,						// Less-case Red
	50.0f, 30.0f,						// Less-case Green
	2.0f, 1.0f,							// Less-case Blue
	1.0f, 0.0f, 0.8f,					// Greater-case Basicnoise
	8.0f, 4.0f,							// Greater-case Red
	30.0f, 10.0f,						// Greater-case Green
	3.0f, 2.0f							// Greater-case Blue
};

// Terrain - Forest floor/soil
const GeneratorGEN ForestGEN = {GROUND_TEXTURE_SIZE,GROUND_TEXTURE_SIZE,
	2.0f, 2.0f, 4, 7, false,			// Noise1
	1.0f, 1.0f, 3, 6, true,				// Noise2
	-0.2f,// -0.0f, -0.0f,				// Zero-crossing test
	0.0f, 1.0f, -0.3f,					// Less-case Basicnoise
	20.0f, 55.0f,						// Less-case Red
	15.0f, 50.0f,						// Less-case Green
	0.0f, 0.0f,							// Less-case Blue
	0.4f, 0.1f, 0.2f,					// Greater-case Basicnoise
	35.0f, 25.0f,						// Greater-case Red
	70.0f, 50.0f,						// Greater-case Green
	0.0f, 0.0f							// Greater-case Blue
};

// Single Stones
const GeneratorGEN GrayStoneGEN = {GROUND_TEXTURE_SIZE,GROUND_TEXTURE_SIZE,
	1.0f, 1.0f, 3, 7, false,			// Noise1
	2.0f, 2.0f, 2, 6, false,			// Noise2
	0.0f,// 0.0f, 0.0f,					// Zero-crossing test
	0.0f, 1.0f, -0.4f,					// Less-case Basicnoise
	90.0f, 80.0f,						// Less-case Red
	90.0f, 80.0f,						// Less-case Green
	90.0f, 80.0f,						// Less-case Blue
	1.0f, 0.0f, 0.4f,					// Greater-case Basicnoise
	40.0f, 340.0f,						// Greater-case Red
	40.0f, 340.0f,						// Greater-case Green
	40.0f, 340.0f						// Greater-case Blue
};

// **************************************************************************************************************************************************** //
// Textur manager part
// **************************************************************************************************************************************************** //

// **************************************************************************************************************************************************** //
TextureManager& TextureManager::Get()
{
	static TextureManager TheOneAndOnly;
	return TheOneAndOnly;
}

// **************************************************************************************************************************************************** //
// generates all textures
bool TextureManager::Initialize()
{
	// Generate Noise
	GenerateWhiteNoiseTexture();
	GeneratePerlinNoiseTexture();

	//GenerateBarkTexture_Maple();
	GenerateArbitaryTexture(&m_aBarkTextures[0].m_pTex, &MapleGEN);
	GenerateArbitaryTexture(&m_aBarkTextures[1].m_pTex, &BirchGEN);
	GenerateArbitaryTexture(&m_aBarkTextures[2].m_pTex, &FirGEN);

	GenerateLeafTexture();

	GenerateArbitaryTexture(&m_aTerrainTextures[0].m_pTex, &GrayBrownStoneGEN);
	GenerateArbitaryTexture(&m_aTerrainTextures[1].m_pTex, &GrassGEN);
	GenerateArbitaryTexture(&m_aTerrainTextures[2].m_pTex, &ForestGEN);
	GenerateArbitaryTexture(&m_aTerrainTextures[3].m_pTex, &GrayStoneGEN);

	return true;
}

// **************************************************************************************************************************************************** //
// Destroys all textures
void TextureManager::Shutdown()
{
	if(m_aNoiseTexture[0].m_pTex) m_aNoiseTexture[0].m_pTex->Release();
	if(m_aNoiseTexture[1].m_pTex) m_aNoiseTexture[1].m_pTex->Release();

	if(m_aNoiseTexture[1].m_pTex) m_aBarkTextures[0].m_pTex->Release();
	if(m_aBarkTextures[1].m_pTex) m_aBarkTextures[1].m_pTex->Release();
	if(m_aBarkTextures[2].m_pTex) m_aBarkTextures[2].m_pTex->Release();

	if(m_aTerrainTextures[0].m_pTex) m_aTerrainTextures[0].m_pTex->Release();
	if(m_aTerrainTextures[1].m_pTex) m_aTerrainTextures[1].m_pTex->Release();
	if(m_aTerrainTextures[2].m_pTex) m_aTerrainTextures[2].m_pTex->Release();
	if(m_aTerrainTextures[3].m_pTex) m_aTerrainTextures[3].m_pTex->Release();

	if(m_aLeafTextures[0].m_pTex) m_aLeafTextures[0].m_pTex->Release();
	if(m_aLeafTextures[1].m_pTex) m_aLeafTextures[1].m_pTex->Release();
	if(m_aLeafTextures[2].m_pTex) m_aLeafTextures[2].m_pTex->Release();
}

// **************************************************************************************************************************************************** //
// Creates a rendertarget with given size and format
Texture TextureManager::CreateTexture(const unsigned int _Width, const unsigned int _Height, const D3DFORMAT _Format)
{
	Texture Tex;
	long hr;
	if(FAILED(hr = Renderer::Get().m_pD3DDevice->CreateTexture(_Width, _Height, 1, D3DUSAGE_RENDERTARGET, _Format, D3DPOOL_DEFAULT, &Tex.m_pTex, nullptr)))
		throw "Can't create Rendertarget.";

	return Tex;
}

// **************************************************************************************************************************************************** //
void TextureManager::GenerateWhiteNoiseTexture()
{
	// Create four chanels with white noise
	long hr;
	if(FAILED(hr = Renderer::Get().m_pD3DDevice->CreateTexture(WHITE_NOISE_TEXTURE_SIZE, WHITE_NOISE_TEXTURE_SIZE, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_aNoiseTexture[0].m_pTex, nullptr)))
		return;

	// Locking
	D3DLOCKED_RECT Rect;
	m_aNoiseTexture[0].m_pTex->LockRect(0, &Rect, nullptr, 0);

	// Filling
	for(int i=0;i<WHITE_NOISE_TEXTURE_SIZE*Rect.Pitch;++i)
	{
		unsigned char val = rand() % 255;
		((unsigned char*)Rect.pBits)[i] = val;
		// Put a copy into system memory
		if((i/4)%(Rect.Pitch/4)<WHITE_NOISE_TEXTURE_SIZE)
			m_aNoise[i%4][(i/4)%(Rect.Pitch/4)][i/Rect.Pitch] = val*0.0078125f-1.0f;	// Scale: Byte/128-1 -> [-1,1]
	}

	m_aNoiseTexture[0].m_pTex->UnlockRect(0);
}


// **************************************************************************************************************************************************** //
float PerlinInterpolation(float _x)
{
	// C2-continual implementation
	// For details see "Burger-GradientNoiseGerman-2008".
	return _x*_x*_x*(_x*(_x*6.0f-15.0f)+10.0f);
}

float PerlinInterpolationHard(float _x)
{
	return sqrt(_x);
}

// **************************************************************************************************************************************************** //
// Parameters for noise-function
// Frequence ratio 1:(1.618033989^2)
//const float aFrequence[] = {0.005321559f, 0.013932022f, 0.036474508f, 0.095491503f, 0.25f};
// Frequence ratio 1:2
const float aFrequence[] = {0.00390625f, 0.0078125f, 0.015625f, 0.03125f, 0.0625f, 0.125f, 0.25f, 0.5f};
const int aMaxSamples[] = {(int)(PERLIN_NOISE_TEXTURE_SIZE*aFrequence[0]), (int)(PERLIN_NOISE_TEXTURE_SIZE*aFrequence[1]), (int)(PERLIN_NOISE_TEXTURE_SIZE*aFrequence[2]), (int)(PERLIN_NOISE_TEXTURE_SIZE*aFrequence[3]), (int)(PERLIN_NOISE_TEXTURE_SIZE*aFrequence[4]), (int)(PERLIN_NOISE_TEXTURE_SIZE*aFrequence[5]), (int)(PERLIN_NOISE_TEXTURE_SIZE*aFrequence[6]), (int)(PERLIN_NOISE_TEXTURE_SIZE*aFrequence[7])};

// Calculates the Perlin-noise-generated height at given Point
float TextureManager::PerlinHeight(float _x, float _y, int _LowOctave, int _HighOctave, float (*lerp)(float))
{
	if(!lerp) lerp = PerlinInterpolation;
	// Calculate a sum of frequenzes
	float Res = 0.0f;
	for(int i=_LowOctave;i<_HighOctave;++i)
	{
		int IDx = i%4;
		int IDy = (i+1)%4;
		// Calculate position and divide to index and fractional
		float xfrac = _x*aFrequence[i];
		float yfrac = _y*aFrequence[i];
		int x0 = ((int)xfrac) % aMaxSamples[i];
		int y0 = ((int)yfrac) % aMaxSamples[i];
		int x1 = (x0+1) % aMaxSamples[i];
		int y1 = (y0+1) % aMaxSamples[i];
		xfrac -= (int)xfrac;
		yfrac -= (int)yfrac;
		// Sample Gradients (m_aNoise) direct to tangents
		float w00 = m_aNoise[IDx][x0][y0]*xfrac			+ m_aNoise[IDy][x0][y0]*yfrac;
		float w01 = m_aNoise[IDx][x0][y1]*xfrac			+ m_aNoise[IDy][x0][y1]*(yfrac-1.0f);
		float w10 = m_aNoise[IDx][x1][y0]*(xfrac-1.0f)	+ m_aNoise[IDy][x1][y0]*yfrac;
		float w11 = m_aNoise[IDx][x1][y1]*(xfrac-1.0f)	+ m_aNoise[IDy][x1][y1]*(yfrac-1.0f);

		// Interpolation similar to bilinear. The difference is that the coffecients
		// are not linear.
		xfrac = lerp(xfrac);
		yfrac = lerp(yfrac);
		float w0 = w00+xfrac*(w10-w00);
		float w1 = w01+xfrac*(w11-w01);
		Res += (w0+yfrac*(w1-w0))/exp((float)(i-_LowOctave)); //log(i-_LowOctave+2.0f);
	}
	return Res;
}


// **************************************************************************************************************************************************** //
void TextureManager::GeneratePerlinNoiseTexture()
{
	// Downgrade to ONE
	// Create four chanels with perlin noise
	long hr;
	if(FAILED(hr = Renderer::Get().m_pD3DDevice->CreateTexture(PERLIN_NOISE_TEXTURE_SIZE, PERLIN_NOISE_TEXTURE_SIZE, 1, 0, D3DFMT_R32F, D3DPOOL_MANAGED, &m_aNoiseTexture[1].m_pTex, nullptr)))
		return;

	// Locking
	D3DLOCKED_RECT Rect;
	m_aNoiseTexture[1].m_pTex->LockRect(0, &Rect, nullptr, 0);

	// Filling
	for(int i=0;i<PERLIN_NOISE_TEXTURE_SIZE*Rect.Pitch;i+=4)
	{
		((float*)Rect.pBits)[i/4] = PerlinHeight((i%Rect.Pitch)/4.0f, (float)(i)/Rect.Pitch, 0, PERLIN_NOISE_TEXTURE_OCTAVES, nullptr);
	}

	m_aNoiseTexture[1].m_pTex->UnlockRect(0);
}



// **************************************************************************************************************************************************** //
void TextureManager::GenerateArbitaryTexture(LPDIRECT3DTEXTURE9* _ppTex, const GeneratorGEN* _pGEN)
{
	// Create four chanels
	long hr;
	if(FAILED(hr = Renderer::Get().m_pD3DDevice->CreateTexture(_pGEN->TexSizeX, _pGEN->TexSizeY, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, _ppTex, nullptr)))
		return;

	// Locking
	D3DLOCKED_RECT Rect;
	(*_ppTex)->LockRect(0, &Rect, nullptr, 0);
	unsigned char* pBuffer = ((unsigned char*)Rect.pBits);

	// Filling
	for(int i=0;i<BARK_TEXTURE_SIZE*Rect.Pitch;i+=4)
	{
		float x = (i%Rect.Pitch)/4.0f;
		float y = i/Rect.Pitch;
		float Noise1 = PerlinHeight( x*_pGEN->Noise1_XFreq , y*_pGEN->Noise1_YFreq , _pGEN->Noise1_LowOctave, _pGEN->Noise1_HighOctave, PerlinInterpolation);
		float Noise2 = PerlinHeight( x*_pGEN->Noise2_XFreq , y*_pGEN->Noise2_YFreq , _pGEN->Noise2_LowOctave, _pGEN->Noise2_HighOctave, PerlinInterpolation);//*/
		float Noise3 = m_aNoise[3][i%WHITE_NOISE_TEXTURE_SIZE][(i/WHITE_NOISE_TEXTURE_SIZE)%WHITE_NOISE_TEXTURE_SIZE];
		float Noise1a = _pGEN->Noise1_Abs?fabs(Noise1):Noise1;
		Noise2 = _pGEN->Noise2_Abs?fabs(Noise2):Noise2;
		float G_Noise = Noise1a*_pGEN->G_Noise1 + Noise2*_pGEN->G_Noise2 + Noise3*_pGEN->G_Noise3;
		float L_Noise = Noise1a*_pGEN->L_Noise1 + Noise2*_pGEN->L_Noise2 + Noise3*_pGEN->L_Noise3;
		// Red
		pBuffer[i+2] = (unsigned char)((Noise1>_pGEN->ZeroCrossingOffsetR)?_pGEN->L_OffsetR+_pGEN->L_FactorR*L_Noise:_pGEN->G_OffsetR+_pGEN->G_FactorR*G_Noise);
		// Green
		pBuffer[i+1] = (unsigned char)((Noise1>_pGEN->ZeroCrossingOffsetR)?_pGEN->L_OffsetG+_pGEN->L_FactorG*L_Noise:_pGEN->G_OffsetG+_pGEN->G_FactorG*G_Noise);
		// Blue
		pBuffer[ i ] = (unsigned char)((Noise1>_pGEN->ZeroCrossingOffsetR)?_pGEN->L_OffsetB+_pGEN->L_FactorB*L_Noise:_pGEN->G_OffsetB+_pGEN->G_FactorB*G_Noise);//*/
		// Alpha
		pBuffer[i+3] = 255;
		// The following implementation seems to need more code (compiled)
	/*	if(Noise1>_pGEN->ZeroCrossingOffsetR)
		{
			float L_Noise = Noise1a*_pGEN->L_Noise1 + Noise2*_pGEN->L_Noise2 + Noise3*_pGEN->L_Noise3;
			// Red
			pBuffer[i+2] = (unsigned char)(_pGEN->L_OffsetR+_pGEN->L_FactorR*L_Noise);
			// Green
			pBuffer[i+1] = (unsigned char)(_pGEN->L_OffsetG+_pGEN->L_FactorG*L_Noise);
			// Blue
			pBuffer[ i ] = (unsigned char)(_pGEN->L_OffsetB+_pGEN->L_FactorB*L_Noise);		
		} else {
			float G_Noise = Noise1a*_pGEN->G_Noise1 + Noise2*_pGEN->G_Noise2 + Noise3*_pGEN->G_Noise3;
			// Red
			pBuffer[i+2] = (unsigned char)(_pGEN->G_OffsetR+_pGEN->G_FactorR*G_Noise);
			// Green
			pBuffer[i+1] = (unsigned char)(_pGEN->G_OffsetG+_pGEN->G_FactorG*G_Noise);
			// Blue
			pBuffer[ i ] = (unsigned char)(_pGEN->G_OffsetB+_pGEN->G_FactorB*G_Noise);		
		}//*/
	}

	// Unlocking
	(*_ppTex)->UnlockRect(0);
}

// **************************************************************************************************************************************************** //
void TextureManager::GenerateLeafTexture()
{
	// Create four chanels
	long hr;
	if(FAILED(hr = Renderer::Get().m_pD3DDevice->CreateTexture(LEAF_TEXTURE_SIZE, LEAF_TEXTURE_SIZE, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_aLeafTextures[0].m_pTex, nullptr)))
		return;

	// Locking
	D3DLOCKED_RECT Rect;
	m_aLeafTextures[0].m_pTex->LockRect(0, &Rect, nullptr, 0);
	unsigned char* pBuffer = ((unsigned char*)Rect.pBits);

	// Filling
	for(int i=0;i<BARK_TEXTURE_SIZE*Rect.Pitch;i+=4)
	{
		int x = (i%Rect.Pitch)/4;
		int y = (i/Rect.Pitch);
		float Noise1 = PerlinHeight( x*2.0f , y*2.0f , 4, 6, PerlinInterpolation)*2;
		float Noise2 = PerlinHeight( ((i+300)%Rect.Pitch)/4.0f , (i/(float)Rect.Pitch) , 0, 3, PerlinInterpolation);
		x -= LEAF_TEXTURE_SIZE>>1;
		y -= LEAF_TEXTURE_SIZE>>1;
		Noise2 += Noise1;
		Noise2 /= std::max(2.0f,(x*x+y*y)*0.0005f);
		Noise1 *= 0.5f;
		// Red
		pBuffer[i+2] = (unsigned char)(/*(Noise2>-0.2f)?0.0f:*/60.0f+50.0f*Noise1);
		// Green
		pBuffer[i+1] = (unsigned char)(/*(Noise2>-0.2f)?0.0f:*/100.0f+100.0f*fabs(Noise1));
		// Blue
		pBuffer[ i ] = (unsigned char)(/*(Noise2>-0.2f)?0.0f:*/20.0f+10.0f*Noise1);
		// Alpha
		pBuffer[i+3] = (Noise2>-0.2f)?0:255;
	}
	// Optimize for alpha-test (Could be removed for size restrictions)
	/*for(int i=Rect.Pitch;i<(BARK_TEXTURE_SIZE-1)*Rect.Pitch;i+=4)
	{
		// Count alpha Pixels
		int Count = (pBuffer[i-1]+pBuffer[i+7]+pBuffer[i+3+Rect.Pitch]+pBuffer[i+3-Rect.Pitch]+pBuffer[i+3])/255;
		// Reset alpha
		pBuffer[i+3] = Count>2?255:0;
	}*/

	// Unlocking
	m_aLeafTextures[0].m_pTex->UnlockRect(0);
}

// **************************************************************************************************************************************************** //
