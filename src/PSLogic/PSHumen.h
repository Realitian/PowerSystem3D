#ifndef _PSHUMEN_H
#define _PSHUMEN_H

#include "PSMotion.h"
#include "Global.h"
struct SkinNode;

class PSHumen
{
public:
	PSHumen();
	~PSHumen();

public:
	void Idle( float fElapsedTime );
	void Render( PSStage stage );

	void Walk();
	void Run();
	void Forward( float dx, float dz );
	
	void RotateStart( int x );
	void Rotate( int x );
	void RotateY( int x );
	void RotateEnd( int x );

	void Stand();
	void Knokout();
	
	void SelectSkin( int id );
	void SelectSex( bool bMale );
	void Load( );
	bool m_bLoaded;

public:
	SkinNode* m_pSkinNode[2];
	PSMotion m_Motions[MOTION_COUNT];

	vec3 m_Pos;
	float m_fAngleY;
	int m_iSkinId;
	int m_iSexId;
	int m_iMotionId;

	float m_RotSign;
	int m_RotX;
	bool m_bRotating;
};

#endif //_PSHUMEN_H