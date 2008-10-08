#ifdef SUPPORT_HTTP
#include <afxwin.h>
#include "Global.h"
#include "HttpClient.h"
#endif SUPPORT_HTTP
#include "PSGeometry.h"
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>

vector< P3DImage* > g_EquipNodeImagePool(0);
vector< P3DImage* > g_BuildingImagePool(0);
PSGemNode* g_pIntersectedNode = 0;
P3DGeometry* g_pIntersectedGeo = 0;

P3DGeometry::P3DGeometry() : m_Material()
{
	m_nNumFace = 0;
	m_pIndices = 0;

	m_nNumVertices = 0;
	m_pVertices = 0;
	m_pNormals = 0;
	m_pTexCoords = 0;

	m_pNextMesh = 0;
	m_pImage = 0;

	m_glDlistId = 0;
}

P3DGeometry::~P3DGeometry()
{
	SAFE_DELETE_ARRAY ( m_pIndices );
	SAFE_DELETE_ARRAY ( m_pVertices );
	SAFE_DELETE_ARRAY ( m_pNormals );
	SAFE_DELETE_ARRAY ( m_pTexCoords );
	SAFE_DELETE ( m_pNextMesh );
}

PSGemNode::PSGemNode()
{
	Trans = mat4_id;
	WTrans = mat4_id;
	memset( Name, 0, NAME_LEN );

	pFirstMesh = 0;
	pSibling = 0;
	pFirstChild = 0;
}

PSGemNode::~PSGemNode()
{
	SAFE_DELETE( pFirstMesh );
	SAFE_DELETE( pSibling );
	SAFE_DELETE( pFirstChild );
}

void WriteImagePoolToPSFile( FILE* pFile, vector< P3DImage* > &imagePool );
void WriteToPSFile( PSGemNode* pNode, FILE* pFile );
void WriteGeometryToPSFile( P3DGeometry* pMesh, FILE* pFile );
void WriteMaterialToPSFile( P3DMaterial &pMesh, FILE* pFile );

void WriteToPSFile( PSGemNode* pRootNode, char* filepath, vector< P3DImage* > &imagePool )
{
	FILE* pFile = fopen( filepath, "w+b");
	if ( pFile )
	{
		WriteImagePoolToPSFile( pFile, imagePool );
		WriteToPSFile( pRootNode, pFile );
		fclose( pFile );
	}

	file_compress( filepath );
}

void WriteToPSFile( PSGemNode* pNode, FILE* pFile )
{
	char token;
	if ( pNode )
	{
		token = 1;
		fwrite( &token, 1, 1, pFile );

		fwrite( &pNode->Name, NAME_LEN, 1, pFile );
		fwrite( pNode->Trans.mat_array, sizeof(mat4), 1, pFile );
		fwrite( pNode->WTrans.mat_array, sizeof(mat4), 1, pFile );

		WriteGeometryToPSFile( pNode->pFirstMesh, pFile );

		pNode->bbMin = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
		pNode->bbMax = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

		P3DGeometry* pGeo = pNode->pFirstMesh;
		
		while( pGeo )
		{
			if ( pNode->bbMin.x > pGeo->m_bbMin.x )
				pNode->bbMin.x = pGeo->m_bbMin.x;
			if ( pNode->bbMin.y > pGeo->m_bbMin.y )
				pNode->bbMin.y = pGeo->m_bbMin.y;
			if ( pNode->bbMin.z > pGeo->m_bbMin.z )
				pNode->bbMin.z = pGeo->m_bbMin.z;

			if ( pNode->bbMax.x < pGeo->m_bbMax.x )
				pNode->bbMax.x = pGeo->m_bbMax.x;
			if ( pNode->bbMax.y < pGeo->m_bbMax.y )
				pNode->bbMax.y = pGeo->m_bbMax.y;
			if ( pNode->bbMax.z < pGeo->m_bbMax.z )
				pNode->bbMax.z = pGeo->m_bbMax.z;

			pGeo = pGeo->m_pNextMesh;
		}

		if ( pNode->pFirstMesh )
		{
			pNode->bbMin = pNode->WTrans * pNode->bbMin;
			pNode->bbMax = pNode->WTrans * pNode->bbMax;

			SwapMinMax( pNode->bbMin, pNode->bbMax );
		}

		fwrite( pNode->bbMin.vec_array, sizeof(vec3), 1, pFile );
		fwrite( pNode->bbMax.vec_array, sizeof(vec3), 1, pFile );

		WriteToPSFile( pNode->pFirstChild, pFile );
		WriteToPSFile( pNode->pSibling, pFile );
	}
	else
	{
		token = 0;
		fwrite( &token, 1, 1, pFile );
	}
}

void WriteGeometryToPSFile( P3DGeometry* pMesh, FILE* pFile )
{
	char token;
	if ( pMesh )
	{
		token = 1;
		fwrite( &token, 1, 1, pFile );

		fwrite( &pMesh->m_nNumFace, sizeof(int), 1, pFile );
		fwrite( pMesh->m_pIndices, sizeof(short)*pMesh->m_nNumFace*3, 1, pFile );

		fwrite( &pMesh->m_nNumVertices, sizeof(int), 1, pFile );
		fwrite( pMesh->m_pVertices, sizeof(vec3)*pMesh->m_nNumVertices, 1, pFile );
		fwrite( pMesh->m_pNormals, sizeof(vec3)*pMesh->m_nNumVertices, 1, pFile );
		fwrite( pMesh->m_pTexCoords, sizeof(vec2)*pMesh->m_nNumVertices, 1, pFile );

		pMesh->m_bbMin = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
		pMesh->m_bbMax = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

		for ( int v = 0 ; v < pMesh->m_nNumVertices ; v++ )
		{
			if ( pMesh->m_bbMin.x > pMesh->m_pVertices[v].x )
				pMesh->m_bbMin.x = pMesh->m_pVertices[v].x;
			if ( pMesh->m_bbMin.y > pMesh->m_pVertices[v].y )
				pMesh->m_bbMin.y = pMesh->m_pVertices[v].y;
			if ( pMesh->m_bbMin.z > pMesh->m_pVertices[v].z )
				pMesh->m_bbMin.z = pMesh->m_pVertices[v].z;

			if ( pMesh->m_bbMax.x < pMesh->m_pVertices[v].x )
				pMesh->m_bbMax.x = pMesh->m_pVertices[v].x;
			if ( pMesh->m_bbMax.y < pMesh->m_pVertices[v].y )
				pMesh->m_bbMax.y = pMesh->m_pVertices[v].y;
			if ( pMesh->m_bbMax.z < pMesh->m_pVertices[v].z )
				pMesh->m_bbMax.z = pMesh->m_pVertices[v].z;
		}

		fwrite( pMesh->m_bbMin.vec_array, sizeof(vec3), 1, pFile );
		fwrite( pMesh->m_bbMax.vec_array, sizeof(vec3), 1, pFile );

		WriteMaterialToPSFile( pMesh->m_Material, pFile );
		char token;
		if( pMesh->m_pImage )
		{
			token = 1;
			fwrite( &token, 1, 1, pFile );

			fwrite( pMesh->m_pImage->img_path, PATH_LEN, 1, pFile );
		}
		else
		{
			token = 0;
			fwrite( &token, 1, 1, pFile );
		}
		
		WriteGeometryToPSFile( pMesh->m_pNextMesh, pFile );
	}
	else
	{
		token = 0;
		fwrite( &token, 1, 1, pFile );
	}
}

void WriteMaterialToPSFile( P3DMaterial& m_omat, FILE* pFile )
{
	fwrite( m_omat.ambient, sizeof(float)*4, 1, pFile );
	fwrite( m_omat.diffuse, sizeof(float)*4, 1, pFile );
	fwrite( m_omat.emission, sizeof(float)*4, 1, pFile );
	fwrite( m_omat.specular, sizeof(float)*4, 1, pFile );
	fwrite( &m_omat.shininess, sizeof(float), 1, pFile );
}

void WriteImagePoolToPSFile( FILE* pFile, vector< P3DImage* > &imagePool )
{
	int PoolSize = (int)imagePool.size();
	fwrite( &PoolSize, sizeof(int), 1, pFile );

	for ( int i = 0 ; i < PoolSize ; i++ )
		fwrite( imagePool[i]->img_path, PATH_LEN, 1, pFile );
}

void ReadImagePoolFromPSFile( char* &pBuf, vector< P3DImage* > &imagePool );
void ReadFromPSFile( PSGemNode** ppNode, char* &pBuf, vector< P3DImage* > &imagePool );
void ReadGeometryFromPSFile( P3DGeometry** ppMesh, char* &pBuf, vector< P3DImage* > &imagePool );
void ReadMaterialFromPSFile( P3DMaterial &pMesh, char* &pBuf );

void ReadFromPSFile( PSGemNode** ppRootNode, const char* filepath, vector< P3DImage* > &imagePool )
{
#ifdef SUPPORT_HTTP
	HttpDownLoad( filepath );
#endif SUPPORT_HTTP

	char uncompressedFile[MAX_PATH];
	strcpy( uncompressedFile, filepath );
	strcat( uncompressedFile, ".gz" );
	file_uncompress( filepath, uncompressedFile );

	int fnLen = 0;
	int fh = 0;
	if ( _sopen_s( &fh, uncompressedFile, _O_RDONLY|_O_BINARY, _SH_DENYNO, _S_IREAD ) == 0 )
	{
		fnLen = _filelength( fh );

		char* pBuf = new char[fnLen];
		char* pOrigin = pBuf;
		_read( fh, pBuf, fnLen );

		ReadImagePoolFromPSFile( pBuf, imagePool );
		ReadFromPSFile( ppRootNode, pBuf, imagePool );

		_close( fh );
		SAFE_DELETE_ARRAY( pOrigin );
	}

	_unlink( uncompressedFile );
}

void ReadFromPSFile( PSGemNode** ppNode, char* &pBuf, vector< P3DImage* > &imagePool )
{
	char token;
	memcpy( &token, pBuf, 1 );
	pBuf ++;

	if ( token )
	{
		*ppNode = new PSGemNode;

		memcpy( &((*ppNode)->Name), pBuf, NAME_LEN );
		pBuf += NAME_LEN;
		memcpy( &((*ppNode)->Trans), pBuf, sizeof(mat4) );
		pBuf += sizeof(mat4);
		memcpy( &((*ppNode)->WTrans), pBuf, sizeof(mat4) );
		pBuf += sizeof(mat4);

		ReadGeometryFromPSFile( &((*ppNode)->pFirstMesh), pBuf, imagePool );

		memcpy( (*ppNode)->bbMin.vec_array, pBuf, sizeof(vec3) );
		pBuf += sizeof(vec3);
		memcpy( (*ppNode)->bbMax.vec_array, pBuf, sizeof(vec3) );
		pBuf += sizeof(vec3);

		ReadFromPSFile( &((*ppNode)->pFirstChild), pBuf, imagePool );
		ReadFromPSFile( &((*ppNode)->pSibling), pBuf, imagePool );
	}
	else
	{
		*ppNode = 0;
	}
}

void ReadGeometryFromPSFile( P3DGeometry** ppMesh, char* &pBuf, vector< P3DImage* > &imagePool )
{
	char token;
	memcpy( &token, pBuf, 1 );
	pBuf ++;

	if ( token )
	{
		(*ppMesh) = new P3DGeometry;

		memcpy( &((*ppMesh)->m_nNumFace), pBuf, sizeof(int) );
		pBuf += sizeof(int);

		(*ppMesh)->m_pIndices = new short[(*ppMesh)->m_nNumFace*3];
		memcpy( (*ppMesh)->m_pIndices, pBuf, sizeof(short)*((*ppMesh)->m_nNumFace*3) );
		pBuf += sizeof(short)*((*ppMesh)->m_nNumFace*3);

		memcpy( &((*ppMesh)->m_nNumVertices), pBuf, sizeof(int) );
		pBuf += sizeof(int);

		(*ppMesh)->m_pVertices = new vec3[(*ppMesh)->m_nNumVertices];
		memcpy( (*ppMesh)->m_pVertices, pBuf, sizeof(vec3)*((*ppMesh)->m_nNumVertices) );
		pBuf += sizeof(vec3)*((*ppMesh)->m_nNumVertices);

		(*ppMesh)->m_pNormals = new vec3[(*ppMesh)->m_nNumVertices];
		memcpy( (*ppMesh)->m_pNormals, pBuf, sizeof(vec3)*((*ppMesh)->m_nNumVertices) );
		pBuf += sizeof(vec3)*((*ppMesh)->m_nNumVertices);
		
		(*ppMesh)->m_pTexCoords = new vec2[(*ppMesh)->m_nNumVertices];
		memcpy( (*ppMesh)->m_pTexCoords, pBuf, sizeof(vec2)*((*ppMesh)->m_nNumVertices) );
		pBuf += sizeof(vec2)*((*ppMesh)->m_nNumVertices);

		memcpy( &(*ppMesh)->m_bbMin, pBuf, sizeof(vec3) );
		pBuf += sizeof(vec3);
		memcpy( &(*ppMesh)->m_bbMax, pBuf, sizeof(vec3) );
		pBuf += sizeof(vec3);

		ReadMaterialFromPSFile( (*ppMesh)->m_Material, pBuf );

		char token;
		(*ppMesh)->m_pImage = 0;
		memcpy( &token, pBuf, 1 );
		pBuf ++;

		if( token )
		{
			char texfilepath[PATH_LEN];
			memcpy( texfilepath, pBuf, PATH_LEN );
			pBuf += PATH_LEN;

			for ( size_t iImage = 0 ; iImage < imagePool.size() ; iImage++ )
			{
				if( strcmp(imagePool[iImage]->img_path, texfilepath) == 0 )
					(*ppMesh)->m_pImage = imagePool[iImage];
			}
		}

		ReadGeometryFromPSFile( &((*ppMesh)->m_pNextMesh), pBuf, imagePool );
	}
	else
	{
		(*ppMesh) = 0;
	}
}

void ReadMaterialFromPSFile( P3DMaterial& m_omat, char* &pBuf )
{
	memcpy( &m_omat.ambient, pBuf, sizeof(float)*4 );
	pBuf += sizeof(float)*4;
	memcpy( &m_omat.diffuse, pBuf, sizeof(float)*4 );
	pBuf += sizeof(float)*4;
	memcpy( &m_omat.emission, pBuf, sizeof(float)*4 );
	pBuf += sizeof(float)*4;
	memcpy( &m_omat.specular, pBuf, sizeof(float)*4 );
	pBuf += sizeof(float)*4;
	memcpy( &m_omat.shininess, pBuf, sizeof(float) );
	pBuf += sizeof(float);
}

void ReadImagePoolFromPSFile( char* &pBuf, vector< P3DImage* > &imagePool )
{
	int PoolSize = 0;
	memcpy( &PoolSize, pBuf, sizeof(int) );
	pBuf += sizeof(int);

	for ( int i = 0 ; i < PoolSize ; i++ )
	{
		P3DImage* pImage = new P3DImage;
	
		memcpy( pImage->img_path, pBuf, PATH_LEN );
		pBuf += PATH_LEN;

		GLint iComp;
		GLenum eFormat;
		pImage->pPixels = (unsigned char*)LoadImage( pImage->img_path, &pImage->img_width, &pImage->img_height, &iComp, &eFormat );

		bool bInsert = true;
		for ( size_t iImage = 0 ; iImage < imagePool.size() ; iImage++ )
		{
			if( strcmp(imagePool[iImage]->img_path, pImage->img_path) == 0 )
			{
				delete pImage;
				bInsert = false;
				break;
			}
		}

		if ( bInsert )
			imagePool.push_back(pImage);
	}
}

PSGemNode* FindNodeByName( PSGemNode *pNode, char* name )
{
	if ( pNode == 0 )
		return 0;

	if ( strcmp( pNode->Name, name ) == 0 )
		return pNode;

	PSGemNode* pFindNode = FindNodeByName( pNode->pFirstChild, name );
	if ( pFindNode )
		return pFindNode;

	pFindNode = FindNodeByName( pNode->pSibling, name );
	if ( pFindNode )
		return pFindNode;

	return 0;
}

void NodeToWorldPos( PSGemNode* pNode, vec3 &pos )
{
	if ( pNode )
		pos = pNode->WTrans * pos;
}

void NodeToInvWorldPos( PSGemNode* pNode, vec3 &pos )
{
	if ( pNode )
	{
		mat4 itran;
		invert( itran, pNode->WTrans );
		pos = itran * pos;
	}
}

void NodeToWorldDir( PSGemNode* pNode, vec3 &pos )
{
	if ( pNode )
	{
		vec4 pos4( pos );
		pos4.w = 0;
		pos4 = pNode->WTrans * pos4;
		pos = pos4;
	}
}

void NodeToInvWorldDir( PSGemNode* pNode, vec3 &pos )
{
	if ( pNode == 0 )
	{
		mat4 itran;
		invert( itran, pNode->WTrans );
		mat3 irot;
		itran.get_rot( irot );
		pos = irot * pos;
	}
}

void GetNodeBBoxMap( PSGemNode *pNode, vec3 &bbmin, vec3 &bbmax )
{
	if ( pNode == 0 )
		return;

	if ( strstr( pNode->Name, BUILDING_ROOF_NAME ) || strstr( pNode->Name, BUILDING_OUTSIDE_NAME ) )
		return;

	if ( bbmin.x > pNode->bbMin.x )
		bbmin.x = pNode->bbMin.x;
	if ( bbmin.y > pNode->bbMin.y )
		bbmin.y = pNode->bbMin.y;
	if ( bbmin.z > pNode->bbMin.z )
		bbmin.z = pNode->bbMin.z;

	if ( bbmax.x < pNode->bbMax.x )
		bbmax.x = pNode->bbMax.x;
	if ( bbmax.y < pNode->bbMax.y )
		bbmax.y = pNode->bbMax.y;
	if ( bbmax.z < pNode->bbMax.z )
		bbmax.z = pNode->bbMax.z;

	PSGemNode* pNextNode = pNode->pFirstChild;

	while ( pNextNode )
	{
		GetNodeBBoxMap( pNextNode, bbmin, bbmax );
		pNextNode = pNextNode->pSibling;
	}
}

void GetNodeBBox( PSGemNode *pNode, vec3 &bbmin, vec3 &bbmax, mat4* pParentMat )
{
	mat4 nodeTran = pNode->Trans;
	if ( pParentMat )
		nodeTran = (*pParentMat) * nodeTran;

	P3DGeometry* pGeo = pNode->pFirstMesh;

	vec3 geobbMin = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
	vec3 geobbMax = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	while( pGeo )
	{
		if ( geobbMin.x > pGeo->m_bbMin.x )
			geobbMin.x = pGeo->m_bbMin.x;
		if ( geobbMin.y > pGeo->m_bbMin.y )
			geobbMin.y = pGeo->m_bbMin.y;
		if ( geobbMin.z > pGeo->m_bbMin.z )
			geobbMin.z = pGeo->m_bbMin.z;

		if ( geobbMax.x < pGeo->m_bbMax.x )
			geobbMax.x = pGeo->m_bbMax.x;
		if ( geobbMax.y < pGeo->m_bbMax.y )
			geobbMax.y = pGeo->m_bbMax.y;
		if ( geobbMax.z < pGeo->m_bbMax.z )
			geobbMax.z = pGeo->m_bbMax.z;

		pGeo = pGeo->m_pNextMesh;
	}

	if ( pNode->pFirstMesh )
	{
		geobbMin = nodeTran * geobbMin;
		geobbMax = nodeTran * geobbMax;

		SwapMinMax( geobbMin, geobbMax );

		if ( bbmin.x > geobbMin.x )
			bbmin.x = geobbMin.x;
		if ( bbmin.y > geobbMin.y )
			bbmin.y = geobbMin.y;
		if ( bbmin.z > geobbMin.z )
			bbmin.z = geobbMin.z;

		if ( bbmax.x < geobbMax.x )
			bbmax.x = geobbMax.x;
		if ( bbmax.y < geobbMax.y )
			bbmax.y = geobbMax.y;
		if ( bbmax.z < geobbMax.z )
			bbmax.z = geobbMax.z;
	}

	PSGemNode* pNextNode = pNode->pFirstChild;

	while ( pNextNode )
	{
		GetNodeBBox( pNextNode, bbmin, bbmax, &nodeTran );
		pNextNode = pNextNode->pSibling;
	}
}

void GetNodeBBox( PSGemNode *pNode, vec3 &bbmin, vec3 &bbmax )
{
	if ( pNode == 0 )
		return;

	if ( bbmin.x > pNode->bbMin.x )
		bbmin.x = pNode->bbMin.x;
	if ( bbmin.y > pNode->bbMin.y )
		bbmin.y = pNode->bbMin.y;
	if ( bbmin.z > pNode->bbMin.z )
		bbmin.z = pNode->bbMin.z;

	if ( bbmax.x < pNode->bbMax.x )
		bbmax.x = pNode->bbMax.x;
	if ( bbmax.y < pNode->bbMax.y )
		bbmax.y = pNode->bbMax.y;
	if ( bbmax.z < pNode->bbMax.z )
		bbmax.z = pNode->bbMax.z;

	PSGemNode* pNextNode = pNode->pFirstChild;
	
	while ( pNextNode )
	{
		GetNodeBBox( pNextNode, bbmin, bbmax );
		pNextNode = pNextNode->pSibling;
	}
}

bool IsChildNode( PSGemNode* pParent, PSGemNode* pChild )
{
	if ( pParent == 0 )
		return false;

	if ( pParent == pChild )
		return true;

	PSGemNode* pNode = pParent->pFirstChild;
	while( pNode )
	{
		if ( IsChildNode( pNode, pChild ) )
			return true;

		pNode = pNode->pSibling;
	}

	return false;
}

void UpdateWorldMatrix(PSGemNode* pNode, PSGemNode* pParent)
{
	if(pNode == NULL)
		return;

	pNode->WTrans = mat4_id;

	if(pParent)
	{
		pNode->WTrans = pParent->WTrans * pNode->Trans;
	}
	else
	{
		pNode->WTrans = pNode->Trans;
	}

	UpdateWorldMatrix(pNode->pSibling, pParent);
	UpdateWorldMatrix(pNode->pFirstChild, pNode);
}

void UpdateBoundingBox(PSGemNode* pNode)
{
	if(pNode == NULL)
		return;

	pNode->bbMin = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
	pNode->bbMax = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	P3DGeometry* pGeo = pNode->pFirstMesh;

	while( pGeo )
	{
		if ( pNode->bbMin.x > pGeo->m_bbMin.x )
			pNode->bbMin.x = pGeo->m_bbMin.x;
		if ( pNode->bbMin.y > pGeo->m_bbMin.y )
			pNode->bbMin.y = pGeo->m_bbMin.y;
		if ( pNode->bbMin.z > pGeo->m_bbMin.z )
			pNode->bbMin.z = pGeo->m_bbMin.z;

		if ( pNode->bbMax.x < pGeo->m_bbMax.x )
			pNode->bbMax.x = pGeo->m_bbMax.x;
		if ( pNode->bbMax.y < pGeo->m_bbMax.y )
			pNode->bbMax.y = pGeo->m_bbMax.y;
		if ( pNode->bbMax.z < pGeo->m_bbMax.z )
			pNode->bbMax.z = pGeo->m_bbMax.z;

		pGeo = pGeo->m_pNextMesh;
	}

	if ( pNode->pFirstMesh )
	{
		pNode->bbMin = pNode->WTrans * pNode->bbMin;
		pNode->bbMax = pNode->WTrans * pNode->bbMax;

		SwapMinMax( pNode->bbMin, pNode->bbMax );
	}

	UpdateBoundingBox(pNode->pSibling);
	UpdateBoundingBox(pNode->pFirstChild);
}

bool TestCollisionBB( vec3& bbMin, vec3& bbMax, PSLineSeg &lsg )
{
	vec3 end = lsg.start + lsg.dist * lsg.dir;

	if ( end.x > bbMin.x && end.x < bbMax.x &&
		end.z > bbMin.z && end.z < bbMax.z )
		return true;

	return false;
}

bool TestCollisionBB( vec3& bbMin, vec3& bbMax, PSLineSeg &lsg, vec3& IntersetedPos )
{
#define NUMDIM	3
#define RIGHT	0
#define LEFT	1
#define MIDDLE	2
	bool inside = true;
	char quadrant[NUMDIM];
	register int i;
	int whichPlane;
	ps_scalar maxT[NUMDIM];
	ps_scalar candidatePlane[NUMDIM];

	/* Find candidate planes; this loop can be avoided if
	rays cast all from the eye(assume perpsective view) */
	for (i=0; i<NUMDIM; i++)
		if(lsg.start.vec_array[i] < bbMin.vec_array[i]) {
			quadrant[i] = LEFT;
			candidatePlane[i] = bbMin.vec_array[i];
			inside = false;
		}else if (lsg.start.vec_array[i] > bbMax.vec_array[i]) {
			quadrant[i] = RIGHT;
			candidatePlane[i] = bbMax.vec_array[i];
			inside = false;
		}else	{
			quadrant[i] = MIDDLE;
		}

		/* Ray lsg.start.vec_array inside bounding box */
		if(inside)	{
			IntersetedPos = lsg.start;
			return (true);
		}

		/* Calculate T distances to candidate planes */
		for (i = 0; i < NUMDIM; i++)
			if (quadrant[i] != MIDDLE && lsg.dir.vec_array[i] !=0.)
				maxT[i] = (candidatePlane[i]-lsg.start.vec_array[i]) / lsg.dir.vec_array[i];
			else
				maxT[i] = -1.;

		/* Get largest of the maxT's for final choice of intersection */
		whichPlane = 0;
		for (i = 1; i < NUMDIM; i++)
			if (maxT[whichPlane] < maxT[i])
				whichPlane = i;

		/* Check final candidate actually inside box */
		if (maxT[whichPlane] < 0.) return (false);
		for (i = 0; i < NUMDIM; i++)
			if (whichPlane != i) {
				IntersetedPos.vec_array[i] = lsg.start.vec_array[i] + maxT[whichPlane] *lsg.dir.vec_array[i];
				if (IntersetedPos.vec_array[i] < bbMin.vec_array[i] || IntersetedPos.vec_array[i] > bbMax.vec_array[i])
					return (false);
			} else {
				IntersetedPos.vec_array[i] = candidatePlane[i];
			}
			return (true);				/* ray hits box */
}

float TestCollisionTri( vec3& v0, vec3& v1, vec3& v2, PSLineSeg &lsg )
{
	// Find vectors for two edges sharing vert0
	vec3 edge1 = v1 - v0;
	vec3 edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	vec3 pvec;
	cross( pvec, lsg.dir, edge2 );

	// If determinant is near zero, ray lies in plane of triangle
	float det = dot( edge1, pvec );

	vec3 tvec;
	if( det > 0 )
	{
		tvec = lsg.start - v0;
	}
	else
	{
		tvec = v0 - lsg.start;
		det = -det;
	}

	if( det < 0.0001f )
		return FLT_MAX;

	// Calculate U parameter and test bounds
	float u = dot( tvec, pvec );
	if( u < 0.0f || u > det )
		return FLT_MAX;

	// Prepare to test V parameter
	vec3 qvec;
	cross( qvec, tvec, edge1 );

	// Calculate V parameter and test bounds
	float v = dot( lsg.dir, qvec );
	if( v < 0.0f || u + v > det )
		return FLT_MAX;

	// Calculate t, scale parameters, ray intersects triangle
	float t = dot( edge2, qvec );
	float fInvDet = 1.0f / det;
	t *= fInvDet;

	if ( t > lsg.dist || t < 0 )
		return FLT_MAX;

	lsg.dist = t;
	return t;
}

float TestCollision( P3DGeometry* pGeo, PSLineSeg &lsg )
{
	float fDist = FLT_MAX;

	for (int i = 0; i<pGeo->m_nNumFace;i++)
	{
		float fCollidedDist = TestCollisionTri( pGeo->m_pVertices[ pGeo->m_pIndices[3*i] ], pGeo->m_pVertices[ pGeo->m_pIndices[3*i+1] ], pGeo->m_pVertices[ pGeo->m_pIndices[3*i+2] ], lsg);
		if ( fDist > fCollidedDist )
			fDist = fCollidedDist;
	}

	return fDist;
}

float TestCollision( PSGemNode* pNode, PSLineSeg &lsg )
{
	if ( pNode == 0 )
		return FLT_MAX;

	float fDist = FLT_MAX;

 	PSLineSeg iwlsg = lsg;
 	NodeToInvWorldPos( pNode, iwlsg.start );
 	vec3 end = lsg.start + lsg.dist * lsg.dir;
 	NodeToInvWorldPos( pNode, end );
 	iwlsg.dir = end - iwlsg.start;
 	iwlsg.dir.normalize();
 
 	P3DGeometry* pGeo = pNode->pFirstMesh;
 	while( pGeo )
 	{
 		vec3 bbMinW = pNode->WTrans * pGeo->m_bbMin;
 		vec3 bbMaxW = pNode->WTrans * pGeo->m_bbMax;
 		SwapMinMax( bbMinW, bbMaxW );
 		vec3 intersectPos;
 		if ( TestCollisionBB( bbMinW, bbMaxW, lsg, intersectPos ) )
 		{
 			float fCollidedDist = TestCollision( pGeo, iwlsg );
 			if ( fDist > fCollidedDist )
 			{
 				g_pIntersectedNode = pNode;
 				g_pIntersectedGeo = pGeo;
 				fDist = fCollidedDist;
 				lsg.dist = iwlsg.dist;
 			}
 		}
 		pGeo = pGeo->m_pNextMesh;
 	}

	float fFirstChildDist = TestCollision( pNode->pFirstChild, lsg );
	float fSiblingDist = TestCollision( pNode->pSibling, lsg );

	fDist = min( fDist, fFirstChildDist );
	fDist = min( fDist, fSiblingDist );

	return fDist;
}

void SwapMinMax( vec3 &vMin, vec3 &vMax )
{
	if ( vMin.x > vMax.x )
	{
		float temp = vMin.x;
		vMin.x = vMax.x;
		vMax.x = temp;
	}
	if ( vMin.y > vMax.y )
	{
		float temp = vMin.y;
		vMin.y = vMax.y;
		vMax.y = temp;
	}
	if ( vMin.z > vMax.z )
	{
		float temp = vMin.z;
		vMin.z = vMax.z;
		vMax.z = temp;
	}
}
