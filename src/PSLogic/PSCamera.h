#ifndef _PSCAMERA_H
#define _PSCAMERA_H

#include "PS3DMath.h"

class CPSCamera
{
public:
	CPSCamera();

	vec3 m_eyePos;
	vec3 m_eyeDir;
	float m_fAngleX;

	int m_SCY;
	int m_SCX;

	mat4 getProjectMat();
	mat4 getMapProjectMat();

	void Idle( float fElapsedTime );
	void SetStand();
	void RotateX( int );
	mat4 getViewMat();
	mat4 getMapViewMat();
};

#endif //_PSCAMERA_H