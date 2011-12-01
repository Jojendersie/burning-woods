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

const D3DXVECTOR4 Fire::fireStartColor = D3DXVECTOR4(1.2f, 1.0f, 0.91f, 1.0f)*0.2f;
const D3DXVECTOR4 Fire::fireMidColor = D3DXVECTOR4(1.2f, 0.6f, 0.22f, 1.0f)*0.4f;
const D3DXVECTOR4 Fire::fireEndColor = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);

const float Fire::fireStartScale = 0.8f;
const float Fire::fireEndScale = 0.4f;

const float Fire::directionStrengthMin = 0.3f;
const float Fire::directionStrengthMaxSubMin = 2.5f;

const float Fire::lifetimeMin = 0.7f;
const float Fire::lifetimeMaxSubMin = 3.0f;


Fire::Fire(const D3DXVECTOR3& position) : position(position)
{
	Renderer::Get().m_pD3DDevice->CreateVertexBuffer(NUM_PARTICLES_PER_FIRE * sizeof(Particle), 
										D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pFireVB, NULL);
	
	Particle* pParticles;
	m_pFireVB->Lock(0, sizeof(Particle)*NUM_PARTICLES_PER_FIRE, (void**)&pParticles, D3DLOCK_DISCARD);
	for(int i=0; i<NUM_PARTICLES_PER_FIRE; ++i)
	{
		float phi = static_cast<float>(rand()) / RAND_MAX * 2 * g_PI;
		float theta = static_cast<float>(rand()) / RAND_MAX * 2 * g_PI;
		float radius = static_cast<float>(rand()) / RAND_MAX * directionStrengthMaxSubMin + directionStrengthMin;
		pParticles[i].startDirection.x = cosf(theta) * sinf(phi);
		pParticles[i].startDirection.y = sinf(theta) * sinf(phi);
		pParticles[i].startDirection.z = cosf(phi);
		pParticles[i].startDirection *= radius;

		pParticles[i].starttime = static_cast<float>(rand()) / RAND_MAX;
		// less y startdir is less lifetime
		pParticles[i].lifetime = (static_cast<float>(rand()) / RAND_MAX * lifetimeMaxSubMin + lifetimeMin);
	}
	m_pFireVB->Unlock();

	// create light
	lightIndex = Renderer::Get().GetFreeLightPosition();
	Renderer::Get().m_LightList[lightIndex].Active = true;
	Renderer::Get().m_LightList[lightIndex].Position = position;
}

Fire::~Fire(void)
{
	delete m_pFireVB;
}

void Fire::Draw(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos)
{
	Renderer::Get().m_LightList[0].Color = D3DXVECTOR3(2.0f + fabs(sinf(g_PassedTime*4))*0.5, 1.0f + cosf(g_PassedTime*5)*0.1f, 0.02f);
	Renderer::Get().m_LightList[0].RangeSq = 8000.0f + sinf(g_PassedTime*5)*5;

	Renderer::Get().m_pD3DDevice->SetStreamSource(1, m_pFireVB, 0, sizeof(Particle));
    Renderer::Get().m_pD3DDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1ul);
	Renderer::Get().m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
}