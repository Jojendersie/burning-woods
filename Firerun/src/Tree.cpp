#include "stdafx.h"
#include "Tree.h"

// **************************************************************************************************************************************************** //
TreeInstance::TreeInstance(const float _fx,
		const float _fy,
		const float _fz,
		Tree* _pTree)
{
	// Statische Transformation der Instanz mit zufälliger Rotation
	D3DXMatrixRotationY(&m_Transform, rand());
	m_Transform._41 = _fx;
	m_Transform._42 = _fy;
	m_Transform._43 = _fz;

	m_Flags = (TreeStatus)0;
	m_pAffectedNeighbours = nullptr;
	m_pTree = _pTree;
}

void TreeInstance::Render()
{
	// Transformation setzen und Geometrie zeichnen
}