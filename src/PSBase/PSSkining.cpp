#ifdef SUPPORT_HTTP
#include <afxwin.h>
#include "Global.h"
#include "HttpClient.h"
#endif SUPPORT_HTTP
#include "PSSkining.h"
#include <Windows.h>
#include <gl/GL.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>

vector< P3DImage* > g_SkinNodeImagePool(0);

SkinMesh::SkinMesh() : Material()
{
	NumFaces = 0;
	pIndices = 0;

	NumVertices = 0;
	pVertices = 0;
	pOriginVertices = 0;
	pNormals = 0;
	pTexCoords = 0;

	pWeights = 0;
	pBoneIds = 0;

	NumInfluence = 0;
	NumBones = 0;
	pBoneOffsetMatrices = 0;
	pBones = 0;
	chBoneNames = 0;

	pBoneMatrices = 0;
	pNextMesh = 0;
	pImage[0] = 0;
	pImage[1] = 0;
}

SkinMesh::~SkinMesh()
{
	SAFE_DELETE_ARRAY ( pIndices );

	SAFE_DELETE_ARRAY ( pOriginVertices );
	SAFE_DELETE_ARRAY ( pVertices );
	SAFE_DELETE_ARRAY ( pNormals );
	SAFE_DELETE_ARRAY ( pTexCoords );

	SAFE_DELETE_ARRAY ( pWeights );
	SAFE_DELETE_ARRAY ( pBoneIds );

	SAFE_DELETE_ARRAY ( pBoneOffsetMatrices );
	SAFE_DELETE_ARRAY ( pBones );
	SAFE_DELETE_ARRAY ( chBoneNames );
	SAFE_DELETE ( pNextMesh );

	SAFE_DELETE_ARRAY ( pBoneMatrices );
}

SkinNode::SkinNode()
{
	Trans = mat4_id;
	WTrans = mat4_id;
	memset( Name, 0, NAME_LEN );

	pFirstMesh = 0;
	pSibling = 0;
	pFirstChild = 0;
	
	for ( int j = 0 ; j < MOTION_COUNT ; j++ )
		pJoint[j] = 0;
}

SkinNode::~SkinNode()
{
	SAFE_DELETE( pFirstMesh );
	SAFE_DELETE( pSibling );
	SAFE_DELETE( pFirstChild );
}

void WriteImagePoolToPSHFile( FILE* pFile );
void WriteToPSHFile( SkinNode* pNode, FILE* pFile );
void WriteSkinMeshToPSHFile( SkinMesh* pMesh, FILE* pFile );
void WriteMaterialToPSHFile( P3DMaterial &pMesh, FILE* pFile );

void WriteToPSHFile( SkinNode* pRootNode, char* filepath )
{
	FILE* pFile = fopen( filepath, "w+b");
	if ( pFile )
	{
		WriteImagePoolToPSHFile( pFile );
		WriteToPSHFile( pRootNode, pFile );
		fclose( pFile );
	}

	file_compress( filepath );
}

void WriteToPSHFile( SkinNode* pNode, FILE* pFile )
{
	char token;
	if ( pNode )
	{
		token = 1;
		fwrite( &token, 1, 1, pFile );
		
		fwrite( &pNode->Name, NAME_LEN, 1, pFile );
		fwrite( &pNode->Trans, sizeof(mat4), 1, pFile );
		fwrite( &pNode->WTrans, sizeof(mat4), 1, pFile );

		WriteSkinMeshToPSHFile( pNode->pFirstMesh, pFile );
		WriteToPSHFile( pNode->pFirstChild, pFile );
		WriteToPSHFile( pNode->pSibling, pFile );
	}
	else
	{
		token = 0;
		fwrite( &token, 1, 1, pFile );
	}
}

void WriteSkinMeshToPSHFile( SkinMesh* pMesh, FILE* pFile )
{
	char token;
	if ( pMesh )
	{
		token = 1;
		fwrite( &token, 1, 1, pFile );

		fwrite( &pMesh->NumFaces, sizeof(int), 1, pFile );
		fwrite( pMesh->pIndices, sizeof(short)*pMesh->NumFaces*3, 1, pFile );

		fwrite( &pMesh->NumVertices, sizeof(int), 1, pFile );
		fwrite( pMesh->pVertices, sizeof(vec3)*pMesh->NumVertices, 1, pFile );
		fwrite( pMesh->pNormals, sizeof(vec3)*pMesh->NumVertices, 1, pFile );
		fwrite( pMesh->pTexCoords, sizeof(vec2)*pMesh->NumVertices, 1, pFile );
		fwrite( pMesh->pWeights, sizeof(float)*4*pMesh->NumVertices, 1, pFile );
		fwrite( pMesh->pBoneIds, 4*pMesh->NumVertices, 1, pFile );

		fwrite( &pMesh->NumBones, sizeof(short), 1, pFile );
		fwrite( &pMesh->NumInfluence, sizeof(short), 1, pFile );
		fwrite( pMesh->pBoneOffsetMatrices, sizeof(mat4)*pMesh->NumInfluence, 1, pFile );
		fwrite( pMesh->chBoneNames, NAME_LEN*pMesh->NumInfluence, 1, pFile );

		WriteMaterialToPSHFile( pMesh->Material, pFile );
		char token;
		if( pMesh->pImage[0] )
		{
			token = 1;
			fwrite( &token, 1, 1, pFile );

			fwrite( pMesh->pImage[0]->img_path, PATH_LEN, 1, pFile );
		}
		else
		{
			token = 0;
			fwrite( &token, 1, 1, pFile );
		}
		if( pMesh->pImage[1] )
		{
			token = 1;
			fwrite( &token, 1, 1, pFile );

			fwrite( pMesh->pImage[1]->img_path, PATH_LEN, 1, pFile );
		}
		else
		{
			token = 0;
			fwrite( &token, 1, 1, pFile );
		}

		WriteSkinMeshToPSHFile( pMesh->pNextMesh, pFile );
	}
	else
	{
		token = 0;
		fwrite( &token, 1, 1, pFile );
	}
}

void WriteMaterialToPSHFile( P3DMaterial& Material, FILE* pFile )
{
	fwrite( Material.ambient, sizeof(float)*4, 1, pFile );
	fwrite( Material.diffuse, sizeof(float)*4, 1, pFile );
	fwrite( Material.emission, sizeof(float)*4, 1, pFile );
	fwrite( Material.specular, sizeof(float)*4, 1, pFile );
	fwrite( &Material.shininess, sizeof(float), 1, pFile );
}

void WriteImagePoolToPSHFile( FILE* pFile )
{
	int PoolSize = (int)g_SkinNodeImagePool.size();
	fwrite( &PoolSize, sizeof(int), 1, pFile );

	for ( int i = 0 ; i < PoolSize ; i++ )
		fwrite( g_SkinNodeImagePool[i]->img_path, PATH_LEN, 1, pFile );
}

void ReadImagePoolFromPSHFile( char* &pBuf );
void ReadFromPSHFile( SkinNode** ppNode, char* &pBuf );
void ReadSkinMeshFromPSHFile( SkinMesh** ppMesh, char* &pBuf );
void ReadMaterialFromPSHFile( P3DMaterial &pMesh, char* &pBuf );

void ReadFromPSHFile( SkinNode** ppRootNode, const char* filepath )
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

		ReadImagePoolFromPSHFile( pBuf );
		ReadFromPSHFile( ppRootNode, pBuf );

		_close( fh );
		SAFE_DELETE_ARRAY( pOrigin );
	}
	
	_unlink( uncompressedFile );
}

void ReadFromPSHFile( SkinNode** ppNode, char* &pBuf )
{
	char token;
	memcpy( &token, pBuf, 1 );
	pBuf++;

	if ( token )
	{
		*ppNode = new SkinNode;

		memcpy( &((*ppNode)->Name), pBuf, NAME_LEN );
		pBuf += NAME_LEN;
		memcpy( &((*ppNode)->Trans), pBuf, sizeof(mat4) );
		pBuf += sizeof(mat4);
		memcpy( &((*ppNode)->WTrans), pBuf, sizeof(mat4) );
		pBuf += sizeof(mat4);

		ReadSkinMeshFromPSHFile( &((*ppNode)->pFirstMesh), pBuf );
		ReadFromPSHFile( &((*ppNode)->pFirstChild), pBuf );
		ReadFromPSHFile( &((*ppNode)->pSibling), pBuf );
	}
	else
	{
		*ppNode = 0;
	}
}

void ReadSkinMeshFromPSHFile( SkinMesh** ppMesh, char* &pBuf )
{
	char token;
	memcpy( &token, pBuf, 1 );
	pBuf++;

	if ( token )
	{
		(*ppMesh) = new SkinMesh;

		memcpy( &((*ppMesh)->NumFaces), pBuf, sizeof(int) );
		pBuf += sizeof(int);
		
		(*ppMesh)->pIndices = new short[(*ppMesh)->NumFaces*3];
		memcpy( (*ppMesh)->pIndices, pBuf, sizeof(short)*((*ppMesh)->NumFaces*3) );
		pBuf += sizeof(short)*((*ppMesh)->NumFaces*3);

		memcpy( &((*ppMesh)->NumVertices), pBuf, sizeof(int) );
		pBuf += sizeof(int);

		(*ppMesh)->pVertices = new vec3[(*ppMesh)->NumVertices];
		memcpy( (*ppMesh)->pVertices, pBuf, sizeof(vec3)*((*ppMesh)->NumVertices) );
		pBuf += sizeof(vec3)*((*ppMesh)->NumVertices);
		
		(*ppMesh)->pNormals = new vec3[(*ppMesh)->NumVertices];
		memcpy( (*ppMesh)->pNormals, pBuf, sizeof(vec3)*((*ppMesh)->NumVertices) );
		pBuf += sizeof(vec3)*((*ppMesh)->NumVertices);

		(*ppMesh)->pTexCoords = new vec2[(*ppMesh)->NumVertices];
		memcpy( (*ppMesh)->pTexCoords, pBuf, sizeof(vec2)*((*ppMesh)->NumVertices) );
		pBuf += sizeof(vec2)*((*ppMesh)->NumVertices);
		
		(*ppMesh)->pWeights = new float[(*ppMesh)->NumVertices*4];
		memcpy( (*ppMesh)->pWeights, pBuf, sizeof(float)*4*((*ppMesh)->NumVertices) );
		pBuf += sizeof(float)*4*((*ppMesh)->NumVertices);
		
		(*ppMesh)->pBoneIds = new unsigned char[(*ppMesh)->NumVertices*4];
		memcpy( (*ppMesh)->pBoneIds, pBuf, 4*((*ppMesh)->NumVertices) );
		pBuf += 4*((*ppMesh)->NumVertices);

		memcpy( &((*ppMesh)->NumBones), pBuf, sizeof(short) );
		pBuf += sizeof(short);

		memcpy( &((*ppMesh)->NumInfluence), pBuf, sizeof(short) );
		pBuf += sizeof(short);

		(*ppMesh)->pBoneOffsetMatrices = new mat4[(*ppMesh)->NumInfluence];
		memcpy( (*ppMesh)->pBoneOffsetMatrices, pBuf, sizeof(mat4)*((*ppMesh)->NumInfluence) );
		pBuf += sizeof(mat4)*((*ppMesh)->NumInfluence);

		(*ppMesh)->chBoneNames = new char[NAME_LEN*((*ppMesh)->NumInfluence)];
		memcpy( (*ppMesh)->chBoneNames, pBuf, NAME_LEN*((*ppMesh)->NumInfluence) );
		pBuf += NAME_LEN*((*ppMesh)->NumInfluence);

		ReadMaterialFromPSHFile( (*ppMesh)->Material, pBuf );

		char token;
		(*ppMesh)->pImage[0] = 0;
		memcpy( &token, pBuf, 1 );
		pBuf++;

		if( token )
		{
			char texfilepath[PATH_LEN];
			memcpy( texfilepath, pBuf, PATH_LEN );
			pBuf += PATH_LEN;

			for ( size_t iImage = 0 ; iImage < g_SkinNodeImagePool.size() ; iImage++ )
			{
				if( strcmp(g_SkinNodeImagePool[iImage]->img_path, texfilepath) == 0 )
					(*ppMesh)->pImage[0] = g_SkinNodeImagePool[iImage];
			}
		}
		
		(*ppMesh)->pImage[1] = 0;
		memcpy( &token, pBuf, 1 );
		pBuf++;
		if( token )
		{
			char texfilepath[PATH_LEN];
			memcpy( texfilepath, pBuf, PATH_LEN );
			pBuf += PATH_LEN;

			for ( size_t iImage = 0 ; iImage < g_SkinNodeImagePool.size() ; iImage++ )
			{
				if( strcmp(g_SkinNodeImagePool[iImage]->img_path, texfilepath) == 0 )
					(*ppMesh)->pImage[1] = g_SkinNodeImagePool[iImage];
			}
		}

		ReadSkinMeshFromPSHFile( &((*ppMesh)->pNextMesh), pBuf );
	}
	else
	{
		(*ppMesh) = 0;
	}
}

void ReadMaterialFromPSHFile( P3DMaterial& Material, char* &pBuf )
{
	memcpy( &Material.ambient, pBuf, sizeof(float)*4 );
	pBuf += sizeof(float)*4;
	memcpy( &Material.diffuse, pBuf, sizeof(float)*4 );
	pBuf += sizeof(float)*4;
	memcpy( &Material.emission, pBuf, sizeof(float)*4 );
	pBuf += sizeof(float)*4;
	memcpy( &Material.specular, pBuf, sizeof(float)*4 );
	pBuf += sizeof(float)*4;
	memcpy( &Material.shininess, pBuf, sizeof(float) );
	pBuf += sizeof(float);
}

void ReadImagePoolFromPSHFile( char* &pBuf )
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
		for ( size_t iImage = 0 ; iImage < g_SkinNodeImagePool.size() ; iImage++ )
		{
			if( strcmp(g_SkinNodeImagePool[iImage]->img_path, pImage->img_path) == 0 )
			{
				delete pImage;
				bInsert = false;
				break;
			}
		}

		if ( bInsert )
			g_SkinNodeImagePool.push_back(pImage);
	}
}

void DrawNode( int skinid, SkinNode* pNode )
{
	if ( pNode == 0 )
		return;

	DrawMesh( skinid, pNode->pFirstMesh, &pNode->WTrans );
	DrawNode( skinid, pNode->pFirstChild );
	DrawNode( skinid, pNode->pSibling );
}

void DrawMesh( int skinId, SkinMesh* pMesh, mat4* pParentWMat )
{
	if ( pMesh == 0 )
		return;

 	if ( pMesh->pOriginVertices )
 	{
 		for ( int vi = 0 ; vi < pMesh->NumVertices ; vi++ )
 		{
 			float tw = 0;
 			int bi = 0;
 			pMesh->pVertices[vi] = vec3_null;
 			vec3 temp;
 			
 			for ( ; bi < pMesh->NumBones-1 ; bi++ )
 			{
 				temp = pMesh->pBoneMatrices[ pMesh->pBoneIds[vi*4+bi] ] * pMesh->pOriginVertices[vi];
 				pMesh->pVertices[vi] += pMesh->pWeights[vi*4+bi] * temp;
 				tw += pMesh->pWeights[vi*4+bi];
 			}
 			
 			temp = pMesh->pBoneMatrices[ pMesh->pBoneIds[vi*4+bi] ] * pMesh->pOriginVertices[vi];
 			pMesh->pVertices[vi] += (1 - tw) * temp;
 		}
 	}
	else
	{
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glMultMatrixf( pParentWMat->mat_array );
	}

	glDisable(GL_TEXTURE_2D );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT , pMesh->Material.ambient );
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION , pMesh->Material.emission );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE , pMesh->Material.diffuse );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR ,  pMesh->Material.specular );
	glMaterialf ( GL_FRONT_AND_BACK , GL_SHININESS , pMesh->Material.shininess );

 	if ( pMesh->pImage[skinId] )
 	{
 		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);

		pMesh->pImage[skinId]->GenGLTextures();

		if ( pMesh->pImage[skinId]->glTexId )
		{
 			glBindTexture ( GL_TEXTURE_2D, pMesh->pImage[skinId]->glTexId );
 			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR);
 			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR);
 			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_REPEAT );
 			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_REPEAT );
		}
 	}

	glEnableClientState ( GL_VERTEX_ARRAY );
	glEnableClientState ( GL_NORMAL_ARRAY );
	glEnableClientState ( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer(3, GL_FLOAT, 0, pMesh->pVertices );
	glNormalPointer(GL_FLOAT, 0, pMesh->pNormals);
	glTexCoordPointer( 2, GL_FLOAT, 0, pMesh->pTexCoords );
	glDrawElements ( GL_TRIANGLES, pMesh->NumFaces * 3 , GL_UNSIGNED_SHORT, pMesh->pIndices );

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState ( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	glDisable( GL_TEXTURE_2D );

	if ( !( pMesh->pOriginVertices ) )
		glPopMatrix( );

	DrawMesh( skinId, pMesh->pNextMesh, pParentWMat );
}

SkinNode* FindNodeByName( SkinNode *pNode, char* name )
{
	if ( pNode == 0 )
		return 0;

	if ( strcmp( pNode->Name, name ) == 0 )
		return pNode;

	SkinNode* pFindNode = FindNodeByName( pNode->pFirstChild, name );
	if ( pFindNode )
		return pFindNode;

	pFindNode = FindNodeByName( pNode->pSibling, name );
	if ( pFindNode )
		return pFindNode;

	return 0;
}

void SetBoneMatrix( SkinNode* pNode, SkinNode*pRoot )
{
	if ( pNode == 0 )
		return;

	SkinMesh* pMesh = pNode->pFirstMesh;
	while( pMesh )
	{
		if( pMesh->NumInfluence )
		{
			pMesh->pOriginVertices = new vec3[ pMesh->NumVertices ];
			memcpy( pMesh->pOriginVertices, pMesh->pVertices, sizeof(vec3)*pMesh->NumVertices );

			pMesh->pBones = new SkinNode*[ pMesh->NumInfluence ];
			pMesh->pBoneMatrices = new mat4[ pMesh->NumInfluence ];
			for ( int iI = 0 ; iI < pMesh->NumInfluence ; iI ++ )
				pMesh->pBones[iI] = FindNodeByName( pRoot, pMesh->chBoneNames+NAME_LEN*iI );				
		}

		pMesh = pMesh->pNextMesh;
	}

	SetBoneMatrix( pNode->pFirstChild, pRoot );
	SetBoneMatrix( pNode->pSibling, pRoot );
}

void UpdateBoneMatrix( SkinNode* pNode )
{
	if ( pNode == 0 )
		return;

	SkinMesh* pMesh = pNode->pFirstMesh;
	while( pMesh )
	{
		for( int iI = 0 ; iI < pMesh->NumInfluence ; iI++ )
			pMesh->pBoneMatrices[iI] = pMesh->pBones[iI]->WTrans * pMesh->pBoneOffsetMatrices[iI];

		pMesh = pMesh->pNextMesh;
	}

	UpdateBoneMatrix( pNode->pFirstChild );
	UpdateBoneMatrix( pNode->pSibling );
}

void CalcBBox( SkinNode* pNode )
{

}

void CalcBBox( SkinMesh* pMesh )
{
}

#ifdef CONV_BVH_TO_PSM
#include "motionData.h"
extern motionData g_Motion;
#endif //CONV_BVH_TO_PSM

void AdvanceMotionMatrix( bool bSkinnig, int nMotionId, int nFrameNum, SkinNode* pNode, mat4 *pParentMat, mat4& OriginMatrix )
{
	if ( pNode == 0 )
		return;

	mat4 newMatrix = pNode->Trans;
	vec3 orgpos;
	pNode->Trans.get_translation( orgpos );

#ifdef CONV_BVH_TO_PSM
	joint* pJonit = g_Motion.getJoint( pNode->Name );

	if ( pJonit && ( strcmp( pNode->Name, pJonit->getName() ) == 0 ) )
	{
		float x = 0 , y = 0 , z = 0 ;

// 		if ( strcmp( pNode->Name, "Hips" ) == 0 )
// 		{
// 			pJonit->getTranslation( nFrameNum, x, y, z );
// 			orgpos.x += x;
// 			orgpos.y += y;
// 			orgpos.z += -z;
// 		}

		if ( pJonit->getRotation( nFrameNum, x, y, z ) )
		{
 			mat4 rotx = mat4_id;
 			mat4 roty = mat4_id;
 			mat4 rotz = mat4_id;
 
 			rotx.set_rot( -ps_to_rad*x, vec3_x );
 			roty.set_rot( -ps_to_rad*y, vec3_y );
 			rotz.set_rot( ps_to_rad*z, vec3_z );

			mat4 ITrans = mat4_id;
			invert( ITrans, OriginMatrix );

			newMatrix = ITrans * rotz * rotx * roty * OriginMatrix * newMatrix;
			newMatrix.set_translation( orgpos );
		}
	}
#else
	if ( pNode->pJoint[nMotionId] && bSkinnig )
	{
// 		if ( nMotionId == MOTION_KNOCKOUT_ID && pNode->pJoint[nMotionId]->channelNum == 6 )
// 		{
// 			orgpos.x += pNode->pJoint[nMotionId]->pos[nFrameNum].x;
// 			orgpos.y += pNode->pJoint[nMotionId]->pos[nFrameNum].y;
// 			orgpos.z += -pNode->pJoint[nMotionId]->pos[nFrameNum].z;
// 		}
		
		mat4 ITrans = mat4_id;
		invert( ITrans, OriginMatrix );
		newMatrix = ITrans * pNode->pJoint[nMotionId]->rot[nFrameNum] * OriginMatrix * newMatrix;
		newMatrix.set_translation( orgpos );
	}

#endif //CONV_BVH_TO_PSM

	if ( pParentMat )
		pNode->WTrans = (*pParentMat) * newMatrix;
	else
		pNode->WTrans = newMatrix;

	mat4 originmat = OriginMatrix * pNode->Trans;

	AdvanceMotionMatrix( bSkinnig, nMotionId, nFrameNum, pNode->pSibling, pParentMat, OriginMatrix );
	AdvanceMotionMatrix( bSkinnig, nMotionId, nFrameNum, pNode->pFirstChild, &pNode->WTrans, originmat );
}

void MatchingJoints( int nMotionId, SkinNode* pNode, PSJoint* pJoints, int nJoints )
{
	if ( pNode == 0 )
		return;

	for ( int j = 0 ; j < nJoints ; j++ )
	{
		if ( strcmp( pNode->Name, pJoints[j].name ) == 0 )
			pNode->pJoint[nMotionId] = pJoints+j;
	}

	MatchingJoints( nMotionId, pNode->pFirstChild, pJoints, nJoints );
	MatchingJoints( nMotionId, pNode->pSibling, pJoints, nJoints );
}

void DestroySkinNodeImages()
{
	for ( size_t i = 0 ; i < g_SkinNodeImagePool.size() ; i++ )
	{
		g_SkinNodeImagePool[i]->DestroyGLTextures();
		SAFE_DELETE( g_SkinNodeImagePool[i] );
	}

	g_SkinNodeImagePool.clear();
}