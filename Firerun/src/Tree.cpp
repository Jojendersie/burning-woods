#include "stdafx.h"
#include "Tree.h"
#include "Renderer.h"

// **************************************************************************************************************************************************** //
TreeInstance::TreeInstance(const float _fx,
		const float _fy,
		const float _fz,
		Tree* _pTree)
{
	// Statische Transformation der Instanz mit zufälliger Rotation
	D3DXMatrixRotationY(&m_Transform, rand()*0.1f);
	//D3DXMatrixScaling(&m_Transform, 100.0f, 100.0f, 100.0f);
	m_vPosition.x = m_Transform._41 = _fx;
	m_vPosition.y = m_Transform._42 = _fy;
	m_vPosition.z = m_Transform._43 = _fz;

	m_Flags = 0;
	memset(m_apAffectedNeighbours, 0, sizeof(m_apAffectedNeighbours));
	m_pTree = _pTree;
	m_iRefCounter = 0;

	D3DXMatrixInverse(&m_TransformInv, nullptr, &m_Transform);
}

void TreeInstance::Render(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos)
{
	// Transformation setzen und Geometrie zeichnen
	Renderer::Get().m_pD3DDevice->SetVertexShaderConstantF(0, m_Transform * ViewProjection, 4);
	Renderer::Get().m_pD3DDevice->SetVertexShaderConstantF(4, m_Transform * View, 4);
	// Objectspace Kameraposition - Entfernungsberechnung im defferred renderer
	D3DXVECTOR3 temp;
	Renderer::Get().m_pD3DDevice->SetVertexShaderConstantF(7, *D3DXVec3TransformCoord(&temp, &ViewPos, &m_TransformInv), 1);

	// Entfernung zur Kamera berechnen und als Detaillevel einsetzen
	float fDist = std::max(1.0f, D3DXVec3Length( &(ViewPos - m_vPosition) ));
	int iMax = m_pTree->GetNumDetailLevels();
	m_pTree->Render(std::min(iMax,int(iMax*100.0f/fDist)));
}

// **************************************************************************************************************************************************** //
// Standard Construktor
Tree::Tree(): m_pDetailInfos(nullptr),
			  m_pSegmentMemory(nullptr),
			  m_iNumUsedSegments(0),
			  m_TreeStatus(TFT_CREATED)
{
	memset(&m_Skeleton, 0, sizeof(TreeBone*)*5);
}

// **************************************************************************************************************************************************** //
// Standard Destruktor
Tree::~Tree()
{
	// Free all recources belongig to the tree
	delete[] m_pDetailInfos;
	free(m_pSegmentMemory);
//	delete m_pSkeleton;
}

// **************************************************************************************************************************************************** //
// Allocates one element from m_pSegmentMemory, sets it to zero; returns 0 if memory full
// Parameters:
//	_pParent - The node to which the new one is added. This could be 0
//	_iIndex - Index between 0 and 7 for child position
TreeBone* Tree::AllocBranchSegment(TreeBone* _pParent)
{
	if(m_iNumUsedSegments<m_GenCode.GetMaxBranchSegmentNumber())
	{
		// The parent node has to know its child
		if(_pParent && _pParent->m_iNumChildren<8)
		{
			_pParent->m_pChildren[_pParent->m_iNumChildren++] = &m_pSegmentMemory[m_iNumUsedSegments];
		}
		// Set only the required minimum
		m_pSegmentMemory[m_iNumUsedSegments].m_iNumChildren = 0;
		m_pSegmentMemory[m_iNumUsedSegments].m_pParent = _pParent;
		memset(m_pSegmentMemory[m_iNumUsedSegments].m_pChildren, 0, sizeof(TreeBone*)*8);
		return &m_pSegmentMemory[m_iNumUsedSegments++];
	}
	return nullptr;
}

// **************************************************************************************************************************************************** //
// Load from (virtual) file
void Tree::Load(const TreeGen* _pGen)
{
	// Unload old stuff (the constructor sets a marker, so we can distinguish)
	delete[] m_pDetailInfos; m_pDetailInfos = nullptr;

	// Load the gen code. This is the only essential information.
	memcpy(&m_GenCode, _pGen, sizeof(TreeGen));

	// Generate Skeleton and Triangulate
	// Do not calculate metrics if not nessecary! (Performance)
	Generate();

	TriangulateCylindrical();
}

// **************************************************************************************************************************************************** //
// Slow method searches for all branches with level after level
void _TraverseLevelOrder(const RecTraverseParam& _pConstParams, TreeBone* _pBone)
{
	if(_pBone->m_iGeneration <= _pConstParams.iCurLevel)
	{
		// Do something with the current bone, if it is part of the current level
		if(_pBone->m_iGeneration == _pConstParams.iCurLevel)
			_pConstParams.pFunction(_pBone, _pConstParams.pParam);

		// Search for more Bones with the level
		for(int i=0; i<_pBone->m_iNumChildren; ++i)
		{
			// Call if smaller or equal. Each branch with smaller gerneration could have
			// children with the correct generation.
			if(_pBone->m_pChildren[i]->m_iGeneration <= _pConstParams.iCurLevel)
				_TraverseLevelOrder(_pConstParams, _pBone->m_pChildren[i]);
		}
	}
}

void Tree::TraverseLevelOrder(TfTreeTraverseFunction _pFunction, const void* _pParam)
{
	// For each level
	for(int i=0; i<m_Metrics.iNumGenerations; ++i)
	{
		RecTraverseParam Params;
		Params.iCurLevel = i;
		Params.pFunction = _pFunction;
		Params.pParam = _pParam;
		// Recursiv call with respect to the current level
		for(int j=0; j<m_Skeleton.iNumChildren; ++j)
			_TraverseLevelOrder(Params, m_Skeleton.pChildren[j]);
	}
}


// **************************************************************************************************************************************************** //
void _TraversePreOrder(const RecTraverseParam& _pConstParams, TreeBone* _pBone)
{
	// Do something with the current bone before recursion
	_pConstParams.pFunction(_pBone, _pConstParams.pParam);

	// For each children
	for(int i=0; i<_pBone->m_iNumChildren; ++i)
		_TraversePreOrder(_pConstParams, _pBone->m_pChildren[i]);
}

// Faster traversion throug the tree. Calls the function for a bone and does
// the recursion afterward.
void Tree::TraversePreOrder(TfTreeTraverseFunction _pFunction, const void* _pParam)
{
	// For each children.
	for(int i=0; i<m_Skeleton.iNumChildren; ++i)
	{
		RecTraverseParam Params;		// Ignore the iCurLevel
		Params.pFunction = _pFunction;
		Params.pParam = _pParam;
		_TraversePreOrder(Params, m_Skeleton.pChildren[i]);
	}
}


// **************************************************************************************************************************************************** //
void _TraversePostOrder(const RecTraverseParam& _pConstParams, TreeBone* _pBone)
{
	// For each children
	for(int i=0; i<_pBone->m_iNumChildren; ++i)
		_TraversePostOrder(_pConstParams, _pBone->m_pChildren[i]);

	// Do something with the current bone after recursion
	_pConstParams.pFunction(_pBone, _pConstParams.pParam);
}

// Faster traversion throug the tree. Calls recursive first and the function for
// the bone after returning from recursion.
void Tree::TraversePostOrder(TfTreeTraverseFunction _pFunction, const void* _pParam)
{
	// For each children.
	for(int i=0; i<m_Skeleton.iNumChildren; ++i)
	{
		RecTraverseParam Params;		// Ignore the iCurLevel
		Params.pFunction = _pFunction;
		Params.pParam = _pParam;
		_TraversePostOrder(Params, m_Skeleton.pChildren[i]);
	}
}

// **************************************************************************************************************************************************** //
// Calculates which LOD consists branches of the given generation
int Tree::GetLODOfGeneration(unsigned int _uiGeneration)
{
	for(int i=0;i<GetNumDetailLevels();++i)
		if(m_pDetailInfos[i].iMaxGeneration >= _uiGeneration) return i;
	return -1;
}

// **************************************************************************************************************************************************** //
// Remove all generated Data (to regenerate a tree from scratch)
/*#ifndef O_64K
void Tree::Clear()
{
	m_TreeStatus = TFT_UNSTABLE;
	delete[] m_pDetailInfos; m_pDetailInfos = nullptr;
	m_Skeleton.~TreeRootNode();
	m_Skeleton.iNumChildren = 0;
	memset(&m_Skeleton, 0, sizeof(TreeBone*)*5);
	m_iNumUsedSegments = 0;

	// Remove branch memory (size can change)
	if(m_pSegmentMemory)	{free(m_pSegmentMemory); m_pSegmentMemory = nullptr;}
}
#endif*/

// **************************************************************************** //
// Calculates the up vector with influences from the light
D3DXVECTOR3 Tree::ModifyVector(const D3DXVECTOR3& _V, const TreeBone* _pBone) const
{
	// Using the up direction and the light dir to determine the initial direction.
	// Goal: Flat branches with a large surface in light direction.
	D3DXVECTOR3 vUp = _V;
	vUp.y /= (_pBone->m_iGeneration*m_GenCode.GetBranchFlattening()+1);
	D3DXVec3Normalize(&vUp, &vUp);
	return vUp;
}

// **************************************************************************** //
void Tree::Render(int _iLOD)
{
	// Branches -------------------------------------- //
	if(_iLOD>0)
	{
		// set correct bark texture (TODO switch for tree instances)
		Renderer::Get().m_pD3DDevice->SetTexture(0, TextureManager::Get().m_aBarkTextures[m_GenCode.Wood.iTextureIndex].m_pTex);

		Renderer::Get().m_pD3DDevice->SetIndices(m_pIndices);
		Renderer::Get().m_pD3DDevice->SetFVF(StdVertex::FVF);		// TODO remove if prior call uses StdVertex too?
		Renderer::Get().m_pD3DDevice->SetStreamSource(0, m_pVertices, 0, sizeof(StdVertex));

		// Drawcall
		unsigned int uiNumTr = GetLODPrimitiveCount( _iLOD-1 );
		Renderer::Get().m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_uiNumVertices, 0, uiNumTr);
	}

	// Leaves ---------------------------------------- //
	/*{
		// Set buffers for rendering
		Renderer::Get().m_pD3DDevice->SetIndices(g_pLeafIndices);
		Renderer::Get().m_pD3DDevice->SetStreamSource(0, m_pLeafVertices, 0, sizeof(StdVertex));
		//m_pEffect->SetTexture( "g_MeshTexture", m_pLeafTexture );

		// Drawcall
		float fDist = 1.0f;	// TODO dist or iLOD based
		unsigned int uiNumTr = unsigned int(GetMetrics().uiNumLeaves*fDist)*2;
		Renderer::Get().m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, uiNumTr*2, 0, uiNumTr);
	}*/
}


// **************************************************************************** //
// *********** TreeBone ******************************************************* //
// **************************************************************************** //
TreeBone::TreeBone(TreeBone* _pParent)
{
	// Set only the required minimum
	m_iNumChildren = 0;
	memset(m_pChildren, 0, sizeof(TreeBone*)*8);
	m_pParent = _pParent;
}

// **************************************************************************** //
// Recursive destructor for the skeleton
TreeBone::~TreeBone()
{
	// DO NOT DESTRUCT ANYTHING this is all done by tree internal memory managment
}

// **************************************************************************** //
// Counts all branch segment of thos branch in root direction.
int TreeBone::GetBranchDistanceToFork()
{
	TreeBone* pBone = this;
	int iCount = 0;
	while(!pBone->IsFirstOfNewGeneration())
	{
		++iCount;
		pBone = pBone->m_pParent;
	}
	return iCount;
}

// **************************************************************************** //
// Counts all branch segment to the tip of this branch.
int TreeBone::GetBranchDistanceToApex()
{
	TreeBone* pBone = this;
	int iCount = 0;
	while(pBone->HasChildren())
	{
		++iCount;
		pBone = pBone->m_pChildren[0];
	}
	return iCount;
}

// **************************************************************************** //
// Determine how many Leaves this branch has
int TreeBone::GetNumLeaves()
{
	// Calculate something like the propability
	float fLeafP = std::min(1.0f,0.05f/m_fDiameter);	// TODO light
	return (int)(fLeafP*(3-m_iNumChildren));
}

// **************************************************************************** //