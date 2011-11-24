#include "StdAfx.h"
#include "Fire.h"
#include "Renderer.h"

const D3DVERTEXELEMENT9 Fire::FireVertexDeclElements[] =
{
{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
{1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},	// Direction
{1, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},	// lifetime, starttime
D3DDECL_END()
};

IDirect3DVertexDeclaration9* Fire::pVertexDecl = NULL;

const float Fire::directionStrengthMin = -4.0f;
const float Fire::directionStrengthMaxSubMin = 8.0f;

const float Fire::lifetimeMin = 0.3f;
const float Fire::lifetimeMaxSubMin = 0.7f;


Fire::Fire(const D3DXVECTOR3& position)
{
	Renderer::Get().m_pD3DDevice->CreateVertexBuffer(NUM_PARTICLES_PER_FIRE * sizeof(Particle), 
										D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pFireVB, NULL);
	
	Particle* pParticles;
	m_pFireVB->Lock(0, sizeof(Particle)*NUM_PARTICLES_PER_FIRE, (void**)&pParticles, D3DLOCK_DISCARD);
	for(int i=0; i<NUM_PARTICLES_PER_FIRE; ++i)
	{
		pParticles[i].startDirection.x = static_cast<float>(rand()) / RAND_MAX * directionStrengthMaxSubMin + directionStrengthMin;
		pParticles[i].startDirection.y = static_cast<float>(rand()) / RAND_MAX * directionStrengthMaxSubMin + directionStrengthMin;
		pParticles[i].startDirection.z = static_cast<float>(rand()) / RAND_MAX * directionStrengthMaxSubMin + directionStrengthMin;
		pParticles[i].starttime = static_cast<float>(rand()) / RAND_MAX;
		pParticles[i].lifetime = static_cast<float>(rand()) / RAND_MAX * lifetimeMaxSubMin + lifetimeMin;
	}
	m_pFireVB->Unlock();

	this->position = position;
}

Fire::~Fire(void)
{
	delete m_pFireVB;
}

void Fire::Draw(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos)
{
	Renderer::Get().m_pD3DDevice->SetStreamSource(1, m_pFireVB, 0, sizeof(Particle));
    Renderer::Get().m_pD3DDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1ul);
	Renderer::Get().m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
}