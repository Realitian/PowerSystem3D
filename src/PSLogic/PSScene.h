#ifndef _PSSCENE_H
#define _PSSCENE_H
#include <vector>
using namespace std;

#include "PSEquipment.h"
#include "PSBuilding.h"

#define EQUIP_NUM 3
#define MAX_EQUIP_TYPE 4

class PSScene
{
public:
	PSScene();
	~PSScene();
	void FindEquip( char* strNodeName, PSGemNode* pGemNode, PSGemNode* pBuildingTree );
	CPSEquipment*  TestCollisionBB( PSLineSeg &lsg );
	float TestCollisionBBNearest( PSLineSeg &lsg );
	void UnLoadEquip();
	void LoadEquipment();
	void Render( bool bMap );
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void SetCtrlValue(short actionID, int ctrlID, float ctrlValue);

	void Idle(float fElapsedTime);
	void StartOperateWithEquip();
	void EndOperateWithEquip();

	void ResetEquips();
public:
	vector<CPSEquipment*> m_vecEquipments;
	vector<PSGemNode*> m_vecEquipNodes;
	CPSEquipment* m_pCurEquip;
};

#endif //_PSSCENE_H