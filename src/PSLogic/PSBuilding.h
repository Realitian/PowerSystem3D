#ifndef _PSBUILDING_H
#define _PSBUILDING_H

#include "PSGeometry.h"
#include "KDTree.h"

class CPSBuilding
{
public:
	CPSBuilding();
	~CPSBuilding();

	Geometry* m_pGeoRoot;
	short* m_pGeoIndices;
	KdTree *m_KdTree;
	PSGemNode* m_pGemNodeRoot;
	PSGemNode* m_pLeftDoorNode;
	PSGemNode* m_pRightDoorNode;
	PSGemNode* m_pStandPointNode;
	vec3 m_StartPos;
public:
	void Unload();
	void Load( const char* psbfilepath );
	void Render( bool bMap, bool bOpaque );
	void Idle( float fElapsedTime );
	void OpenDoor( );
	void CloseDoor();

public:
	void RenderNode( bool bMap, PSGemNode* pNode, bool bOpaque );
	void RenderGeo( P3DGeometry* pGeo, bool bOpaque );

	enum DOORSTATUS
	{
		CLOSED = 0,
		CLOSING,
		OPENING,
		OPENED
	};

	DOORSTATUS m_DoorStatus;

	vec3 m_Center;
	float m_Radius;

	vec3 m_InsideMin;
	vec3 m_InsideMax;

	vec3 m_CenterInside;
	float m_RadiusInside;

	float m_DoorWidth;
	float m_DoorOpenedWidth;

	mat4 m_LeftDoorMat;
	mat4 m_RightDoorMat;
};

#endif //_PSBUILDING_H