// Predeklarationen für die Verwaltung
class TreeInstance;
class StoneInstance;
class Stone;
struct TreeNode;

// Vertexstruct for the terrain - a pos only
struct PosOnlyVertex
{
	D3DXVECTOR3 Position;
	static const DWORD FVF;
	//static const D3DVERTEXELEMENT9		Declaration[];
};

struct StarVertex
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 HDRColor;	// passed as texcoord
	static const DWORD FVF;
};

class Bucket
{
private:
public:
	StoneInstance* m_pStones;
	TreeNode* m_pTreeInstances;

	bool m_bDirty;
	float m_x;
	float m_z;

	void Render(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos);
	void Simulate();
	void ClearStones();

	~Bucket();
};


// Weltverwaltung
class World
{
public:
	bool Initialize();

	// Singleton
	static World& Get();

	// Draws all terrainpatches - needs the shader to set the right positions - all other shader and texture-related commands have to be in the renderer!
	void DrawTerrain(const D3DXVECTOR3& _CameraPos, const D3DXVECTOR2& _CameraDir2D, const float _HorizontalFOV);
	//void DrawSky();

	float GetTerrainHeightAt(const float X, const float Z);

	static const unsigned short		m_TerrainGridSize;
	static const unsigned short		m_HalfTerrainGridSize;
	static const unsigned short		m_NumStones = 10;
	static const unsigned short		m_NumBuckets = 9;
	static const float				m_BucketSize;

	Bucket m_Buckets[m_NumBuckets][m_NumBuckets];

	// Simulieren des Brennens - Iteration über alle Buckets
	void Simulate();

private:
	World() {}
	~World();

	// gets index 
	unsigned short GetPosOnlyVertexIndex(const unsigned short x, const unsigned short y) const;

	// Terrain
	static const float				m_TerrainScale;
	static const unsigned short		m_NumTerrainVertices[2];
	static const unsigned int		m_NumTerrainPrimitives[2];
	//IDirect3DVertexDeclaration9*	m_pPosOnlyVertexDecl;
	IDirect3DVertexBuffer9*			m_pTerrainMesh_VB;
	IDirect3DIndexBuffer9*			m_pTerrainMesh_IB[2];	// two lods - 0 is more detailed

	// Sky
	//static const unsigned int		m_NumStars;
	//IDirect3DVertexBuffer9*			m_pSkys_VB;

	// Managmanet aller Objekte

	// Generierte Instanzen
	Stone* m_apStones[m_NumStones];

	// geladene Weltposition und Verschiebung im Array
	int m_BucketOffsetX;
	int m_BucketOffsetZ;

	// Testet, ob das ObjectQuad und das ViewQuad sich so überschneiden, dass
	// etwas gelöscht wird oder, dass etwas nachgeneriert werden muss.
	void Reload();

	void Fill(Bucket* _pBucket);
};