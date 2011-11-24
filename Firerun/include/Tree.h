
// **************************************************************************************************************************************************** //
// Geometrien generierter Bäume
class Tree
{
};

// **************************************************************************************************************************************************** //
// Verwaltungsstruktur (Bäume merken sich ihre Nachbarn)
class TreeInstance;
struct TreeNode
{
	TreeInstance*	pTree;
	TreeNode*		pNext;
};

// **************************************************************************************************************************************************** //
// Statusflags des Baums
enum TreeStatus {
	DELETE_REQUEST	= 0x00000001,	// Löschrequest, dieser Baum wird in Zukunft gelöscht und darf
									// nicht mehrreferenziert werden.
	BURN_REQUEST	= 0x00000002,	// Entzündungsrequest, dieser Baum entflammt
	BUNRING			= 0x00000004,	//	Burning
	BURNED			= 0x00000008	//	Burned
};

// Instanzen einzelner Geometrien
class TreeInstance
{
private:
	Tree* m_pTree;
	TreeNode* m_pAffectedNeighbours;

public:
	D3DXMATRIX m_Transform;

	TreeInstance(const float _fx,
		const float _fy,
		const float _fz,
		Tree* _pTree);

	TreeStatus m_Flags;

	void Render();
};