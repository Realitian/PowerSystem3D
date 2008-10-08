#ifndef _PSMOTION_H_
#define _PSMOTION_H_

#include "PS3DMath.h"
#include "PSDefine.h"

struct PSJoint
{
	PSJoint();
	~PSJoint();

	char name[NAME_LEN];
	vec3* rotAngle;
	mat4* rot;

	void ReadFromPSMFile( FILE* pFile, int nframeNum );
	void WriteToPSMFile( FILE* pFile, int nframeNum );
};

struct PSMotion
{
	PSMotion();
	~PSMotion();

	int nFrameNum;
	float fFrameInterval;
	int nJointNum;
	PSJoint* pJoints;

	void ReadFromPSMFile( const char* filepath );
	void WriteToPSMFile( char* filepath );
};

#endif //_PSMOTION_H_
