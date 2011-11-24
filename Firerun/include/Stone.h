// **************************************************************************************************************************************************** //
// Vertexstruct for Stones etc.
struct StdVertex
{
	float x,y,z;
	float nx,ny,nz;
	float u,v;
	static const DWORD FVF;
};

// **************************************************************************************************************************************************** //
// Simple a Stone-Mesh
class Stone
{
private:
	PDIRECT3DVERTEXBUFFER9	m_pVertexBuffer;
	PDIRECT3DINDEXBUFFER9	m_pIndexBuffer;

public:
	Stone(const float _fa,
		const float _fb,
		const float _fc);
	~Stone();

	void Render();
};

// **************************************************************************************************************************************************** //
class StoneInstance
{
private:
	Stone* m_pStone;
	D3DXMATRIX m_Transform;
	D3DXMATRIX m_TransformInv;
public:
	StoneInstance* m_pNextStone;
	float m_fRadius;	// radius of collision

	

	StoneInstance(const float _fx,
		const float _fy,
		const float _fz,
		Stone* _pStone);

	void Render(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos);
};