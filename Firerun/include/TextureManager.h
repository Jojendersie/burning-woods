#pragma once


// No values below 256
#define WHITE_NOISE_TEXTURE_SIZE	256
#define PERLIN_NOISE_TEXTURE_SIZE	1024
#define BARK_TEXTURE_SIZE			256
#define GROUND_TEXTURE_SIZE			256
#define LEAF_TEXTURE_SIZE			256

// Use only numbers between 1 and 7
#define PERLIN_NOISE_TEXTURE_OCTAVES 7


// RHW transformed vertexstruct.
struct QuadVertex
{
	float	x,y,z,w;
	float	u,v;
	D3DXVECTOR3 ViewDir;
	const static DWORD FVF;
};

// the screen aligned quad
extern QuadVertex g_aQuad[4];


// The Texture itself. Using this struct allows direkt drawing to screen.
class Texture
{
public:
	LPDIRECT3DTEXTURE9 m_pTex;

	// Constructor
	Texture() : m_pTex(nullptr)	{}

	// Sets this texture as rendertarget and aligns the quad
	void SetAsTarget(unsigned int _Index = 0);

	// Draws screen aligned quad with this texture
	void Draw();
};

// **************************************************************************************************************************************************** //
#pragma pack (push, 1)
struct GeneratorGEN
{
	unsigned short	TexSizeX;
	unsigned short	TexSizeY;

	float Noise1_XFreq;
	float Noise1_YFreq;
	unsigned char Noise1_LowOctave;
	unsigned char Noise1_HighOctave;
	bool Noise1_Abs;

	float Noise2_XFreq;
	float Noise2_YFreq;
	unsigned char Noise2_LowOctave;
	unsigned char Noise2_HighOctave;
	bool Noise2_Abs;

	float ZeroCrossingOffsetR;
//	float ZeroCrossingOffsetG;
//	float ZeroCrossingOffsetB;

	// Formulas Noise1 <= ZeroCrossingOffsetR
	// L_Offset + L_Factor*Noise2
	float L_Noise1;
	float L_Noise2;
	float L_Noise3;
	float L_OffsetR;
	float L_FactorR;
	float L_OffsetG;
	float L_FactorG;
	float L_OffsetB;
	float L_FactorB;

	// Formulas Noise1 > ZeroCrossingOffsetR
	// G_Offset + G_Factor*Noise2
	float G_Noise1;
	float G_Noise2;
	float G_Noise3;
	float G_OffsetR;
	float G_FactorR;
	float G_OffsetG;
	float G_FactorG;
	float G_OffsetB;
	float G_FactorB;
};
#pragma pack (pop)

// **************************************************************************************************************************************************** //
// The manager and generator
class TextureManager
{
public:
	// Different-general-noise Textures
	// Index 0: White-Noise
	// Index 1: Perlin-Noise
	Texture m_aNoiseTexture[2];

	Texture m_aBarkTextures[3];

	// Textures for stones and ground
	// Stone, Grass, Forest, Stone 2 (underground)
	Texture m_aTerrainTextures[4];

	Texture m_aLeafTextures[3];

	// **************************************************************************************************************************************************** //
	// Functions

	// Singleton
	static TextureManager& Get();

	// generates all textures
	bool Initialize();

	// Destroys all textures
	void Shutdown();

	// Creates a rendertarget with given size and format
	Texture CreateTexture(const unsigned int _Width, const unsigned int _Height, const D3DFORMAT _Format);

	// Calculates the Perlin-noise-generated height at given Point
	// Only uses Octaves between 0 and 7 (included). The low one should be
	// smaller than the second. Danger: No error testing!
	float PerlinHeight(float _x, float _y, int _LowOctave, int _HighOctave, float (*lerp)(float));

private:
	TextureManager() {}
	~TextureManager(){}

	// ~1MB Noise for Processor-Lookup. It contains the same values like
	// m_aNoiseTexture[0], but they are scaled to [-1,1]
	float m_aNoise[4][WHITE_NOISE_TEXTURE_SIZE][WHITE_NOISE_TEXTURE_SIZE];

	void GenerateWhiteNoiseTexture();
	void GeneratePerlinNoiseTexture();

	void GenerateArbitaryTexture(LPDIRECT3DTEXTURE9* _ppTex, const GeneratorGEN* _pGEN);

	void GenerateLeafTexture();
};