// **************************************************************************************************************************************************** //
// Vertexstruct for Stones etc.
struct StdVertex
{
	D3DXVECTOR3 pos;
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

	float m_RadiusSq;

	void Render();
};

// **************************************************************************************************************************************************** //
class StoneInstance
{
	D3DXMATRIX m_Transform;
	D3DXMATRIX m_TransformInv;
public:
	Stone* m_pStone;
	D3DXVECTOR3 m_Position;
	StoneInstance* m_pNextStone;

	

	StoneInstance(const float _fx,
		const float _fy,
		const float _fz,
		Stone* _pStone);

	void Render(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos);
};