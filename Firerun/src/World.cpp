#include "stdafx.h"
#include "Stone.h"
#include "World.h"
#include "Renderer.h"
#include "Tree.h"
#include "Player.h"

const DWORD PosOnlyVertex::FVF = D3DFVF_XYZ;
//const DWORD StarVertex::FVF = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0);


const float World::m_TerrainScale = 100.0f;
const unsigned short World::m_TerrainGridSize = 255;	// maximum possible with 16bit indexbuffering
const unsigned short World::m_HalfTerrainGridSize = 128;
const unsigned short World::m_NumTerrainVertices[] = { m_TerrainGridSize*m_TerrainGridSize, 
														m_HalfTerrainGridSize*m_HalfTerrainGridSize };
const unsigned int World::m_NumTerrainPrimitives[] = { (World::m_TerrainGridSize-1)*(World::m_TerrainGridSize-1)*2, 
													(World::m_HalfTerrainGridSize-1)*(World::m_HalfTerrainGridSize-1)*2 };

//const unsigned int World::m_NumStars = 20000;

const float World::m_BucketSize = 128.0f;

// **************************************************************************************************************************************************** //
World& World::Get()
{
	static World TheOneAndOnly;
	return TheOneAndOnly;
}

// **************************************************************************************************************************************************** //
bool World::Initialize()
{
	// -------------------------------------- Terrain --------------------------------------
//	Renderer::Get().m_pD3DDevice->CreateVertexDeclaration(PosOnlyVertex::Declaration, &m_pPosOnlyVertexDecl);
	Renderer::Get().m_pD3DDevice->CreateIndexBuffer(sizeof(WORD) * m_NumTerrainPrimitives[0] * 3, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pTerrainMesh_IB[0], NULL);
	Renderer::Get().m_pD3DDevice->CreateIndexBuffer(sizeof(WORD) * m_NumTerrainPrimitives[1] * 3, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pTerrainMesh_IB[1], NULL);
	Renderer::Get().m_pD3DDevice->CreateVertexBuffer(sizeof(PosOnlyVertex) * m_NumTerrainVertices[0], D3DUSAGE_WRITEONLY, PosOnlyVertex::FVF, D3DPOOL_DEFAULT, &m_pTerrainMesh_VB, NULL);

	PosOnlyVertex* pVertices;
	m_pTerrainMesh_VB->Lock(0, 0, (void**)(&pVertices), D3DLOCK_DISCARD);
	WORD* pIndices[2];
	m_pTerrainMesh_IB[0]->Lock(0, 0, (void**)(&pIndices[0]), D3DLOCK_DISCARD);
	m_pTerrainMesh_IB[1]->Lock(0, 0, (void**)(&pIndices[1]), D3DLOCK_DISCARD);

	unsigned int Index0 = 0;
	unsigned int Index1 = 0;
	float AngleCos = cosf(Renderer::m_FOVMax);
	int HalfGridSize = m_TerrainGridSize/2;
	for(int y=0; y<m_TerrainGridSize; ++y)
	{
		for(int x=0; x<m_TerrainGridSize; ++x)
		{
			WORD vert0 = GetPosOnlyVertexIndex(x,y);
			pVertices[vert0].Position.z = y-HalfGridSize; // 127500.0f / (255-y) - 500;	// perspective distribution - current view distance: 500 -> 255*500 = 127500
			pVertices[vert0].Position.x = x-HalfGridSize; //shiftedX;//powf(sinf((shiftedX / HalfGridSize) * g_PI_4), 8.0f) * shiftedX *	// x distribution - perspective sucks! sine is much better, but not perfect
											//pVertices[vert0].Position.z * AngleCos;		// the angle
			if(y%2==1 && (x==0 || x==m_TerrainGridSize-1))
				pVertices[vert0].Position.y = 0.2f;
			else if(x%2==1 && (y==0 || y==m_TerrainGridSize-1))
				pVertices[vert0].Position.y = 0.2f;
			else
				pVertices[vert0].Position.y = 0.0f;

			if(x > m_TerrainGridSize-2 || y > m_TerrainGridSize-2)
				continue;

			WORD vert1 = GetPosOnlyVertexIndex(x+1,y);
			WORD vert2 = GetPosOnlyVertexIndex(x,y+1);
			WORD vert3 = GetPosOnlyVertexIndex(x+1,y+1);

			pIndices[0][Index0++] = vert2;	
			pIndices[0][Index0++] = vert1;
			pIndices[0][Index0++] = vert0;

			pIndices[0][Index0++] = vert3;
			pIndices[0][Index0++] = vert1;
			pIndices[0][Index0++] = vert2;

			if(x%2 == 0 && y%2 == 0)
			{
				WORD vert1 = GetPosOnlyVertexIndex(x+2,y);
				WORD vert2 = GetPosOnlyVertexIndex(x,y+2);
				WORD vert3 = GetPosOnlyVertexIndex(x+2,y+2);

				pIndices[1][Index1++] = vert2;	
				pIndices[1][Index1++] = vert1;
				pIndices[1][Index1++] = vert0;

				pIndices[1][Index1++] = vert3;
				pIndices[1][Index1++] = vert1;
				pIndices[1][Index1++] = vert2;
			}
		}
	}


	m_pTerrainMesh_VB->Unlock();
	m_pTerrainMesh_IB[0]->Unlock();
	m_pTerrainMesh_IB[1]->Unlock();

	
	// -------------------------------------- Sky --------------------------------------
	/*Renderer::Get().m_pD3DDevice->CreateVertexBuffer(sizeof(StarVertex) * m_NumStars, D3DUSAGE_WRITEONLY, StarVertex::FVF, D3DPOOL_DEFAULT, &m_pSkys_VB, NULL);
	StarVertex* pSkyVertices;
	m_pSkys_VB->Lock(0, 0, (void**)(&pSkyVertices), D3DLOCK_DISCARD);
	for(int i=0; i<m_NumStars; ++i)
	{
		pSkyVertices[i].HDRColor = D3DXVECTOR3(static_cast<float>(rand()) / RAND_MAX * 2 + 0.2f,
												static_cast<float>(rand()) / RAND_MAX * 2 + 0.2f,
												static_cast<float>(rand()) / RAND_MAX * 2 + 0.2f);
		float theta = (static_cast<double>(rand()) / RAND_MAX)*g_PI - g_PI_4*2;
		float phi	= (static_cast<double>(rand()) / RAND_MAX)*g_PI*2;
		pSkyVertices[i].Position = D3DXVECTOR3(sinf(theta)*cos(phi), cos(theta), sin(theta)*sin(phi));
	}
	m_pSkys_VB->Unlock();*/

	// -------------------------------------- Objects --------------------------------------
	for(int i=0;i<m_NumStones;++i)
	{
		float fBaseSize = rand()*0.5f/32767.0f+0.4f;
		m_apStones[i] = new Stone(fBaseSize+rand()/32767.0f, fBaseSize+rand()/32767.0f, fBaseSize+rand()/32767.0f);
	}

	m_BucketOffsetX = 0;
	m_BucketOffsetZ = 0;
	for(int X=0;X<m_NumBuckets;++X)
		for(int Z=0;Z<m_NumBuckets;++Z)
		{
			// Orte initialisieren
			m_Buckets[X][Z].m_x = (X-m_NumBuckets/2.0f)*m_BucketSize;
			m_Buckets[X][Z].m_z = (Z-m_NumBuckets/2.0f)*m_BucketSize;
			// Generieren
			Fill(&m_Buckets[X][Z]);
		}


	return true;
}

unsigned short World::GetPosOnlyVertexIndex(const unsigned short x, const unsigned short y) const
{
	return y * m_TerrainGridSize + x;
}

// **************************************************************************************************************************************************** //
World::~World()
{
	m_pTerrainMesh_VB->Release();
	m_pTerrainMesh_IB[0]->Release();
	m_pTerrainMesh_IB[1]->Release();
	//m_pSkys_VB->Release();

	for(int i=0;i<m_NumStones;++i) delete m_apStones[i];
}

// **************************************************************************************************************************************************** //
void World::DrawTerrain(const D3DXVECTOR3& _CameraPos, const D3DXVECTOR2& _CameraDir2D, const float _HorizontalFOV)
{
	Renderer::Get().m_pD3DDevice->SetStreamSource(0, m_pTerrainMesh_VB, 0, sizeof(PosOnlyVertex));
	Renderer::Get().m_pD3DDevice->SetIndices(m_pTerrainMesh_IB[0]);
	Renderer::Get().m_pD3DDevice->SetFVF(PosOnlyVertex::FVF);

	// Texture
	Renderer::Get().m_pD3DDevice->SetTexture(D3DVERTEXTEXTURESAMPLER0, TextureManager::Get().m_aNoiseTexture[1].m_pTex);
	Renderer::Get().m_pD3DDevice->SetTexture(0, TextureManager::Get().m_aTerrainTextures[1].m_pTex);
	Renderer::Get().m_pD3DDevice->SetTexture(1, TextureManager::Get().m_aTerrainTextures[0].m_pTex);
	Renderer::Get().m_pD3DDevice->SetTexture(2, TextureManager::Get().m_aTerrainTextures[2].m_pTex);
	Renderer::Get().m_pD3DDevice->SetTexture(3, TextureManager::Get().m_aNoiseTexture[1].m_pTex);

	// mid
	D3DXVECTOR3 CameraPosFloor(floorf(_CameraPos.x), 0.0f, floorf(_CameraPos.z));
	Renderer::Get().m_pD3DDevice->SetVertexShaderConstantF(7, CameraPosFloor, 1);
	Renderer::Get().m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_NumTerrainVertices[0], 0, m_NumTerrainPrimitives[0]);
	
	// around
	Renderer::Get().m_pD3DDevice->SetIndices(m_pTerrainMesh_IB[1]);
	for(int x=-1;x<=1; ++x)
	{
		for(int z=-1;z<=1; ++z)
		{
			if(x==0 && z==0)
				continue;
			D3DXVECTOR3 TerrainPos(CameraPosFloor.x + x*World::m_TerrainGridSize -x, 0.0f, CameraPosFloor.z + z*World::m_TerrainGridSize -z); 
			D3DXVECTOR2 vec(TerrainPos.x - _CameraPos.x, TerrainPos.z - _CameraPos.z);
			D3DXVec2Normalize(&vec, &vec);
			if(D3DXVec2Dot(&vec, &_CameraDir2D) < 0.0f)
				continue;
			Renderer::Get().m_pD3DDevice->SetVertexShaderConstantF(7, TerrainPos, 1);
			Renderer::Get().m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_NumTerrainVertices[1], 0, m_NumTerrainPrimitives[1]);
		}
	}
}

// **************************************************************************************************************************************************** //
/*void World::DrawSky()
{
	Renderer::Get().m_pD3DDevice->SetStreamSource(0, m_pSkys_VB, 0, sizeof(StarVertex));
	Renderer::Get().m_pD3DDevice->SetFVF(StarVertex::FVF);
	Renderer::Get().m_pD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, m_NumStars);
}*/

float World::GetTerrainHeightAt(const float X, const float Z)
{
	// Zerrung beachten
	float fx = X*PERLIN_NOISE_TEXTURE_SIZE/1024.0f;	// OPT
	float fz = Z*PERLIN_NOISE_TEXTURE_SIZE/1024.0f;
	// Positives modulu
	int i = (int)(fx/1024.0f);
	fx -= i*1024.0f - (fx>=0.0f?0.0f:1024.0f);
	i = (int)(fz/1024.0f);
	fz -= i*1024.0f - (fz>=0.0f?0.0f:1024.0f);
	return TextureManager::Get().PerlinHeight(fx, fz, 0, PERLIN_NOISE_TEXTURE_OCTAVES, nullptr) * m_TerrainScale;
}

// **************************************************************************************************************************************************** //
void Bucket::ClearStones()
{
	// Delete stones. Will be used in different places.
	while(m_pStones)
	{
		StoneInstance* pTemp = m_pStones;
		m_pStones = m_pStones->m_pNextStone;
		delete pTemp;
	}
}

Bucket::~Bucket()
{
	ClearStones();
}

void Bucket::Simulate()
{
	// traverse list of all trees:
	// - if delete request than delete now
	// - if starting burn readd tree into list of burning trees
	TreeNode* pCurrent = m_pTreeInstances;
	TreeNode* pLast = nullptr;
	while(pCurrent)
	{
		if(pCurrent->pTree->m_Flags & DELETE_REQUEST)
		{
			TreeNode* pNext = pCurrent->pNext;
			if(pLast) pLast->pNext = pNext;
			else m_pTreeInstances = pNext;
			delete pCurrent->pNext;
			delete pCurrent;
			pCurrent = pNext;
		}

		pCurrent = pCurrent->pNext;
	}
}

void Bucket::Render(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos)
{
	// TODO: Culling
	// render all stones
	StoneInstance* pCurStone=m_pStones;
	while(pCurStone)
	{
		pCurStone->Render(View, ViewProjection, ViewPos);
		pCurStone = pCurStone->m_pNextStone;
	}

	// render all trees
	TreeNode* pCurTree=m_pTreeInstances;
	while(pCurTree)
	{
		pCurTree->pTree->Render();
		pCurTree = pCurTree->pNext;
	}
};

// **************************************************************************************************************************************************** //
void World::Simulate()
{
	Reload();
	/*for(int x=0;x<m_NumBuckets;++x)
		for(int z=0;z<m_NumBuckets;++z)
			m_Buckets[x][z].Simulate();*/
}

void World::Reload()
{
	int DeltaX = (int)(Player::Get().m_CameraPosition.x/m_BucketSize);
	int DeltaZ = (int)(Player::Get().m_CameraPosition.z/m_BucketSize);

	for(int X=0;X<m_NumBuckets;X++)
		for(int Z=0;Z<m_NumBuckets;Z++)
			if(m_Buckets[X][Z].m_bDirty)
			{
				assert(m_Buckets[X][Z].m_pStones==nullptr);
				assert(m_Buckets[X][Z].m_pTreeInstances==nullptr);
				// Neu generieren
				Fill(&m_Buckets[X][Z]);
			}

	// DeltaX-m_BucketOffsetX<0
	// -> unload right boundry, reload als left boundry
	// DeltaX-m_BucketOffsetX>0
	// -> unload left, reload right
	if(DeltaX!=m_BucketOffsetX)
	{
		int isgn = DeltaX-m_BucketOffsetX;
		isgn = isgn/abs(isgn);
		// left or right columne
		int ColX = ((m_BucketOffsetX+min(0,isgn)) % m_NumBuckets + m_NumBuckets) % m_NumBuckets;
		// Every bucket have to be cleared first.
		// Refilling with code on top in the next frame.
		for(int Z=0;Z<m_NumBuckets;Z++)
		{
			// renew Position
			m_Buckets[ColX][Z].m_x += isgn*m_NumBuckets*m_BucketSize;//= (DeltaX-m_NumBuckets/2.0f)*m_BucketSize;
			m_Buckets[ColX][Z].ClearStones();
			m_Buckets[ColX][Z].m_bDirty = true;	// OPT: Dirty nach ClearStones verschieben?
			// Set delete request for trees. They will be unloaded
			// automaticly after doing this. Delete request is also
			// automated by setting m_bDirty. The simulation sets the
			// flags when seeing dirty buckets.
		}
		m_BucketOffsetX+=isgn;
	}

	// same as for X-Direction
	// DeltaZ-m_BucketOffsetZ<0
	// DeltaZ-m_BucketOffsetZ>0
	if(DeltaZ!=m_BucketOffsetZ)
	{
		int isgn = DeltaZ-m_BucketOffsetZ;
		isgn = isgn/abs(isgn);
		int RowZ = ((m_BucketOffsetZ+min(0,isgn)) % m_NumBuckets + m_NumBuckets) % m_NumBuckets;
		for(int X=0;X<m_NumBuckets;X++)
		{
			m_Buckets[X][RowZ].m_z += isgn*m_NumBuckets*m_BucketSize;
			m_Buckets[X][RowZ].ClearStones();
			m_Buckets[X][RowZ].m_bDirty = true;	// OPT: Dirty nach ClearStones verschieben?
		}
		m_BucketOffsetZ+=isgn;
	}
}

void World::Fill(Bucket* _pBucket)
{
	// Random number of stone instanzes
	// The number increases with distance, to create difficulty
	int iNumStones = (int)(sqrt(_pBucket->m_x*_pBucket->m_x+_pBucket->m_z*_pBucket->m_z)*0.01f)+1;
	iNumStones = rand()%iNumStones+iNumStones;
	for(int i=0;i<iNumStones;++i)
	{
		// Randompos im Bucket
		float fx = _pBucket->m_x+rand()*m_BucketSize/32767.0f-m_BucketSize*0.5f/32767.0f;
		float fz = _pBucket->m_z+rand()*m_BucketSize/32767.0f-m_BucketSize*0.5f/32767.0f;
		StoneInstance* pNew = new StoneInstance(fx,
			GetTerrainHeightAt(fx,fz),
			fz,
			m_apStones[rand()%m_NumStones]);
		pNew->m_pNextStone = _pBucket->m_pStones;
		_pBucket->m_pStones = pNew;
	}

	_pBucket->m_bDirty = false;
}