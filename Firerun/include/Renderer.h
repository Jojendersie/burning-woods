#include "TextureManager.h"


#define BRIGHTPASS_TEXTURE_SIZE 0.25f

// The Renderer
class Renderer
{
public:
	bool Initialize(const unsigned int _ResolutionX, const unsigned int _ResolutionY, const HWND _WindowHandle, 
							const bool _Windowed, const bool _VSync = true);

	// Singleton
	static Renderer& Get();

	// sets the texturefilter
	void SetTextureFilter(const unsigned int Stage, const _D3DTEXTUREFILTERTYPE filter);

	// sets the texture repeate mode
	void SetTextureAdressing(const unsigned int Stage, const _D3DTEXTUREADDRESS RepeatMode);

	// Draws the entire scene - returns false if a critical error occured
	// ViewMatrix		is determined from outside, but the projection is decided internally depending on velocity
	// CameraRotationY	is needed for terrain rendering
	bool Draw(const D3DXMATRIX& ViewMatrix, const D3DXVECTOR3& CameraPos, D3DXVECTOR3& CameraDir, const float PlayerVelocity);


	// lightstructure
	struct Light
	{
		D3DXVECTOR3 Position;
		D3DXVECTOR3 Color;
		float RangeSq;
		bool Active;
	};

	// determines a free index in m_LightList
	unsigned int GetFreeLightPosition();

	// public lightlist
	// to add a new light call GetFreeLightPosition and set m_LightList[index].Active to true
#define MAX_LIGHTS 1024
	Light m_LightList[MAX_LIGHTS];

	
	IDirect3DDevice9*	m_pD3DDevice;	
	//D3DCAPS9			m_Caps;		
	unsigned int		m_ResolutionX;
	unsigned int		m_ResolutionY;

	// FOV properties
	static const float m_FOVMin;
	static const float m_FOVMax;
	static const float m_FOVVelocityFactor;

private:
	Renderer();
	~Renderer();

	IDirect3D9*				m_pD3D;

	// Backbuffer
	IDirect3DSurface9* m_pBackBuffer;
	// DepthStencil
	IDirect3DSurface9* m_pDepthStencilBuffer;

	// -----------------------------------
	// Textures
	// GBuffer
	Texture m_pGBuffer0;	// Albedo + sign(Normal.z)
	Texture m_pGBuffer1;	// Normal XZ
	Texture m_pGBuffer2;	// Depth

	// HDR
	Texture m_pHDRScreenBuffer;
	Texture m_pOldLuminance;
	Texture m_pLuminance[5];
	Texture m_pBrightPass[2];

	// -----------------------------------
	// Shader
	// Terrain
	IDirect3DVertexShader9* m_pTerrainVS;
	IDirect3DPixelShader9*	m_pTerrainPS;

	// Quad with ViewRay
	IDirect3DVertexShader9* m_pViewRayQuadVS;

	// Deferred Shading
	IDirect3DPixelShader9* m_pDirLightPS; // Dirlight, Ambient, Fog
	IDirect3DPixelShader9* m_pPointLightPS;

	// HDR related
	IDirect3DPixelShader9*	m_pBrightPassPS;
	IDirect3DPixelShader9*	m_pGaussianBlurPS;	// 15taps
	IDirect3DPixelShader9*	m_pLuminanceInitialPS;
	IDirect3DPixelShader9*	m_pLuminanceDownsamplePS;
	IDirect3DPixelShader9*	m_pLuminanceLastPS;
	IDirect3DPixelShader9*	m_pTonemappinFinishPS;

	// Sky
	IDirect3DPixelShader9*	m_pSkyPS;

	// Stone
	IDirect3DPixelShader9*	m_pStonePS;
	IDirect3DVertexShader9*	m_pStoneVS;

	// fire
	IDirect3DPixelShader9*	m_pFirePS;
	IDirect3DVertexShader9*	m_pFireVS;

	// universal textured quad
	struct BillboardVertex
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texcoord;
	};
	IDirect3DVertexBuffer9* m_pBillboardVB;
	IDirect3DIndexBuffer9*  m_pBillboardIB;

	// -----------------------------------
	// Offsets, Weights and other Values
	// Luminance Sampleoffsets
	float	m_LuminanceInitialSampling[8];
	float	m_LuminanceSamplingOffsets[4][16];

	// Brightpass
	float	m_BrightPassSamplingOffsets[16];
	float	m_BrightPassBlurOffsets_Horizontal[12][4];	// without middle sample 
	float	m_BrightPassBlurOffsets_Vertical[12][4];

	// Sky/Athmosphere
	const D3DXVECTOR3 m_DirectionalLightDirection;
	const D3DXVECTOR3 m_DirectionalLightDiffuseColor;
	const D3DXVECTOR3 m_AmbientColor;
};