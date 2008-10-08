#include "X2PSHConv.h"

mat4 ConvertD3DMatrix2GLMatrix( D3DXMATRIX* pD3DMatrix );

void LoadSkinMeshXFile( char* xfilepath, IDirect3DDevice9* pd3dDevice,  D3DXFRAME_DERIVED** ppRoot, ID3DXAnimationController** ppAnimController )
{
	CAllocateHierarchy Alloc;
	D3DXLoadMeshHierarchyFromX( xfilepath, D3DXMESH_MANAGED, pd3dDevice, &Alloc, NULL, (D3DXFRAME**)ppRoot, ppAnimController );
	SetupBoneMatrixPointers( *ppRoot );
	UpdateFrameMatrices( *ppRoot, 0 );
}

void DestroyFrame( D3DXFRAME_DERIVED* pRoot )
{
	CAllocateHierarchy Alloc;
	D3DXFrameDestroy( pRoot, &Alloc );
}

extern char sPrefix[];
extern char sPostfix[];
extern char alterTexture[];

void TrimPrePostfix( char* trimedName, char* name )
{
	if( name )
	{
		char* nodename = (char*)name;
		char* prefix = strstr( nodename, sPrefix );
		if ( prefix )
			nodename = prefix+strlen(sPrefix);
		int namelen = (int)strlen( nodename );

		char* postfix = strstr( nodename, sPostfix );
		if ( postfix )
			namelen = namelen - (int)strlen(sPostfix);

		nodename[namelen] = '\0';

		strncpy( trimedName, nodename, namelen );
		trimedName[namelen] = '\0';
	}
}

SkinNode* ConvertXToSkinNode( D3DXFRAME_DERIVED* pD3DRoot, char* mediapath, IDirect3DDevice9* pd3dDevice, mat4* pParentMatrix )
{
	if ( pD3DRoot == 0 )
		return 0;

	SkinNode* pSkinNodeRoot = new SkinNode;
	
	if( pD3DRoot->Name )
	{
		TrimPrePostfix( pSkinNodeRoot->Name, pD3DRoot->Name );
	}

	pSkinNodeRoot->Trans = ConvertD3DMatrix2GLMatrix( &pD3DRoot->TransformationMatrix );
	if ( pParentMatrix == 0 )
		pSkinNodeRoot->WTrans = pSkinNodeRoot->Trans;
	else
		pSkinNodeRoot->WTrans = (*pParentMatrix) * pSkinNodeRoot->Trans;

	ConvertXMeshToSkinMesh( &pSkinNodeRoot->pFirstMesh, (D3DXMESHCONTAINER_DERIVED*)pD3DRoot->pMeshContainer, mediapath, pd3dDevice );

	pSkinNodeRoot->pSibling = ConvertXToSkinNode( (D3DXFRAME_DERIVED*)pD3DRoot->pFrameSibling, mediapath, pd3dDevice, pParentMatrix );
	pSkinNodeRoot->pFirstChild = ConvertXToSkinNode( (D3DXFRAME_DERIVED*)pD3DRoot->pFrameFirstChild, mediapath, pd3dDevice, &pSkinNodeRoot->WTrans );

	return pSkinNodeRoot;
}

void ConvertXMeshToSkinMesh( SkinMesh** ppSkinMesh, D3DXMESHCONTAINER_DERIVED* pD3DMesh, char* mediapath, IDirect3DDevice9* pd3dDevice )
{
	if ( pD3DMesh == 0 )
	{
		*ppSkinMesh = 0;
		return;
	}

	D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
	pD3DMesh->MeshData.pMesh->GetDeclaration(pDecl);

	LPBYTE *pVerts;
	LPBYTE *pIndics;
	pD3DMesh->MeshData.pMesh->LockVertexBuffer( D3DLOCK_READONLY, (LPVOID*)&pVerts );
	pD3DMesh->MeshData.pMesh->LockIndexBuffer( D3DLOCK_READONLY, (LPVOID*)&pIndics );
	int NumOfVertex = pD3DMesh->MeshData.pMesh->GetNumBytesPerVertex();

	SkinMesh* pSkinMesh = new SkinMesh;
	*ppSkinMesh = pSkinMesh;

	for (DWORD iAttrib = 0 ; iAttrib < pD3DMesh->NumAttributeGroups ; iAttrib++ )
	{
		pSkinMesh->NumFaces = pD3DMesh->pAttributeTable[ iAttrib ].FaceCount;
		pSkinMesh->NumVertices = pD3DMesh->pAttributeTable[ iAttrib ].VertexCount;
		
		pSkinMesh->pVertices = new vec3[pD3DMesh->pAttributeTable[ iAttrib ].VertexCount];
		pSkinMesh->pNormals = new vec3[pD3DMesh->pAttributeTable[ iAttrib ].VertexCount];
		pSkinMesh->pTexCoords = new vec2[pD3DMesh->pAttributeTable[ iAttrib ].VertexCount];

		pSkinMesh->pWeights = new float[4*pD3DMesh->pAttributeTable[ iAttrib ].VertexCount];
		pSkinMesh->pBoneIds = new unsigned char[4*pD3DMesh->pAttributeTable[ iAttrib ].VertexCount];

		if( pD3DMesh->pSkinInfo )
		{
			pSkinMesh->NumBones = (short)pD3DMesh->NumInfl;
			pSkinMesh->NumInfluence = (short)pD3DMesh->pSkinInfo->GetNumBones();
			pSkinMesh->pBoneOffsetMatrices = new mat4[pSkinMesh->NumInfluence];
			pSkinMesh->chBoneNames = new char[NAME_LEN*pSkinMesh->NumInfluence];
			
			for ( int iBone = 0 ; iBone < pSkinMesh->NumInfluence ; iBone++ )
			{
				pSkinMesh->pBoneOffsetMatrices[iBone] = ConvertD3DMatrix2GLMatrix( pD3DMesh->pSkinInfo->GetBoneOffsetMatrix(iBone) );
				TrimPrePostfix( pSkinMesh->chBoneNames+NAME_LEN*iBone, (char*)pD3DMesh->pSkinInfo->GetBoneName( iBone ) );
			}
		}

		LPBYTE* pCurrentPos = pVerts + pD3DMesh->pAttributeTable[ iAttrib ].VertexStart*NumOfVertex/4;
		float* pCurrentFloatPos;
		unsigned char* pCurrentUCharPos;

		for ( DWORD iVert = 0 ; iVert < pD3DMesh->pAttributeTable[ iAttrib ].VertexCount ; iVert++ )
		{
			int iDecl = 0;
			pCurrentFloatPos = (float*)pCurrentPos;
			pSkinMesh->pVertices[iVert].x = SCALE_MODEL*pCurrentFloatPos[0];
 			pSkinMesh->pVertices[iVert].y = SCALE_MODEL*pCurrentFloatPos[1];
 			pSkinMesh->pVertices[iVert].z = SCALE_MODEL*pCurrentFloatPos[2];
			
			if( pD3DMesh->pSkinInfo )
			{
				if ( NumOfVertex >= 40 )
				{
					iDecl++;
					pCurrentPos += (pDecl[iDecl].Offset)/4;
					pCurrentFloatPos = (float*)pCurrentPos;
					for ( WORD iWeight = 0 ; iWeight < (pDecl[2].Offset-pDecl[1].Offset)/sizeof(float) ; iWeight++ )
 						pSkinMesh->pWeights[4*iVert+iWeight] = pCurrentFloatPos[iWeight];
				}
			
				iDecl++;
				pCurrentPos += (pDecl[iDecl].Offset-pDecl[iDecl-1].Offset)/4;
				pCurrentUCharPos = (unsigned char*)pCurrentPos;
				pSkinMesh->pBoneIds[4*iVert+0] = pCurrentUCharPos[0];
  				pSkinMesh->pBoneIds[4*iVert+1] = pCurrentUCharPos[1];
				pSkinMesh->pBoneIds[4*iVert+2] = pCurrentUCharPos[2];
				pSkinMesh->pBoneIds[4*iVert+3] = pCurrentUCharPos[3];
			}

			iDecl++;
			pCurrentPos += (pDecl[iDecl].Offset-pDecl[iDecl-1].Offset)/4;
			pCurrentFloatPos = (float*)pCurrentPos;
			pSkinMesh->pNormals[iVert].x = pCurrentFloatPos[0];
			pSkinMesh->pNormals[iVert].y = pCurrentFloatPos[1];
			pSkinMesh->pNormals[iVert].z = pCurrentFloatPos[2];

			iDecl++;
			pCurrentPos += (pDecl[iDecl].Offset-pDecl[iDecl-1].Offset)/4;
			pCurrentFloatPos = (float*)pCurrentPos;

//			pD3DMesh->pMaterials
			pSkinMesh->pTexCoords[iVert].x = pCurrentFloatPos[0];
			pSkinMesh->pTexCoords[iVert].y = pCurrentFloatPos[1];
			
			pCurrentPos += (NumOfVertex-pDecl[iDecl].Offset)/4;
		}

		pSkinMesh->pIndices = new short[pD3DMesh->pAttributeTable[iAttrib].FaceCount*3];
		memset( pSkinMesh->pIndices, 0, sizeof(pD3DMesh->pAttributeTable[iAttrib].FaceCount*3) );
		
		if ( pD3DMesh->MeshData.pMesh->GetOptions() & D3DXMESH_32BIT )
		{
			for ( DWORD iIndex = 0 ; iIndex < pD3DMesh->pAttributeTable[iAttrib].FaceCount*3 ; iIndex++ )
				pSkinMesh->pIndices[iIndex] = 
				((int*)pIndics)[iIndex+pD3DMesh->pAttributeTable[iAttrib].FaceStart*3] - (short)pD3DMesh->pAttributeTable[iAttrib].VertexStart;
		}
		else
		{
			for ( DWORD iIndex = 0 ; iIndex < pD3DMesh->pAttributeTable[iAttrib].FaceCount*3 ; iIndex++ )
				pSkinMesh->pIndices[iIndex] = ((short*)pIndics)[iIndex+pD3DMesh->pAttributeTable[iAttrib].FaceStart*3] - (short)pD3DMesh->pAttributeTable[iAttrib].VertexStart;
		}

		pSkinMesh->Material.ambient[0] = pD3DMesh->pMaterials[iAttrib].MatD3D.Ambient.r;
		pSkinMesh->Material.ambient[1] = pD3DMesh->pMaterials[iAttrib].MatD3D.Ambient.g;
		pSkinMesh->Material.ambient[2] = pD3DMesh->pMaterials[iAttrib].MatD3D.Ambient.b;
		pSkinMesh->Material.ambient[3] = pD3DMesh->pMaterials[iAttrib].MatD3D.Ambient.a;

		pSkinMesh->Material.diffuse[0] = pD3DMesh->pMaterials[iAttrib].MatD3D.Diffuse.r;
		pSkinMesh->Material.diffuse[1] = pD3DMesh->pMaterials[iAttrib].MatD3D.Diffuse.g;
		pSkinMesh->Material.diffuse[2] = pD3DMesh->pMaterials[iAttrib].MatD3D.Diffuse.b;
		pSkinMesh->Material.diffuse[3] = pD3DMesh->pMaterials[iAttrib].MatD3D.Diffuse.a;

		pSkinMesh->Material.specular[0] = pD3DMesh->pMaterials[iAttrib].MatD3D.Specular.r;
		pSkinMesh->Material.specular[1] = pD3DMesh->pMaterials[iAttrib].MatD3D.Specular.g;
		pSkinMesh->Material.specular[2] = pD3DMesh->pMaterials[iAttrib].MatD3D.Specular.b;
		pSkinMesh->Material.specular[3] = pD3DMesh->pMaterials[iAttrib].MatD3D.Specular.a;
	
		pSkinMesh->Material.emission[0] = pD3DMesh->pMaterials[iAttrib].MatD3D.Emissive.r;
		pSkinMesh->Material.emission[1] = pD3DMesh->pMaterials[iAttrib].MatD3D.Emissive.g;
		pSkinMesh->Material.emission[2] = pD3DMesh->pMaterials[iAttrib].MatD3D.Emissive.b;
		pSkinMesh->Material.emission[3] = pD3DMesh->pMaterials[iAttrib].MatD3D.Emissive.a;

		pSkinMesh->Material.shininess = pD3DMesh->pMaterials[iAttrib].MatD3D.Power;

		pSkinMesh->pImage[0] = 0;
		pSkinMesh->pImage[1] = 0;
		
		if ( pD3DMesh->pMaterials[iAttrib].pTextureFilename && lstrlen( pD3DMesh->pMaterials[iAttrib].pTextureFilename ) > 0)
		{
			//image0
			char texturepath[PATH_LEN];
			strcpy( texturepath, mediapath );
			strcat( texturepath, alterTexture );
			strcat( texturepath, ".jpg" );

			bool bFindInPool = false;
			size_t iImage = 0;
			for ( iImage = 0 ; iImage < g_SkinNodeImagePool.size() ; iImage++ )
			{
				if ( strcmp( g_SkinNodeImagePool[iImage]->img_path, texturepath ) == 0 )
				{
					bFindInPool = true;
					break;
				}
			}
			
			if ( bFindInPool )
				pSkinMesh->pImage[0] = g_SkinNodeImagePool[iImage];
			else
			{
				P3DImage* pP3dImage = new P3DImage;
				g_SkinNodeImagePool.push_back( pP3dImage );
				strcpy( pP3dImage->img_path, texturepath );
				pSkinMesh->pImage[0] = pP3dImage;
			}

			//image1
			strcpy( texturepath, mediapath );
			strcat( texturepath, alterTexture );
			strcat( texturepath, "1.jpg" );

			bFindInPool = false;
			iImage = 0;
			for ( iImage = 0 ; iImage < g_SkinNodeImagePool.size() ; iImage++ )
			{
				if ( strcmp( g_SkinNodeImagePool[iImage]->img_path, texturepath ) == 0 )
				{
					bFindInPool = true;
					break;
				}
			}

			if ( bFindInPool )
				pSkinMesh->pImage[1] = g_SkinNodeImagePool[iImage];
			else
			{
				P3DImage* pP3dImage = new P3DImage;
				g_SkinNodeImagePool.push_back( pP3dImage );
				strcpy( pP3dImage->img_path, texturepath );
				pSkinMesh->pImage[1] = pP3dImage;
			}
		}

		if ( iAttrib < pD3DMesh->NumAttributeGroups-1 )
		{
			pSkinMesh->pNextMesh = new SkinMesh;
			pSkinMesh = pSkinMesh->pNextMesh ;
		}
	}

	pD3DMesh->MeshData.pMesh->UnlockVertexBuffer( );
	pD3DMesh->MeshData.pMesh->UnlockIndexBuffer( );

	ConvertXMeshToSkinMesh( &pSkinMesh->pNextMesh, (D3DXMESHCONTAINER_DERIVED*)pD3DMesh->pNextMeshContainer, mediapath, pd3dDevice );
}

mat4 ConvertD3DMatrix2GLMatrix( D3DXMATRIX* pD3DMatrix )
{
	mat4 glMat;

 	glMat._11 = pD3DMatrix->_11;
 	glMat._12 = pD3DMatrix->_12;
 	glMat._13 = pD3DMatrix->_13;
 	glMat._14 = pD3DMatrix->_14;
 	glMat._21 = pD3DMatrix->_21;
 	glMat._22 = pD3DMatrix->_22;
 	glMat._23 = pD3DMatrix->_23;
 	glMat._24 = pD3DMatrix->_24;
 	glMat._31 = pD3DMatrix->_31;
 	glMat._32 = pD3DMatrix->_32;
 	glMat._33 = pD3DMatrix->_33;
 	glMat._34 = pD3DMatrix->_34;
 	glMat._41 = SCALE_MODEL*pD3DMatrix->_41;
 	glMat._42 = SCALE_MODEL*pD3DMatrix->_42;
 	glMat._43 = SCALE_MODEL*pD3DMatrix->_43;
 	glMat._44 = pD3DMatrix->_44;

	return glMat;
}

PSGemNode* ConvertXToGemNode( D3DXFRAME_DERIVED* pD3DRoot, char* mediapath, IDirect3DDevice9* pd3dDevice, mat4* pParentMatrix, vector< P3DImage* > &imagePool )
{
	if ( pD3DRoot == 0 )
		return 0;

	PSGemNode* pGemNodeRoot = new PSGemNode;

	if( pD3DRoot->Name )
		strcpy( pGemNodeRoot->Name, pD3DRoot->Name );

	pGemNodeRoot->Trans = ConvertD3DMatrix2GLMatrix( &pD3DRoot->TransformationMatrix );
	//if ( pParentMatrix == 0 )
	//{
	//	pGemNodeRoot->Trans._31 *= -1;
	//	pGemNodeRoot->Trans._32 *= -1;
	//	pGemNodeRoot->Trans._33 *= -1;
	//	pGemNodeRoot->Trans._34 *= -1;
	//}
	
	if ( pParentMatrix )
		pGemNodeRoot->WTrans = (*pParentMatrix) * pGemNodeRoot->Trans;
	else
		pGemNodeRoot->WTrans = pGemNodeRoot->Trans;

	bool bIsEqiupNode = false;
	if ( strstr( pGemNodeRoot->Name, BUILDING_UT_NAME) )
		bIsEqiupNode = true;
	if ( strstr( pGemNodeRoot->Name, BUILDING_US_NAME) )
		bIsEqiupNode = true;
	if ( strstr( pGemNodeRoot->Name, BUILDING_IT_NAME) )
		bIsEqiupNode = true;
	if ( strstr( pGemNodeRoot->Name, BUILDING_IS_NAME) )
		bIsEqiupNode = true;

	if ( !bIsEqiupNode )
		ConvertXMeshToGeometry( &pGemNodeRoot->pFirstMesh, (D3DXMESHCONTAINER_DERIVED*)pD3DRoot->pMeshContainer, mediapath, pd3dDevice, imagePool );

	pGemNodeRoot->pSibling = ConvertXToGemNode( (D3DXFRAME_DERIVED*)pD3DRoot->pFrameSibling, mediapath, pd3dDevice, pParentMatrix, imagePool );
	if ( !bIsEqiupNode )
		pGemNodeRoot->pFirstChild = ConvertXToGemNode( (D3DXFRAME_DERIVED*)pD3DRoot->pFrameFirstChild, mediapath, pd3dDevice, &pGemNodeRoot->WTrans, imagePool );

	return pGemNodeRoot;
}

void ConvertXMeshToGeometry( P3DGeometry** ppGeometry, D3DXMESHCONTAINER_DERIVED* pD3DMesh, char* mediapath, IDirect3DDevice9* pd3dDevice, vector< P3DImage* > &imagePool )
{
	if ( pD3DMesh == 0 )
	{
		*ppGeometry = 0;
		return;
	}

	D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
	pD3DMesh->MeshData.pMesh->GetDeclaration(pDecl);

	LPBYTE *pVerts;
	LPBYTE *pIndics;
	pD3DMesh->MeshData.pMesh->LockVertexBuffer( D3DLOCK_READONLY, (LPVOID*)&pVerts );
	pD3DMesh->MeshData.pMesh->LockIndexBuffer( D3DLOCK_READONLY, (LPVOID*)&pIndics );
	int NumOfVertex = pD3DMesh->MeshData.pMesh->GetNumBytesPerVertex();

	P3DGeometry* pGeometry = new P3DGeometry;
	*ppGeometry = pGeometry;

	for (DWORD iAttrib = 0 ; iAttrib < pD3DMesh->NumAttributeGroups ; iAttrib++ )
	{
		pGeometry->m_nNumFace = pD3DMesh->pAttributeTable[ iAttrib ].FaceCount;
		pGeometry->m_nNumVertices = pD3DMesh->pAttributeTable[ iAttrib ].VertexCount;

		pGeometry->m_pVertices = new vec3[pD3DMesh->pAttributeTable[ iAttrib ].VertexCount];
		pGeometry->m_pNormals = new vec3[pD3DMesh->pAttributeTable[ iAttrib ].VertexCount];
		pGeometry->m_pTexCoords = new vec2[pD3DMesh->pAttributeTable[ iAttrib ].VertexCount];

		LPBYTE* pCurrentPos = pVerts + pD3DMesh->pAttributeTable[ iAttrib ].VertexStart*NumOfVertex/4;
		float* pCurrentFloatPos;

		for ( DWORD iVert = 0 ; iVert < pD3DMesh->pAttributeTable[ iAttrib ].VertexCount ; iVert++ )
		{
			int iDecl = 0;
			pCurrentFloatPos = (float*)pCurrentPos;
			pGeometry->m_pVertices[iVert].x = SCALE_MODEL*pCurrentFloatPos[0];
			pGeometry->m_pVertices[iVert].y = SCALE_MODEL*pCurrentFloatPos[1];
			pGeometry->m_pVertices[iVert].z = SCALE_MODEL*pCurrentFloatPos[2];

			iDecl++;
			pCurrentPos += (pDecl[iDecl].Offset-pDecl[iDecl-1].Offset)/4;
			pCurrentFloatPos = (float*)pCurrentPos;
			pGeometry->m_pNormals[iVert].x = pCurrentFloatPos[0];
			pGeometry->m_pNormals[iVert].y = pCurrentFloatPos[1];
			pGeometry->m_pNormals[iVert].z = pCurrentFloatPos[2];

			iDecl++;
			pCurrentPos += (pDecl[iDecl].Offset-pDecl[iDecl-1].Offset)/4;
			pCurrentFloatPos = (float*)pCurrentPos;

			pGeometry->m_pTexCoords[iVert].x = pCurrentFloatPos[0];
			pGeometry->m_pTexCoords[iVert].y = pCurrentFloatPos[1];

			pCurrentPos += (NumOfVertex-pDecl[iDecl].Offset)/4;
		}

		pGeometry->m_pIndices = new short[pD3DMesh->pAttributeTable[iAttrib].FaceCount*3];
		memset( pGeometry->m_pIndices, 0, sizeof(pD3DMesh->pAttributeTable[iAttrib].FaceCount*3) );

		if ( pD3DMesh->MeshData.pMesh->GetOptions() & D3DXMESH_32BIT )
		{
			for ( DWORD iIndex = 0 ; iIndex < pD3DMesh->pAttributeTable[iAttrib].FaceCount*3 ; iIndex++ )
				pGeometry->m_pIndices[iIndex] = 
				((int*)pIndics)[iIndex+pD3DMesh->pAttributeTable[iAttrib].FaceStart*3] - (short)pD3DMesh->pAttributeTable[iAttrib].VertexStart;
		}
		else
		{
			for ( DWORD iIndex = 0 ; iIndex < pD3DMesh->pAttributeTable[iAttrib].FaceCount*3 ; iIndex++ )
				pGeometry->m_pIndices[iIndex] = ((short*)pIndics)[iIndex+pD3DMesh->pAttributeTable[iAttrib].FaceStart*3] - (short)pD3DMesh->pAttributeTable[iAttrib].VertexStart;
		}

		pGeometry->m_Material.ambient[0] = pD3DMesh->pMaterials[iAttrib].MatD3D.Ambient.r;
		pGeometry->m_Material.ambient[1] = pD3DMesh->pMaterials[iAttrib].MatD3D.Ambient.g;
		pGeometry->m_Material.ambient[2] = pD3DMesh->pMaterials[iAttrib].MatD3D.Ambient.b;
		pGeometry->m_Material.ambient[3] = pD3DMesh->pMaterials[iAttrib].MatD3D.Ambient.a;

		pGeometry->m_Material.diffuse[0] = pD3DMesh->pMaterials[iAttrib].MatD3D.Diffuse.r;
		pGeometry->m_Material.diffuse[1] = pD3DMesh->pMaterials[iAttrib].MatD3D.Diffuse.g;
		pGeometry->m_Material.diffuse[2] = pD3DMesh->pMaterials[iAttrib].MatD3D.Diffuse.b;
		pGeometry->m_Material.diffuse[3] = pD3DMesh->pMaterials[iAttrib].MatD3D.Diffuse.a;

		pGeometry->m_Material.specular[0] = pD3DMesh->pMaterials[iAttrib].MatD3D.Specular.r;
		pGeometry->m_Material.specular[1] = pD3DMesh->pMaterials[iAttrib].MatD3D.Specular.g;
		pGeometry->m_Material.specular[2] = pD3DMesh->pMaterials[iAttrib].MatD3D.Specular.b;
		pGeometry->m_Material.specular[3] = pD3DMesh->pMaterials[iAttrib].MatD3D.Specular.a;

		pGeometry->m_Material.emission[0] = pD3DMesh->pMaterials[iAttrib].MatD3D.Emissive.r;
		pGeometry->m_Material.emission[1] = pD3DMesh->pMaterials[iAttrib].MatD3D.Emissive.g;
		pGeometry->m_Material.emission[2] = pD3DMesh->pMaterials[iAttrib].MatD3D.Emissive.b;
		pGeometry->m_Material.emission[3] = pD3DMesh->pMaterials[iAttrib].MatD3D.Emissive.a;

		pGeometry->m_Material.shininess = pD3DMesh->pMaterials[iAttrib].MatD3D.Power;

		pGeometry->m_pImage = 0;

		if ( pD3DMesh->pMaterials[iAttrib].pTextureFilename && lstrlen( pD3DMesh->pMaterials[iAttrib].pTextureFilename ) > 0)
		{
			//image0
			char texturepath[PATH_LEN];
			strcpy( texturepath, mediapath );
			strcat( texturepath, pD3DMesh->pMaterials[iAttrib].pTextureFilename );

			bool bFindInPool = false;
			size_t iImage = 0;
			for ( iImage = 0 ; iImage < imagePool.size() ; iImage++ )
			{
				if ( strcmp( imagePool[iImage]->img_path, texturepath ) == 0 )
				{
					bFindInPool = true;
					break;
				}
			}

			if ( bFindInPool )
				pGeometry->m_pImage = imagePool[iImage];
			else
			{
				P3DImage* pP3dImage = new P3DImage;
				imagePool.push_back( pP3dImage );
				strcpy( pP3dImage->img_path, texturepath );
				pGeometry->m_pImage = pP3dImage;
			}
		}

		if ( iAttrib < pD3DMesh->NumAttributeGroups-1 )
		{
			pGeometry->m_pNextMesh = new P3DGeometry;
			pGeometry = pGeometry->m_pNextMesh ;
		}
	}

	pD3DMesh->MeshData.pMesh->UnlockVertexBuffer( );
	pD3DMesh->MeshData.pMesh->UnlockIndexBuffer( );

	ConvertXMeshToGeometry( &pGeometry->m_pNextMesh, (D3DXMESHCONTAINER_DERIVED*)pD3DMesh->pNextMeshContainer, mediapath, pd3dDevice, imagePool );
}