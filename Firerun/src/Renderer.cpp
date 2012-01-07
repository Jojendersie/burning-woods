#include "stdafx.h"
#include "Renderer.h"
#include "World.h"
#include "Texturemanager.h"
#include "Stone.h"

#include "Fire.h"

// For the min function
#include <algorithm>

#include "Shader.h"

#define NVIDIA_PERFHUD_POSSIBLE
//#define TERRAIN_WIRE

// FOV
const float Renderer::m_FOVMin = (62.0f/180.0f);
const float Renderer::m_FOVMax = (87.0f/180.0f);
const float Renderer::m_FOVVelocityFactor = 0.01f;

const float Renderer::m_RadialBlurStepVelocityFactor = 0.2f;
const float Renderer::m_RadialBlurWeightVelocityFactor = 1.8f;

// **************************************************************************************************************************************************** //
Renderer& Renderer::Get()
{
	static Renderer TheOneAndOnly;
	return TheOneAndOnly;
}

// **************************************************************************************************************************************************** //
bool Renderer::Initialize(const unsigned int _ResolutionX, const unsigned int _ResolutionY, const HWND _WindowHandle, const bool _Windowed, const bool _VSync)
{
	memset(m_LightList, 0, sizeof(Light)*MAX_LIGHTS);

	m_ResolutionX = _ResolutionX;
	m_ResolutionY = _ResolutionY;

	// D3D Device erstellen
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(!m_pD3D)
		return false; //throw "Can't create D3D-Object!";	

	// Present params
	D3DPRESENT_PARAMETERS PresentParams;
	ZeroMemory(&PresentParams, sizeof(D3DPRESENT_PARAMETERS));
	if(!_Windowed)
	{
		PresentParams.BackBufferWidth				= _ResolutionX;
		PresentParams.BackBufferHeight				= _ResolutionY;
	}
	PresentParams.BackBufferFormat				= D3DFMT_X8R8G8B8;
	PresentParams.BackBufferCount				= 1;
//	PresentParams.MultiSampleType				= D3DMULTISAMPLE_NONE;
//	PresentParams.MultiSampleQuality			= 0;
	PresentParams.SwapEffect					= D3DSWAPEFFECT_DISCARD;
	PresentParams.hDeviceWindow					= _WindowHandle;
	PresentParams.Windowed						= _Windowed;
//	PresentParams.EnableAutoDepthStencil		= false;
//	PresentParams.AutoDepthStencilFormat		= D3DFMT_D24S8;
//	PresentParams.Flags							= 0;
//	PresentParams.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;
	PresentParams.PresentationInterval			= _VSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;


	// D3D Interface
	unsigned int Adapter = 0;
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;
#ifdef NVIDIA_PERFHUD_POSSIBLE
	// Look for 'NVIDIA PerfHUD' adapter
	// If it is present, override default settings
	for(unsigned int i=0; i<m_pD3D->GetAdapterCount(); i++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		m_pD3D->GetAdapterIdentifier(i, 0, &Identifier);
		if(strstr(Identifier.Description, "PerfHUD") != 0)
		{
			Adapter = i;
			DeviceType = D3DDEVTYPE_REF;
			break;
		}
	}
#endif

	if(FAILED(m_pD3D->CreateDevice(Adapter, DeviceType, _WindowHandle, D3DCREATE_HARDWARE_VERTEXPROCESSING, &PresentParams, &m_pD3DDevice)))
		return false;

	m_BackBufferPixelSize.x = 1.0f / _ResolutionX;
	m_BackBufferPixelSize.y = 1.0f / _ResolutionY;

	// GetBackbuffer
	m_pD3DDevice->GetRenderTarget(0, &m_pBackBuffer);

	// create depth stencil
	m_pD3DDevice->CreateDepthStencilSurface(_ResolutionX, _ResolutionY, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, true, &m_pDepthStencilBuffer, 0);
	m_pD3DDevice->SetDepthStencilSurface(m_pDepthStencilBuffer);

	// ZBuffering always on
	m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, true);

	// create GBuffer
	m_pGBuffer0 = TextureManager::Get().CreateTexture(_ResolutionX, _ResolutionY, D3DFMT_A8R8G8B8);
	m_pGBuffer1 = TextureManager::Get().CreateTexture(_ResolutionX, _ResolutionY, D3DFMT_G16R16F);
	m_pGBuffer2 = TextureManager::Get().CreateTexture(_ResolutionX, _ResolutionY, D3DFMT_R32F);

	// create HDR related Textures
	m_pHDRScreenBuffer = TextureManager::Get().CreateTexture(_ResolutionX, _ResolutionY, D3DFMT_A16B16G16R16F);
	m_pOldLuminance = TextureManager::Get().CreateTexture(1, 1, D3DFMT_R16F);
	m_pLuminance[0] = TextureManager::Get().CreateTexture(1, 1, D3DFMT_R16F);
	m_pLuminance[1] = TextureManager::Get().CreateTexture(4, 4, D3DFMT_R16F);
	m_pLuminance[2] = TextureManager::Get().CreateTexture(16, 16, D3DFMT_R16F);
	m_pLuminance[3] = TextureManager::Get().CreateTexture(64, 64, D3DFMT_R16F);
	m_pLuminance[4] = TextureManager::Get().CreateTexture(256, 256, D3DFMT_R16F);
	int BrightPassX = _ResolutionX * BRIGHTPASS_TEXTURE_SIZE;
	int BrightPassY = _ResolutionY * BRIGHTPASS_TEXTURE_SIZE;
	m_pBrightPass[0] = TextureManager::Get().CreateTexture(BrightPassX, BrightPassY, D3DFMT_A8R8G8B8);
	m_pBrightPass[1] = TextureManager::Get().CreateTexture(BrightPassX, BrightPassY, D3DFMT_A8R8G8B8);

	// backbuffer copy
	m_pPreBackBuffer = TextureManager::Get().CreateTexture(_ResolutionX, _ResolutionY, D3DFMT_A8R8G8B8);

	// Create Shader
		// Terrain
	m_pD3DDevice->CreatePixelShader(g_aterrainPS, &m_pTerrainPS);
	m_pD3DDevice->CreateVertexShader(g_aterrainVS, &m_pTerrainVS);

		// Quad with rays
	m_pD3DDevice->CreateVertexShader(g_aScreenQuadWithRaysVS, &m_pViewRayQuadVS);
		// Deferred Shading - Dirlight, Ambient, Fog
	m_pD3DDevice->CreatePixelShader(g_aDeferredDirLightPS, &m_pDirLightPS);
	m_pD3DDevice->CreatePixelShader(g_aDeferredPointLightPS, &m_pPointLightPS);

		// HDR Stuff
	m_pD3DDevice->CreatePixelShader(g_aBrightPassPS, &m_pBrightPassPS);
	m_pD3DDevice->CreatePixelShader(g_aGaussianBlurPS, &m_pGaussianBlurPS);
	m_pD3DDevice->CreatePixelShader(g_aluminance_intialPS, &m_pLuminanceInitialPS);
	m_pD3DDevice->CreatePixelShader(g_aluminance_downsamplePS, &m_pLuminanceDownsamplePS);
	m_pD3DDevice->CreatePixelShader(g_aluminance_lastPS, &m_pLuminanceLastPS);
	m_pD3DDevice->CreatePixelShader(g_aToneMappingFinishPS, &m_pTonemappinFinishPS);

		// Sky
	m_pD3DDevice->CreatePixelShader(g_aSkyPS, &m_pSkyPS);

		// Radial Blur
	m_pD3DDevice->CreatePixelShader(g_aradialBlurPS, &m_pRadialBlurPS);

		// Stone
	m_pD3DDevice->CreatePixelShader(g_aStonePS, &m_pStonePS);
	m_pD3DDevice->CreateVertexShader(g_aStoneVS, &m_pStoneVS);	

			// Stone
	m_pD3DDevice->CreatePixelShader(g_afirePS, &m_pFirePS);
	m_pD3DDevice->CreateVertexShader(g_afireVS, &m_pFireVS);	


	// Luminance Sampleoffsets
	float Offset = 0.6f / 256.0f;
	float OffsetsInitial[8] = { Offset, Offset,   -Offset, -Offset,   -Offset, Offset,   Offset, -Offset };
	memcpy(m_LuminanceInitialSampling, OffsetsInitial, sizeof(float)*8);
	int size = 1;
	for(int i=0; i<4; ++i)
	{
		Offset = 1.0f / size;
		size *= 4;
		float Offsets[] = { 0,Offset,   0,-Offset,   Offset,0,   -Offset,0,    Offset,Offset,   -Offset,-Offset,    Offset,-Offset,    -Offset,Offset  };
		memcpy(m_LuminanceSamplingOffsets[i], Offsets, sizeof(float)*16);
	}

	// Brightpass downsample
	float OffsetX = m_BackBufferPixelSize.x;
	float OffsetY = m_BackBufferPixelSize.y;
	float OffsetsBrightpass[] = { 0,OffsetY,   0,-OffsetY,   OffsetX,0,   -OffsetX,0,    OffsetX,OffsetY,   -OffsetX,-OffsetY,    OffsetX,-OffsetY,    -OffsetX,OffsetY  };
	memcpy(m_BrightPassSamplingOffsets, OffsetsBrightpass, sizeof(float)*16);


	// Gaussian Blur Fliter - all values are precomputed for optimal 15 sample blur
	OffsetX = 1.0f/BrightPassX;
	OffsetY = 1.0f/BrightPassY;
		// other samples
	for(int i = 0; i < 12; i++)
	{
		int iInv = 11-i;
		m_BrightPassBlurOffsets_Horizontal[i][0] = iInv * OffsetX;
		m_BrightPassBlurOffsets_Horizontal[i][1] = 0.0f;
		m_BrightPassBlurOffsets_Horizontal[i][2] = -m_BrightPassBlurOffsets_Horizontal[i][0];
		m_BrightPassBlurOffsets_Horizontal[i][3] = 0.0f;
		
		m_BrightPassBlurOffsets_Vertical[i][0] = 0.0f;
		m_BrightPassBlurOffsets_Vertical[i][1] = iInv * OffsetY;
		m_BrightPassBlurOffsets_Vertical[i][2] = 0.0f;
		m_BrightPassBlurOffsets_Vertical[i][3] = -m_BrightPassBlurOffsets_Vertical[i][1];
	}

	// universal Quad vb
	m_pD3DDevice->CreateVertexBuffer(sizeof(BillboardVertex)*4, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &m_pBillboardVB, NULL);
	m_pD3DDevice->CreateIndexBuffer(sizeof(WORD)*6, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pBillboardIB, NULL);
	WORD* indices;
	m_pBillboardIB->Lock(0, sizeof(WORD)*6, (void**)&indices, D3DLOCK_DISCARD);
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 3;
	indices[3] = 1;
	indices[4] = 2;
	indices[5] = 3;
	m_pBillboardVB->Unlock();

	// fire
	m_pD3DDevice->CreateVertexDeclaration(Fire::FireVertexDeclElements, &Fire::pVertexDecl);

	return true;
}

Renderer::Renderer() :
	m_DirectionalLightDirection(0.577f, 0.577f, 0.577f),	// 1, 1, 1 normalized
	m_DirectionalLightDiffuseColor(0.6f, 0.6f, 0.64f),
	m_AmbientColor(0.2f, 0.2f, 0.24f)
{
}

// **************************************************************************************************************************************************** //
Renderer::~Renderer()
{
	// OPT - remove release calls
	Fire::pVertexDecl->Release();

	m_pBillboardVB->Release();
	m_pBillboardIB->Release();

	m_pFirePS->Release();
	m_pFireVS->Release();
	m_pBrightPassPS->Release();
	m_pGaussianBlurPS->Release();	// 15taps
	m_pLuminanceInitialPS->Release();
	m_pLuminanceDownsamplePS->Release();
	m_pLuminanceLastPS->Release();
	m_pTonemappinFinishPS->Release();
	m_pTerrainPS->Release();
	m_pTerrainVS->Release();
	m_pSkyPS->Release();
	m_pRadialBlurPS->Release();
	m_pStonePS->Release();
	m_pStoneVS->Release();
	m_pPointLightPS->Release();
	m_pDirLightPS->Release();

	m_pDepthStencilBuffer->Release();
	m_pD3D->Release();
	m_pD3DDevice->Release();
}

// **************************************************************************************************************************************************** //
void Renderer::SetTextureFilter(const unsigned int Stage, const _D3DTEXTUREFILTERTYPE filter)
{
	m_pD3DDevice->SetSamplerState(Stage, D3DSAMP_MINFILTER, filter);
	m_pD3DDevice->SetSamplerState(Stage, D3DSAMP_MAGFILTER, filter);
	m_pD3DDevice->SetSamplerState(Stage, D3DSAMP_MIPFILTER, filter); 	
}
void Renderer::SetTextureAdressing(const unsigned int Stage, const _D3DTEXTUREADDRESS RepeatMode)
{
	m_pD3DDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSU, RepeatMode);
	m_pD3DDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSV, RepeatMode);
}

// **************************************************************************************************************************************************** //
bool Renderer::Draw(const D3DXMATRIX& ViewMatrix, const D3DXVECTOR3& CameraPos, D3DXVECTOR3& CameraDir, const float PlayerVelocity)
{
	// Setting matrices
	D3DXMATRIX ProjectionMatrix;
	float FOV = std::min<float>(m_FOVMax, m_FOVMin + PlayerVelocity*m_FOVVelocityFactor);
	float ScreenRatio = static_cast<float>(m_ResolutionX)/m_ResolutionY;
	D3DXMatrixPerspectiveFovLH(&ProjectionMatrix, FOV, ScreenRatio, 0.1f, 10000.0f);
	m_pD3DDevice->SetTransform(D3DTS_PROJECTION, &ProjectionMatrix);	// obsolet?
	m_pD3DDevice->SetTransform(D3DTS_VIEW, &ViewMatrix);
	D3DXMATRIX ident; D3DXMatrixIdentity(&ident);
	m_pD3DDevice->SetTransform(D3DTS_WORLD, &ident);
	D3DXMATRIX ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;

	D3DXVECTOR2 CameraDir2D(CameraDir.x,CameraDir.z);
	D3DXVec2Normalize(&CameraDir2D, &CameraDir2D);


	// make billboard billboardlike ;)
	BillboardVertex* vertices;
	m_pBillboardVB->Lock(0, sizeof(BillboardVertex)*4, (void**)&vertices, D3DLOCK_DISCARD);
	D3DXVECTOR3 cameraX(ViewMatrix._11, ViewMatrix._21, ViewMatrix._31);
	D3DXVECTOR3 cameraY(ViewMatrix._12, ViewMatrix._22, ViewMatrix._32);
	vertices[0].position = -cameraX-cameraY;
	vertices[1].position = +cameraX-cameraY;
	vertices[2].position = +cameraX+cameraY;
	vertices[3].position = -cameraX+cameraY;
	vertices[0].texcoord = D3DXVECTOR2(0.0f, 0.0f);
	vertices[1].texcoord = D3DXVECTOR2(1.0f, 0.0f);
	vertices[2].texcoord = D3DXVECTOR2(1.0f, 1.0f);
	vertices[3].texcoord = D3DXVECTOR2(0.0f, 1.0f);	// positions are updated framewise
	m_pBillboardVB->Unlock();


	// Begin Szene
	m_pD3DDevice->BeginScene();
	
	// --------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------
	m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, true);

	// --------------------------------------------------------------------------------------
	// GBuffer Pass
	m_pGBuffer0.SetAsTarget(0);
	m_pGBuffer1.SetAsTarget(1);
	m_pGBuffer2.SetAsTarget(2);
	m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

	
#pragma region STONES
	// Stones
	SetTextureFilter(0, D3DTEXF_LINEAR);
	SetTextureAdressing(0, D3DTADDRESS_WRAP);
	m_pD3DDevice->SetVertexShader(m_pStoneVS);
	m_pD3DDevice->SetPixelShader(m_pStonePS);
	for(int i=0;i<World::Get().m_NumBuckets;++i)
		for(int j=0;j<World::Get().m_NumBuckets;++j)
		{
			Bucket* pBucket = &World::Get().m_Buckets[i][j];
			D3DXVECTOR2 vec(pBucket->m_x - CameraPos.x + CameraDir2D.x*64.0f, pBucket->m_z - CameraPos.z + CameraDir2D.y*64.0f);
			D3DXVec2Normalize(&vec, &vec);
			if(D3DXVec2Dot(&vec, &CameraDir2D) > 0.4f)
				pBucket->Render(ViewMatrix, ViewProjectionMatrix, CameraPos);
		}
#pragma endregion
		
#pragma region TERRAIN
	// Terrain
#ifdef TERRAIN_WIRE
	m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
#endif
		// World matrix - dark matrices magic
	m_pD3DDevice->SetPixelShader(m_pTerrainPS);
	m_pD3DDevice->SetVertexShader(m_pTerrainVS);
	m_pD3DDevice->SetVertexShaderConstantF(0, ViewProjectionMatrix, 4);
	m_pD3DDevice->SetVertexShaderConstantF(4, ViewMatrix, 3);
	m_pD3DDevice->SetVertexShaderConstantF(8, CameraPos, 1);

		// YUpViewSpace
	D3DXVECTOR3 YUpViewSpace; D3DXVec3TransformNormal(&YUpViewSpace, &D3DXVECTOR3(0,1,0), &ViewMatrix);
	m_pD3DDevice->SetPixelShaderConstantF(0, YUpViewSpace, 1);

	//SetTextureAdressing(0, D3DTADDRESS_WRAP);
	SetTextureAdressing(1, D3DTADDRESS_WRAP);
	SetTextureAdressing(2, D3DTADDRESS_WRAP);
	SetTextureFilter(0, D3DTEXF_LINEAR);
	SetTextureFilter(1, D3DTEXF_LINEAR);
	SetTextureFilter(2, D3DTEXF_LINEAR);
	SetTextureFilter(3, D3DTEXF_LINEAR);
	World::Get().DrawTerrain(CameraPos, CameraDir2D, FOV*ScreenRatio);

#ifdef TERRAIN_WIRE
	m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
#endif

#pragma endregion
	
	// --------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------

	m_pD3DDevice->SetVertexShader(m_pViewRayQuadVS);

#pragma region LIGHTING
	// --------------------------------------------------------------------------------------
	// Lighting Pass
	D3DSURFACE_DESC Desc;
	m_pBackBuffer->GetDesc(&Desc);
	g_aQuad[1].x = Desc.Width - 0.5f;
										g_aQuad[2].y = Desc.Height - 0.5f;
	g_aQuad[3].x = Desc.Width - 0.5f;	g_aQuad[3].y = Desc.Height - 0.5f;
	m_pD3DDevice->SetFVF(QuadVertex::FVF);

	// Viewspace Viewdir in vertices
	float TanHalfFov = tanf(FOV);
	D3DXVECTOR3 CameraUpperRightCorner(TanHalfFov, TanHalfFov / ScreenRatio, 1.0f);
	D3DXVec3Normalize(&CameraUpperRightCorner, &CameraUpperRightCorner);
	g_aQuad[0].ViewDir = CameraUpperRightCorner; g_aQuad[0].ViewDir.x = -g_aQuad[0].ViewDir.x;
	g_aQuad[1].ViewDir = CameraUpperRightCorner; 
	g_aQuad[2].ViewDir = -CameraUpperRightCorner; g_aQuad[2].ViewDir.z = -g_aQuad[2].ViewDir.z;
	g_aQuad[3].ViewDir = CameraUpperRightCorner; g_aQuad[3].ViewDir.y = -g_aQuad[3].ViewDir.y;

	// Set GBuffer
	m_pHDRScreenBuffer.SetAsTarget(0);
	m_pD3DDevice->SetRenderTarget(1, NULL);
	m_pD3DDevice->SetRenderTarget(2, NULL);
	m_pD3DDevice->SetTexture(0, m_pGBuffer0.m_pTex);
	m_pD3DDevice->SetTexture(1, m_pGBuffer1.m_pTex);
	m_pD3DDevice->SetTexture(2, m_pGBuffer2.m_pTex); 
	SetTextureFilter(0, D3DTEXF_NONE);
	SetTextureFilter(1, D3DTEXF_NONE);
	SetTextureFilter(2, D3DTEXF_NONE);
	SetTextureAdressing(0, D3DTADDRESS_CLAMP);
	SetTextureAdressing(1, D3DTADDRESS_CLAMP);
	SetTextureAdressing(2, D3DTADDRESS_CLAMP);

	// clip background
	m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
	m_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_NOTEQUAL);

	// Shader
//   LightColor            c0       1
//   DirLightDir_ViewSpace c1       1
//   AmbientLight          c2       1
//   sampGBuffer0          s0       1
//   sampGBuffer1          s1       1
	m_pD3DDevice->SetPixelShaderConstantF(0, m_DirectionalLightDiffuseColor, 1);
	D3DXVECTOR3 DirLightViewSpace; D3DXVec3TransformNormal(&DirLightViewSpace, &m_DirectionalLightDirection, &ViewMatrix);
	m_pD3DDevice->SetPixelShaderConstantF(1, DirLightViewSpace, 1);
	m_pD3DDevice->SetPixelShaderConstantF(2, m_AmbientColor, 1);
	m_pD3DDevice->SetPixelShader(m_pDirLightPS);
	
	// Draw
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));

	// point lights
	m_pD3DDevice->SetPixelShader(m_pPointLightPS);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	for(unsigned int i=0; i<MAX_LIGHTS; ++i)
	{
		if(!m_LightList[i].Active)
			continue;

		// cull

		// set prop
		//   LightColor              c0       1
		//   LightPosition_ViewSpace c1       1
		//   RangeSq                 c2       1
		//   sampGBuffer0            s0       1
		//   sampGBuffer1            s1       1
		//   sampGBuffer2            s2       1
		m_pD3DDevice->SetPixelShaderConstantF(0, m_LightList[i].Color, 1);
		D3DXVECTOR3 LightPosView;
		D3DXVec3TransformCoord(&LightPosView, &m_LightList[i].Position, &ViewMatrix);
		m_pD3DDevice->SetPixelShaderConstantF(1, LightPosView, 1);
		m_pD3DDevice->SetPixelShaderConstantF(2, &m_LightList[i].RangeSq, 1);
		
		// draw
		m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));
	}
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
#pragma endregion

#pragma region SKY
	// --------------------------------------------------------------------------------------
	// Sky Pass
	m_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
		// Viewdir in vertices
	D3DXMATRIX InvViewMatrix; D3DXMatrixInverse(&InvViewMatrix, NULL, &ViewMatrix);
	D3DXVec3TransformNormal(&g_aQuad[0].ViewDir, &g_aQuad[0].ViewDir, &InvViewMatrix);
	D3DXVec3TransformNormal(&g_aQuad[1].ViewDir, &g_aQuad[1].ViewDir, &InvViewMatrix);
	D3DXVec3TransformNormal(&g_aQuad[2].ViewDir, &g_aQuad[2].ViewDir, &InvViewMatrix);
	D3DXVec3TransformNormal(&g_aQuad[3].ViewDir, &g_aQuad[3].ViewDir, &InvViewMatrix);
	
	Renderer::Get().m_pD3DDevice->SetTexture(0, TextureManager::Get().m_aNoiseTexture[0].m_pTex);
	Renderer::Get().m_pD3DDevice->SetTexture(1, TextureManager::Get().m_aNoiseTexture[1].m_pTex);
	SetTextureAdressing(0, D3DTADDRESS_WRAP);
	SetTextureAdressing(1, D3DTADDRESS_WRAP);
	SetTextureFilter(0, D3DTEXF_LINEAR);
	SetTextureFilter(1, D3DTEXF_LINEAR);
//	m_pD3DDevice->SetVertexShader(m_pSkyVS);	// using also the quad with rays
	m_pD3DDevice->SetPixelShader(m_pSkyPS);
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));


	m_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
#pragma endregion

#pragma region FIRE
	// testfire
	static Fire* fire = nullptr;
	if(!fire)
		fire = new Fire(D3DXVECTOR3(0.0f,0.0f,0.0f));

	m_pD3DDevice->SetFVF(0);
	m_pD3DDevice->SetVertexDeclaration(Fire::pVertexDecl);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pD3DDevice->SetStreamSource(0, m_pBillboardVB, 0, sizeof(BillboardVertex));
    m_pD3DDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NUM_PARTICLES_PER_FIRE);

	m_pD3DDevice->SetIndices(m_pBillboardIB);

	m_pD3DDevice->SetPixelShader(m_pFirePS);
	m_pD3DDevice->SetVertexShader(m_pFireVS);

	//Renderer::Get().m_pD3DDevice->SetTexture(0, TextureManager::Get().m_aNoiseTexture[0].m_pTex);
	m_pD3DDevice->SetVertexShaderConstantF(4, &g_PassedTime, 1);
	m_pD3DDevice->SetVertexShaderConstantF(5, Fire::fireStartColor, 1);
	m_pD3DDevice->SetVertexShaderConstantF(6, Fire::fireMidColor, 1);
	m_pD3DDevice->SetVertexShaderConstantF(7, Fire::fireEndColor, 1);
	m_pD3DDevice->SetVertexShaderConstantF(8, &Fire::fireStartScale, 1);
	m_pD3DDevice->SetVertexShaderConstantF(9, &Fire::fireEndScale, 1);

	fire->Draw(ViewMatrix, ViewProjectionMatrix, CameraPos);

	m_pD3DDevice->SetStreamSourceFreq(0, 1);
	m_pD3DDevice->SetStreamSourceFreq(1, 1);
	m_pD3DDevice->SetVertexDeclaration(nullptr);
	m_pD3DDevice->SetFVF(QuadVertex::FVF);

	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

//	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

#pragma endregion

#pragma region HDR_TONEMAP
	m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, false);
	m_pD3DDevice->SetVertexShader(m_pViewRayQuadVS);

	// --------------------------------------------------------------------------------------
	// HDR & Tonemapping
	SetTextureAdressing(0, D3DTADDRESS_CLAMP);
	SetTextureAdressing(1, D3DTADDRESS_CLAMP);

		// Inital
	m_pLuminance[4].SetAsTarget(0);
	m_pD3DDevice->SetTexture(0, m_pHDRScreenBuffer.m_pTex);
	m_pD3DDevice->SetPixelShaderConstantF(0, m_LuminanceInitialSampling, 2);
	m_pD3DDevice->SetPixelShader(m_pLuminanceInitialPS);
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));
		// downsample
	m_pD3DDevice->SetPixelShader(m_pLuminanceDownsamplePS);
	for(int i=3; i>0; i--)
	{
		m_pLuminance[i].SetAsTarget(0);
		m_pD3DDevice->SetPixelShaderConstantF(0, m_LuminanceSamplingOffsets[i], 4);
		m_pD3DDevice->SetTexture(0, m_pLuminance[i+1].m_pTex);
		m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));
	}
		// Last
	m_pD3DDevice->SetPixelShader(m_pLuminanceLastPS);
	m_pLuminance[0].SetAsTarget(0);
	m_pD3DDevice->SetPixelShaderConstantF(0, m_LuminanceSamplingOffsets[0], 4);
	m_pD3DDevice->SetPixelShaderConstantF(4, &g_FrameTime, 1);
	m_pD3DDevice->SetTexture(0, m_pLuminance[1].m_pTex);
	m_pD3DDevice->SetTexture(1, m_pOldLuminance.m_pTex);
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));


	// Brightpass init
	m_pBrightPass[0].SetAsTarget(0);
	m_pD3DDevice->SetTexture(0, m_pHDRScreenBuffer.m_pTex);
	m_pD3DDevice->SetTexture(1, m_pLuminance[0].m_pTex);
	m_pD3DDevice->SetPixelShaderConstantF(0, m_BrightPassSamplingOffsets, 4);
	m_pD3DDevice->SetPixelShader(m_pBrightPassPS);
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));

	// Brightpass blur
	//SetTextureFilter(0, D3DTEXF_LINEAR);

	m_pBrightPass[1].SetAsTarget(0);
	m_pD3DDevice->SetTexture(0, m_pBrightPass[0].m_pTex);
	m_pD3DDevice->SetPixelShaderConstantF(0, m_BrightPassBlurOffsets_Horizontal[0], 12);
	m_pD3DDevice->SetPixelShader(m_pGaussianBlurPS);
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));

	m_pBrightPass[0].SetAsTarget(0);
	m_pD3DDevice->SetTexture(0, m_pBrightPass[1].m_pTex);
	m_pD3DDevice->SetPixelShaderConstantF(0, m_BrightPassBlurOffsets_Vertical[0], 12);
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));
	
	// combine
#pragma region FINAL
	// Realign screenquad
	m_pBackBuffer->GetDesc(&Desc);

	g_aQuad[1].x = Desc.Width - 0.5f;
										g_aQuad[2].y = Desc.Height - 0.5f;
	g_aQuad[3].x = Desc.Width - 0.5f;	g_aQuad[3].y = Desc.Height - 0.5f;

	
	m_pPreBackBuffer.SetAsTarget(0);
	m_pD3DDevice->SetTexture(0, m_pHDRScreenBuffer.m_pTex);	
	m_pD3DDevice->SetTexture(1, m_pLuminance[0].m_pTex);
	m_pD3DDevice->SetTexture(2, m_pBrightPass[0].m_pTex);	SetTextureFilter(2, D3DTEXF_LINEAR);
	m_pD3DDevice->SetPixelShader(m_pTonemappinFinishPS);
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));
#pragma endregion 
#pragma endregion

#pragma region RADIAL_BLUR
	m_pD3DDevice->SetRenderTarget(0, m_pBackBuffer);
	m_pD3DDevice->SetTexture(0, m_pPreBackBuffer.m_pTex);
	m_pD3DDevice->SetPixelShader(m_pRadialBlurPS);
	m_pD3DDevice->SetPixelShaderConstantF(0, m_BackBufferPixelSize * PlayerVelocity * m_RadialBlurStepVelocityFactor, 1);
	float weight = 1.0f / (PlayerVelocity + 0.0001f) * m_RadialBlurWeightVelocityFactor;
	m_pD3DDevice->SetPixelShaderConstantF(1, &weight, 1);
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_aQuad, sizeof(QuadVertex));
#pragma endregion


#ifdef TEXTURE_TEST
	// Texturegen test - do not delete
	m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_pD3DDevice->SetPixelShader(0);
	m_pD3DDevice->SetVertexShader(0);
	//m_pLuminance[2].Draw();
	switch(g_TexIndex)
	{
		case 0: TextureManager::Get().m_aNoiseTexture[0].Draw(); break;
		case 1: TextureManager::Get().m_aNoiseTexture[1].Draw(); break;
		case 2: TextureManager::Get().m_aBarkTextures[0].Draw(); break;
		case 3: TextureManager::Get().m_aBarkTextures[1].Draw(); break;
		case 4: TextureManager::Get().m_aLeafTextures[0].Draw(); break;
		case 5: TextureManager::Get().m_aTerrainTextures[0].Draw(); break;
		case 6: TextureManager::Get().m_aTerrainTextures[1].Draw(); break;
		case 7: TextureManager::Get().m_aTerrainTextures[2].Draw(); break;
	}
#endif


	// End Scene
	m_pD3DDevice->EndScene();

	if(FAILED(m_pD3DDevice->Present(NULL, NULL, NULL, NULL)))
		return false;

	// Swap Luminance
	IDirect3DTexture9* pTemp = m_pLuminance[0].m_pTex;
	m_pLuminance[0].m_pTex = m_pOldLuminance.m_pTex;
	m_pOldLuminance.m_pTex = pTemp;
	
	return true;
}

unsigned int Renderer::GetFreeLightPosition()
{
	for(unsigned int i=0; i<MAX_LIGHTS; ++i)
	{
		if(m_LightList[i].Active == false)
			return i;
	}
	return 0;
}