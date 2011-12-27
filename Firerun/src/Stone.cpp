#include "stdafx.h"
#include "Stone.h"
#include "Renderer.h"

const DWORD StdVertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
const DWORD StoneDetail = 16;
const DWORD StoneNumVertices = StoneDetail*(StoneDetail-2)+2;
const DWORD StoneNumIndices = 2*3*((StoneDetail-1)*(StoneDetail-3)+StoneDetail-1);

// **************************************************************************************************************************************************** //
Stone::Stone(const float _fa,
		const float _fb,
		const float _fc)
{
	// alloc buffers for the instance
	Renderer::Get().m_pD3DDevice->CreateVertexBuffer(StoneNumVertices*sizeof(StdVertex),
													 D3DUSAGE_WRITEONLY,
													 StdVertex::FVF,
													 D3DPOOL_DEFAULT,
													 &m_pVertexBuffer,
													 nullptr);

	Renderer::Get().m_pD3DDevice->CreateIndexBuffer(StoneNumIndices*2,
													D3DUSAGE_WRITEONLY,
													D3DFMT_INDEX16,
													D3DPOOL_DEFAULT,
													&m_pIndexBuffer,
													nullptr);

	// lock buffers to edit
	StdVertex* pVertexbuffer;
	m_pVertexBuffer->Lock(0, 0, (void**)&pVertexbuffer, 0);
	WORD* pIndexbuffer;
	m_pIndexBuffer->Lock(0, 0, (void**)&pIndexbuffer, 0);

	// Minimum size for noise-amplitude
	float fMinS = min(min(_fa,_fb),_fc);

	// first cup - vertex
	pVertexbuffer->pos.x = 0.0f + TextureManager::Get().PerlinHeight(0.0f, 0.0f, 0, 4, nullptr);
	pVertexbuffer->pos.y = 0.0f;
	pVertexbuffer->pos.z = _fc;
	pVertexbuffer->nx = 0.0f;
	pVertexbuffer->ny = 0.0f;
	pVertexbuffer->nz = 1.0f;
	pVertexbuffer->u = 0.5f;
	pVertexbuffer->v = 1.0f;
	++pVertexbuffer;
	
	m_RadiusSq = -1.0f;

	// first trianglefan
	for(int u=0;u<StoneDetail-1;++u)
	{
		(*(pIndexbuffer++)) = 0;
		(*(pIndexbuffer++)) = u+1;
		(*(pIndexbuffer++)) = (u+1)%StoneDetail+1;
	}

	float fDetail = 1.0f/(float)(StoneDetail-1);

	// Die äußere Schleife läuft über die Z-Achse
	for(int t=1;t<StoneDetail-1;++t)
	{
		// Die innere Schleife Erzeugt je einen Ring
		for(int u=0;u<StoneDetail;)	// Counter is increased below to safe performance
		{
			// Radial frequency / segmentation of the sphere
			float ft = g_PI*t*fDetail;
			float fu = (u*fDetail-0.5f)*2.0f*g_PI;
			float OrSin_t = sin(ft);
			float OrCos_t = cos(ft);
			float OrSin_u = sin(fu);
			float OrCos_u = cos(fu);

			// calculate normale for sphere with cross product of partial derivations.
			float nx = _fb * _fc * OrSin_t * OrSin_t * OrCos_u;
			float ny = _fa * _fc * OrSin_t * OrSin_t * OrSin_u;
			float nz = _fa * _fb *			 OrSin_t * OrCos_t;
			float fLen = 1.0f/sqrt(nx*nx+ny*ny+nz*nz);
			//pVertexbuffer->nx = nx*fLen;
			//pVertexbuffer->ny = ny*fLen;
			//pVertexbuffer->nz = nz*fLen;

			srand(nx*1137.0f+ny*44077.0f+nz*92111.0f);
			float fRand = fMinS*fLen*rand()/100000.0f;

			// Position aus Parameterform
			pVertexbuffer->pos.x = (0.05f+_fa) * ( OrSin_t * OrCos_u + nx*fRand);
			pVertexbuffer->pos.y = (0.05f+_fb) * ( OrSin_t * OrSin_u + ny*fRand);
			pVertexbuffer->pos.z = (0.05f+_fc) * ( OrCos_t + nz*fRand);

			float l = D3DXVec3LengthSq(&pVertexbuffer->pos);
			m_RadiusSq = max(m_RadiusSq, l);

			// Add noise to normals too
			// OPT: Andreas rand funktion
			nx += (rand()/32767.0f-0.5f);
			ny += (rand()/32767.0f-0.5f);
			nz += (rand()/32767.0f-0.5f);
			fLen = 1.0f/sqrt(nx*nx+ny*ny+nz*nz);
			pVertexbuffer->nx = nx*fLen;
			pVertexbuffer->ny = ny*fLen;
			pVertexbuffer->nz = nz*fLen;

			// cubic texture coordinates
			// similar to environementmapping
			// A rectangle is spanned over the circle.
			if(fabs(OrSin_u)<0.5f && OrCos_u>0.0f)		// right side
				pVertexbuffer->u = OrSin_u+0.5f;
			else if(fabs(OrSin_u)<0.5f && OrCos_u<0.0f)	// left side
				pVertexbuffer->u = 0.5f-OrSin_u;
			else if(OrSin_u>0.0f)						// upper side
				pVertexbuffer->u = OrCos_u+0.5f;
			else pVertexbuffer->u = 0.5f-OrCos_u;		// lower side

			if(fabs(OrSin_t)<0.5f && OrCos_t>0.0f)		// right side
				pVertexbuffer->v = OrSin_t+0.5f;
			else if(fabs(OrSin_t)<0.5f && OrCos_t<0.0f)	// left side
				pVertexbuffer->v = 0.5f-OrSin_t;
			else if(OrSin_t>0.0f)						// upper side
				pVertexbuffer->v = OrCos_t+0.5f;
			else pVertexbuffer->v = 0.5f-OrCos_t;		// lower side

			++pVertexbuffer;

			++u;
			if(u<StoneDetail && t<StoneDetail-2)
			{
				(*(pIndexbuffer++)) = u%StoneDetail+1+StoneDetail*(t-1);
				(*(pIndexbuffer++)) = u+StoneDetail*(t-1);
				WORD wIndex = u%StoneDetail+1+StoneDetail*t;
				(*(pIndexbuffer++)) = wIndex;

				(*(pIndexbuffer++)) = u+StoneDetail*(t-1);
				(*(pIndexbuffer++)) = min(u+StoneDetail*t,StoneNumVertices-1);
				(*(pIndexbuffer++)) = wIndex;
			}
		}
	}

	// last closing point
	pVertexbuffer->pos.x = 0.0f;
	pVertexbuffer->pos.y = 0.0f;
	pVertexbuffer->pos.z = -_fc;
	pVertexbuffer->nx = 0.0f;
	pVertexbuffer->ny = 0.0f;
	pVertexbuffer->nz = -1.0f;
	pVertexbuffer->u = 0.5f;
	pVertexbuffer->v = 0.0f;
	++pVertexbuffer;

	for(int u=0;u<StoneDetail-1;++u)
	{
		(*(pIndexbuffer++)) = StoneNumVertices-1;
		(*(pIndexbuffer++)) = StoneNumVertices-1-u-1;
		(*(pIndexbuffer++)) = StoneNumVertices-1-(u+1)%StoneDetail-1;
	}

	// Apply changes.
	m_pVertexBuffer->Unlock();
	m_pIndexBuffer->Unlock();
}

// **************************************************************************************************************************************************** //
Stone::~Stone()
{
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
}

// **************************************************************************************************************************************************** //
void Stone::Render()
{
	// set stone (granite) texture
	Renderer::Get().m_pD3DDevice->SetTexture(0, TextureManager::Get().m_aTerrainTextures[3].m_pTex);
	// render mesh
	Renderer::Get().m_pD3DDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(StdVertex));
	Renderer::Get().m_pD3DDevice->SetIndices(m_pIndexBuffer);
	Renderer::Get().m_pD3DDevice->SetFVF(StdVertex::FVF);
	Renderer::Get().m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, StoneNumVertices, 0, StoneNumIndices/3);
}


// **************************************************************************************************************************************************** //
StoneInstance::StoneInstance(const float _fx,
		const float _fy,
		const float _fz,
		Stone* _pStone)
{
	// Static transformation with random rotation
	D3DXMatrixRotationYawPitchRoll(&m_Transform, rand(), rand(), rand());
	m_Position.x = m_Transform._41 = _fx;
	m_Position.y = m_Transform._42 = _fy;
	m_Position.z = m_Transform._43 = _fz;


	float det = D3DXMatrixDeterminant(&m_Transform);
	D3DXMatrixInverse(&m_TransformInv, &det, &m_Transform);

	m_pStone = _pStone;
}

void StoneInstance::Render(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos)
{
	Renderer::Get().m_pD3DDevice->SetVertexShaderConstantF(0, m_Transform * ViewProjection, 4);
	Renderer::Get().m_pD3DDevice->SetVertexShaderConstantF(4, m_Transform * View, 4);
	D3DXVECTOR3 temp;
	Renderer::Get().m_pD3DDevice->SetVertexShaderConstantF(7, *D3DXVec3TransformCoord(&temp, &ViewPos, &m_TransformInv), 1);
	
	m_pStone->Render();
}