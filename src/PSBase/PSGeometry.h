#ifndef _PSGEOMETRY_H_
#define _PSGEOMETRY_H_

#include "PSMaterial.h"
#include "PS3DMath.h"

struct P3DGeometry
{
	P3DGeometry();
	~P3DGeometry();

	unsigned int m_glDlistId;

	int m_nNumFace;
	int m_nNumVertices;

	short *m_pIndices;
	vec3 *m_pVertices;
	vec3 *m_pNormals;
	vec2 *m_pTexCoords;

	P3DMaterial m_Material;
	P3DImage* m_pImage;

	vec3 m_bbMin, m_bbMax;
	P3DGeometry* m_pNextMesh;
};

struct PSGemNode
{
	PSGemNode();
	~PSGemNode();

	mat4 Trans;
	mat4 WTrans;

	P3DGeometry* pFirstMesh;

	char Name[NAME_LEN];
	PSGemNode* pSibling;
	PSGemNode* pFirstChild;

	vec3 bbMin, bbMax;
};

void WriteToPSFile( PSGemNode* pRootNode, char* filepath, vector< P3DImage* > &imagePool );
void ReadFromPSFile( PSGemNode** ppRootNode, const char* filepath, vector< P3DImage* > &imagePool );

PSGemNode* FindNodeByName( PSGemNode *pNode, char* name );
void GetNodeBBox( PSGemNode *pNode, vec3 &bbmin, vec3 &bbmax );
void GetNodeBBox( PSGemNode *pNode, vec3 &bbmin, vec3 &bbmax, mat4* pParent );
void GetNodeBBoxMap( PSGemNode *pNode, vec3 &bbmin, vec3 &bbmax );
bool IsChildNode( PSGemNode* pParent, PSGemNode* pChild );

void UpdateWorldMatrix(PSGemNode* pNode, PSGemNode* pParent);
void UpdateBoundingBox(PSGemNode* pNode);

void NodeToWorldPos( PSGemNode* pNode, vec3 &pos );
void NodeToWorldDir( PSGemNode* pNode, vec3 &pos );
void NodeToInvWorldPos( PSGemNode* pNode, vec3 &pos );
void NodeToInvWorldDir( PSGemNode* pNode, vec3 &pos );

extern vector< P3DImage* > g_EquipNodeImagePool;
extern vector< P3DImage* > g_BuildingImagePool;
void GenEquipNodeImages();
void DestroyEquipNodeImages();

// Stub for Collision Detection
struct PSLineSeg
{
	vec3 start, dir;
	float dist;
};

bool TestCollisionBB( vec3& bbMin, vec3& bbMax, PSLineSeg &lsg, vec3& IntersetedPos );
bool TestCollisionBB( vec3& bbMin, vec3& bbMax, PSLineSeg &lsg );
float TestCollision( P3DGeometry* pGeo, PSLineSeg &lsg );
float TestCollisionTri( vec3& v0, vec3& v1, vec3& v2, PSLineSeg &lsg );

extern PSGemNode* g_pIntersectedNode;
extern P3DGeometry* g_pIntersectedGeo;
float TestCollision( PSGemNode* pNode, PSLineSeg &lsg );

void SwapMinMax( vec3 &vMin, vec3 &vMax );
#endif //_PSGEOMETRY_H_