#ifndef _PSSKINING_H_
#define _PSSKINING_H_

#include "PS3DMath.h"
#include "PSDefine.h"
#include "PSMotion.h"
#include "PSMaterial.h"

struct SkinNode;

struct SkinMesh
{
	SkinMesh();
	~SkinMesh();

	int NumFaces;
	short* pIndices;

	int NumVertices;
	vec3* pVertices;
	vec3* pOriginVertices;
	vec3* pNormals;
	vec2* pTexCoords;
	
	short NumBones;
	float* pWeights;
	unsigned char* pBoneIds;

	short NumInfluence;
	mat4* pBoneOffsetMatrices;
	SkinNode** pBones;
	char* chBoneNames;

	mat4* pBoneMatrices;

	P3DMaterial Material;
	P3DImage* pImage[SKIN_COUNT];

	vec3 bbMin, bbMax;
	SkinMesh* pNextMesh;
};

struct SkinNode
{
	SkinNode();
	~SkinNode();

	mat4 Trans;
	mat4 WTrans;

	SkinMesh* pFirstMesh;

	char Name[NAME_LEN];
	SkinNode* pSibling;
	SkinNode* pFirstChild;

	vec3 bbMin, bbMax;

	PSJoint* pJoint[MOTION_COUNT];
};

void WriteToPSHFile( SkinNode* pRootNode, char* filepath );
void ReadFromPSHFile( SkinNode** ppRootNode, const char* filepath );

void DrawNode( int skinid, SkinNode* pNode );
void DrawMesh( int skinId, SkinMesh* pMesh, mat4* pParentWMat );

SkinNode* FindNodeByName( SkinNode *pNode, char* name );
void SetBoneMatrix( SkinNode* pNode, SkinNode*pRoot );
void UpdateBoneMatrix( SkinNode* pNode );
void AdvanceMotionMatrix( bool bSkinnig, int nMotionId, int nFrameNum, SkinNode* pNode, mat4 *pParentMat, mat4& OriginMatrix );

void MatchingJoints( int nMotionId, SkinNode* pNode, PSJoint* pJoints, int nJoints );

void CalcBBox( SkinNode* pNode );
void CalcBBox( SkinMesh* pMesh );

extern vector< P3DImage* > g_SkinNodeImagePool;
void DestroySkinNodeImages();

#endif //_PSSKINING_H_