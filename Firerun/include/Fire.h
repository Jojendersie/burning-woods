#pragma once

#define NUM_PARTICLES_PER_FIRE 2000

// Fireparticle system using instancing
// using 2 vbs: 
// - single quad
// - (FireVB) one vertex for every particle
// both are static!
class Fire
{
public:
	Fire(const D3DXVECTOR3& position);
	~Fire();

	void Draw(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos);
	
	static const D3DVERTEXELEMENT9 FireVertexDeclElements[];
	static IDirect3DVertexDeclaration9* pVertexDecl;	// created and destoryed by renderer

	static const D3DXVECTOR4 fireStartColor;
	static const D3DXVECTOR4 fireMidColor;
	static const D3DXVECTOR4 fireEndColor;

	static const float fireStartScale;
	static const float fireEndScale;

private:
	struct Particle
	{
		D3DXVECTOR3 startDirection;
		float	lifetime;	// time of spawn in s
		float	starttime;	// 0-1, a float value that gives a form of "liveoffset"
	};

	D3DXVECTOR3 position;
	int lightIndex;

	static const float directionStrengthMin;	// its per axis - using spherecordinates would be better
	static const float directionStrengthMaxSubMin;

	static const float lifetimeMin;
	static const float lifetimeMaxSubMin;

	IDirect3DVertexBuffer9* m_pFireVB;
};

