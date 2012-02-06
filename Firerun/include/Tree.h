// **************************************************************************** //
// TfData																		//
//																				//
// Created: Johannes Jendersie		02.09.2011									//
//																				//
// Definition of all structured datas to safe trees.							//
//																				//
//	TreeBone -		Part of the abstract skeleton of a tree. Only required		//
//					during generation.											//
//	TreeMetrics -	Statistical informations after generation.					//
//	TreeRootNode -	Base of the tree skeleton.									//
//	TreeGen -		Input information: "Gen Code" of the tree-species			//
// **************************************************************************** //

#pragma once

#include "stdafx.h"
#include "Stone.h"

// **************************************************************************************************************************************************** //
// Every bone is an abstract part of the tree skeleton.
// The struct is recursiv. One Bone can also be a whole tree.
class TreeBone
{
public:
	D3DXVECTOR3 m_vPosition;				// Start of the segment (if tree without any deformation)
	D3DXVECTOR3 m_vDirection;				// Main direction vector
	D3DXVECTOR3 m_vDestDirection;			// Main direction vector without gravitation

	D3DXVECTOR3 m_vUp;						// Up direction vector

	float m_fLength;						// Length of the branch segment
	float m_fDiameter;						// Diameter of the branch
	float m_fAge;							// Time stamp when the branch segment was created

	int m_iNumChildren;						// Number of bone pointers to the children
	int m_iGeneration;						// Level/Generation of the branch

	int m_iParam;							// Extra Data (used in triangulation)
	
	TreeBone*	m_pParent;					// Parent Bone for lookup
	TreeBone*	m_pChildren[8];				// Variable number of children bones, where every distribution in the array is thinkable (apex (set or everthing is 0),0,0,pointer,0,pointer,0,0)

	TreeBone(TreeBone* _pParent);
	~TreeBone();							// Recursive destructor for the skeleton

	bool HasChildren()						{return m_iNumChildren > 0;}					// Does the current segment has children?
	bool IsFirstOfNewGeneration()			{return !m_pParent || m_pParent->m_iGeneration != m_iGeneration;}
	bool IsDecendantOf(TreeBone* _pBone)	{TreeBone* pD = this; while(pD) if(_pBone==pD) return true; else pD = pD->m_pParent; return false;}
	D3DXVECTOR3 GetUpVector()				{return m_vUp;}
	D3DXVECTOR3 GetTipPosition()			{return m_vPosition + m_vDirection*m_fLength;}

	int GetBranchDistanceToFork();												// Counts all branch segment of this branch in root direction.
	int GetBranchDistanceToApex();												// Counts all branch segment to the tip of this branch.
	int GetNumLeaves();
	int GetVertexRingSize(float _fDetail);										// TREEFOLK_TRI_LOD uses cylinder per branch
	void GetNumPrimitives(int& _iIndices, int& _iVertices);						// Checks which geometry (cylinder/tip/root) is used and calculates the number of vertices and indices necessary for this branch
};

// **************************************************************************************************************************************************** //
// Statistical informations after generation.
struct TreeMetrics
{
	unsigned int uiNumBranches;			// Global number of branches
	unsigned int uiNumBranchSegments;	// Global number of bones
	unsigned int uiDeadBranchSegments;	// Number of dead branches (subset from all branch segments). It is not possible to count dead branches, because a branch can be half dead.
	unsigned int uiNumLeaves;			// Number of Leaves calculated during generation	int iNumGenerations;				// Recursive depth of the branching
	int iNumGenerations;				// Recursive depth of the branching
	int iNumTriangulationLevels;		// Number of entries in the m_puiNumIndices array. Not set correct every time.
	float fHeight;						// Height of the tree
	float fDiameter;					// Diameter of the crown
	D3DXVECTOR2 vMassBalancing;			// Space orientation of the whole tree
};

// **************************************************************************************************************************************************** //
// Base of the tree skeleton.
// There can begin multiple truncs and roots in this node.
struct TreeRootNode
{
	int iNumChildren;					// Number of bone pointers to the children
	TreeBone* pChildren[5];				// Variable number of children bones

	TreeRootNode():iNumChildren(0)	{memset(pChildren, 0, sizeof(TreeBone*)*5);}
};


// **************************************************************************************************************************************************** //
// Input information: "Gen Code" of the tree-species
struct TreeGen
{
	struct {
		float fShotPropability;				// Basic probability for a new shot at possible shot-point (between 0.0 and 1.0)
		float fShotDamping;					// Level depending reduction of shot probability (1.0 ist no damping, <1.0 is recommented)
		int iApexShotNumber;				// Number of branch segments per shot at the apex (at least 1)
		float fShotLength;					// Length of one branch segment in the shot
		int iNumChildShots;					// Number of possible children at the tip of each bone/Number of Leaves at the tip (leaf trees have usual 1 or 2, coniferous species can have more.)
		float fRandom;						// Influenzes angles and shot lengthes (0.0 is minimum)
		int iMaxBoneNumber;					// Number of branch segments in the whole tree, used for memory allocation
	} Branching;

	struct {
		float fApexRotation;				// Between each bone the up-vector can be rotated beyond the bone.
		float fApexShotAngle;				// Between each bone there is an angle away from the up-vector
		float fShotAngle;					// Angle between parent and child of different generation. 0° stand for an 90° away from the branch.
		float fBranchFlattening;			// The branch is flatten ignoring the other angles
	} Angles;

	struct {
		float fGrowthRate;					// How much does every bone growth per year (1.0 is no growth, 0.0 is minimum)
		float fHardness;					// Hardness of the wood (stabilizing the angles, 0.0 is minimum)
		float fThicknessProportion;			// The thickness is calculated as Th = fThicknessProportion*sqrt(child1.Th^2 + ... + childn.Th^2)
		float fLeafSize;					// Scale value for one leaf quad
		int iTextureIndex;					// Index of the generated textures in m_aBarkTextures
	} Wood;

	struct {
		float fAge;							// Age of the tree 1.0 is equal to one year
		unsigned int uiRndSeed;				// Randseed of this tree instance (deterministic)
		unsigned int uiTextureMultiplicator;// Tiling (TODO float?)
	} Individual;

/*	void SetAge(float _fAge)									{Individual.fAge = std::min(500.0f, std::max(0.1f, _fAge));}
	void SetApexRotation(float _fApexRotation)					{Angles.fApexRotation = min(g_2_PI, max(0.0f, _fApexRotation));}
	void SetApexShotAngle(float _fApexShotAngle)				{Angles.fApexShotAngle = min(1.5f, max(0.0f, _fApexShotAngle));}
	void SetApexShotNumber(int _iApexShotNumber)				{Branching.iApexShotNumber = min(15, max(1, _iApexShotNumber));}
	void SetGrowthRate(float _fGrowthRate)						{Wood.fGrowthRate = min(10.0f, max(0.05f,  _fGrowthRate));}
	void SetHardness(float _fHardness)							{Wood.fHardness = min(10.0f, max(0.0f, _fHardness));}
	void SetLeafSize(float _fLeafSize)							{Wood.fLeafSize = min(10.0f, max(0.01f, _fLeafSize));}
	void SetNumChildShots(int _iNumChildShots)					{Branching.iNumChildShots = min(8, max(1, _iNumChildShots));}
	void SetRandom(float _fRandom)								{Branching.fRandom = min(1.0f, max(0.0f, _fRandom));}
	void SetRandSeed(int _iSeed)								{Individual.uiRndSeed = (unsigned int)_iSeed;}
	void SetShotAngle(float _fShotAngle)						{Angles.fShotAngle = min(1.5f, max(-1.5f, _fShotAngle));}
	void SetShotDamping(float _fShotDamping)					{Branching.fShotDamping = min(2.0f, max(0.0f, _fShotDamping));}
	void SetShotLength(float _fShotLength)						{Branching.fShotLength = min(3.0f, max(0.05f, _fShotLength));}
	void SetMaxBranchSegmentNumber(int _iMax)					{Branching.iMaxBoneNumber = min(500000, max(100, _iMax));}
	void SetShotPropability(float _fShotPropability)			{Branching.fShotPropability = min(1.0f, max(0.0f, _fShotPropability));}
	void SetThicknessProportion(float _fThicknessProportion)	{Wood.fThicknessProportion = min(5.0f, max(0.5f, _fThicknessProportion));}
	void SetBranchFlattening(float _fBranchFlattening)			{Angles.fBranchFlattening = _fBranchFlattening;}
*/
	float GetAge() const										{return Individual.fAge;}
	float GetApexRotation() const								{return Angles.fApexRotation;}
	float GetApexShotAngle() const								{return Angles.fApexShotAngle;}
	int GetApexShotNumber() const								{return Branching.iApexShotNumber;}
	float GetGrowthRate() const 								{return Wood.fGrowthRate;}
	float GetHardness() const									{return Wood.fHardness;}
	float GetLeafSize() const									{return Wood.fLeafSize;}
	int GetNumChildShots() const								{return Branching.iNumChildShots;}
	float GetRandom() const										{return Branching.fRandom;}
	int GetRandSeed() const										{return (int)Individual.uiRndSeed;}
	float GetShotAngle() const 									{return Angles.fShotAngle;}
	float GetShotDamping() const								{return Branching.fShotDamping;}
	float GetShotLength() const									{return Branching.fShotLength;}
	float GetShotPropability() const							{return Branching.fShotPropability;}
	float GetThicknessProportion() const						{return Wood.fThicknessProportion;}
	int GetMaxBranchSegmentNumber() const						{return Branching.iMaxBoneNumber;}
	float GetBranchFlattening() const							{return Angles.fBranchFlattening;}
};

// **************************************************************************************************************************************************** //
// Interface informations to create Leaves into an arbitary context.
struct TfLeavesetup
{
//	TfFunctionAddLeafVertex pFAddVertex;	// Stream output-callback for vertexbuffers.
//	TfFunctionAddIndex pFAddIndex;		// Stream output-callback for indexbuffers.
	// Indexbuffers are not generated scince it is always just a simple quad.
	// The user can determine the 6 indices by himself.
	// (0 1 2; 0 2 3), (4 5 6; 4 6 7), ... (4*n+0 4*n+1 4*n+2; 4*n+0 4*n+2 4*n+3)

	int iNumMaxPrimordia;				// Reserved: Input value for gen depending informations
	float fLeafSize;					// Reserved (Copy from gen-struct)
	float fGravity;						// Reserved (Copy from gen-struct 1/hardness)
};

// **************************************************************************************************************************************************** //
// **************************************************************************************************************************************************** //
// **************************************************************************************************************************************************** //
typedef void (*TfTreeTraverseFunction)(TreeBone*, const void*);

// Structs for recursive tree editing. This relieves the use of the call stack
struct RecTraverseParam
{
	TfTreeTraverseFunction pFunction;	// User defined function to call for each bone
	int iCurLevel;						// Fixed searchlevel
	const void* pParam;					// User defined data
};

// LOD-Informations for an individual tree
struct TfLODInfo {
	unsigned int uiDetailIndex;			// Render from 0 to this index to get a LOD for the triangulation
	unsigned int uiNumBranches;			// Number of branches in each LOD - not necessary of the same generation
	unsigned int iMaxGeneration;		// Each LOD consists of one or more generations (one generation is never splitted)
	TfLODInfo():uiDetailIndex(0),uiNumBranches(0)	{}
};

// **************************************************************************************************************************************************** //
class Tree
{
private:
	enum {TFT_CREATED=1, TFT_GENERATED=2, TFT_UNSTABLE=3} m_TreeStatus;
	TreeBone*	m_pSegmentMemory;		// Private memory block for all branchsegments (internal managment per tree)
	int			m_iNumUsedSegments;		// How many of the given Segements are realy occupied

	TreeGen		m_GenCode;				// Copy of the initial gen code
	TreeMetrics	m_Metrics;				// The informations for the Tree
	TreeRootNode m_Skeleton;			// Abstract skeleton

	TfLODInfo*	m_pDetailInfos;			// m_Metrics.iNumGenerations indices, indicating the last vertex of the corresponding level

	// Managment
	TreeBone* AllocBranchSegment(TreeBone* _pParent);		// Allocates one element from m_pSegmentMemory, sets it to zero; returns 0 if memory full

	LPDIRECT3DVERTEXBUFFER9	m_pVertices;
	LPDIRECT3DINDEXBUFFER9	m_pIndices;
	LPDIRECT3DVERTEXBUFFER9	m_pLeafVertices;
	StdVertex*		m_pLockedVertexBuffer;	// Defined during triangulation prozess
	unsigned int*	m_pdwLockedIndices;		// Defined during triangulation prozess
	unsigned int	m_uiNumVertices;
	unsigned int	m_uiNumIndices;
	unsigned int	m_uiNumLeafVertices;
	void CalculateVertex(TreeBone* _pBone, float _fRatio, float _fTex, int _iTexMul, int _bInitial, const D3DXVECTOR3& _vApex);		// Creates the i-th (Ratio to full ring) vertex of the ring defined by the given bone.
	void AddLeaves(TreeBone* _pBone, TfLeavesetup* _pParam);
	void AddBottomCenteredLeaf(TreeBone* _pBone, float _fOrder, const TfLeavesetup* _pParam);
	void AddLeafVertex(const float* _afPos, const float* _afNormal, const float* _afTexCoord, const float _fLight);
	void InitMeshBuffes();

	// Generator sub routines
	TreeBone* GenerateSkelton(TreeBone* _pParent, const D3DXVECTOR3& _vDir, float _fRootDist, float _fYear);
	TreeBone* GrowthApex(TreeBone* _pBone, D3DXVECTOR3 vDir, int _iMinRemaining);
	void CalcShotDirection(TreeBone* _pBone, D3DXVECTOR3& _vDir);

	void GenerateSkelton();				// Generate abstrakt skeleton of a tree.
	void CreateRoot();					// Create one new shot from the ground
	D3DXVECTOR3 ModifyVector(const D3DXVECTOR3& _V, const TreeBone* _pBone) const;		// Calculates the up vector with influences from the light

	//TreeBone* AddRootNode();			// Adds a new start bone to the tree skeleton
	friend void GrowthPrimordia(TreeBone* _pBone, const void* _pParam);
	friend void GrowthApex(TreeBone* _pBone, Tree* _pTree, int _iMinRemaining);
	friend void AddShot(TreeBone* _pBone, int _iOrder, Tree* _pTree);
	friend void CalcLength(TreeBone* _pBone, const void* _pTree);
	friend bool ShotPropabilityTest(TreeBone* _pBone, const Tree* _pTree);
	friend void CalculateMetrics(TreeBone* _pBone, const void* _pParam);
	friend void TraverseBuildTriangles(TreeBone* _pBone, const void* _pParam);
	//friend void TraverseCalculateNumberOfVertices(TreeBone* _pBone, const void* _pParam);

	static void _cdecl TraverseCalculateNumberOfVertices(TreeBone* _pBone, const void* _pParam);
public:
	// Standard Destruktor and Construktor
	Tree();
	~Tree();

	void Load(const TreeGen* _pGen);

	void Render(int _iLOD);

	const TreeRootNode& GetSkeleton() const		{return m_Skeleton;}
	const TreeMetrics& GetMetrics() const		{return m_Metrics;}

	// Slow method searches for all branches with level after level
	void TraverseLevelOrder(TfTreeTraverseFunction _pFunction, const void* _pParam);

	// Faster traversion throug the tree. Calls recursive first and the function for
	// the bone after returning from recursion.
	void TraversePostOrder(TfTreeTraverseFunction _pFunction, const void* _pParam);

	// Faster traversion throug the tree. Calls the function for a bone and does
	// the recursion afterward.
	void TraversePreOrder(TfTreeTraverseFunction _pFunction, const void* _pParam);

	const TreeGen* GetGen() const	{return &m_GenCode;}
	TreeGen* GetGen()				{return &m_GenCode;}

#ifndef O_64K
	// Remove all generated Data (to regenerate a tree from scratch)
	void Clear();
#endif

	// Main generator function. A new random instance with the given gencode is
	// generated. Calculated is only the abstract modell.
	void Generate();

	// Build up a triangualtion. This generates all kind of vertex informations.
	// Set up the vertex buffers... depends on user.
	void TriangulateCylindrical();

	// Creates an leaf representation and the Leaf-LOD
	void CreateLeaves(TfLeavesetup &_Properties);

	unsigned int GetLODIndexCount(unsigned int _uiLevel)			{return m_pDetailInfos?m_pDetailInfos[_uiLevel].uiDetailIndex:0;}
	unsigned int GetLODNumBranches(unsigned int _uiLevel)			{return m_pDetailInfos?m_pDetailInfos[_uiLevel].uiNumBranches:0;}
	int GetLODMaxGeneration(unsigned int _uiLevel)					{return m_pDetailInfos?m_pDetailInfos[_uiLevel].iMaxGeneration:-1;}
	int GetLODOfGeneration(unsigned int _uiGeneration);			// Calculates which LOD consists branches of the given generation
#ifndef O_64K
	unsigned int GetLODPrimitiveCount(unsigned int _uiLevel)		{return m_pDetailInfos?m_pDetailInfos[_uiLevel].uiDetailIndex/3:0;}
	//void SetLODIndexCount(unsigned int _uiLevel, unsigned int _uiIndex)		{if(m_pDetailInfos) m_pDetailInfos[_uiLevel].uiDetailIndex=_uiIndex;}
#endif
	int GetNumDetailLevels()										{return m_Metrics.iNumTriangulationLevels;}

	bool IsGenerated()												{return m_TreeStatus == TFT_GENERATED;}
};



// **************************************************************************************************************************************************** //
// Verwaltungsstruktur (Bäume merken sich ihre Nachbarn)
class TreeInstance;
/*struct TreeNode
{
	TreeInstance*	pTree;
	TreeNode*		pNext;
};
*/

// **************************************************************************************************************************************************** //
// Instanzen einzelner Geometrien
class TreeInstance
{
friend class Bucket;
private:
	static const int NUM_AFFECTED_NEIGHBOURS = 7;
	static const int DELETE_REQUEST	= 0x00000001;
	static const int BUNRING		= 0x00000004;
	static const int BURNED			= 0x00000008;

	Tree* m_pTree;
	TreeInstance* m_apAffectedNeighbours[NUM_AFFECTED_NEIGHBOURS];	// Übertrage Energie nur an k nächste Nachbarn -> kein dynamisches Managment notwendig.
	int m_iRefCounter;							// Wie viele andere Bäume haben diesen referenziert?

public:
	D3DXMATRIX m_Transform;
	D3DXMATRIX m_TransformInv;
	TreeInstance* m_pNextTree;
	D3DXVECTOR3 m_vPosition;

	TreeInstance(const float _fx,
		const float _fy,
		const float _fz,
		Tree* _pTree);

	int m_Flags;

	bool DecRef()	{if(--m_iRefCounter<0) {delete this; return true;} else return false;}

	void Render(const D3DXMATRIX& View, const D3DXMATRIX& ViewProjection, const D3DXVECTOR3& ViewPos);
};