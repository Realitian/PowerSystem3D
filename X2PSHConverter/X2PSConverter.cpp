#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include "X2PSHConv.h"
#include "PSDefine.h"

LPDIRECT3D9             g_pD3D           = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice     = NULL; // Our rendering device

HRESULT InitD3D( HWND hWnd )
{
    // Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    // Set up the structure used to create the D3DDevice. Since we are now
    // using more complex geometry, we will create a device with a zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // Create the D3DDevice
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

	return S_OK;
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

char sPrefix[64] = "Aragor_";
char sPostfix[64] = "_Aragor";
char alterTexture[64] = "maleSkin";

#include "ArchiScene.h"

//#define CONV_BBOX
void MyMethod()
{
	vector< Geometry* > vecGeos;
	vector< CMaterial* > vecMats;
	vector< CBnsImage* > vecImages;

	LPD3DXBUFFER pMatBuffer = 0;
	LPD3DXMESH pMesh = 0;
	DWORD numMat = 0;

	D3DXLoadMeshFromX( "2_.x", D3DXMESH_MANAGED, g_pd3dDevice, 0, &pMatBuffer, 0, &numMat, &pMesh );

	D3DXMATERIAL* pMaterials = 0;
	if ( pMatBuffer )
		pMaterials = (D3DXMATERIAL*)pMatBuffer->GetBufferPointer();

	D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
	pMesh->GetDeclaration(pDecl);

	LPBYTE *pVerts;
	LPBYTE *pIndics;
	pMesh->LockVertexBuffer( D3DLOCK_READONLY, (LPVOID*)&pVerts );
	pMesh->LockIndexBuffer( D3DLOCK_READONLY, (LPVOID*)&pIndics );
	int NumOfVertex = pMesh->GetNumBytesPerVertex();
	
	D3DXATTRIBUTERANGE* pAttributeRange;
	DWORD nAttTableSize;
	pMesh->GetAttributeTable( 0, &nAttTableSize );
	pAttributeRange = new D3DXATTRIBUTERANGE[nAttTableSize];
	pMesh->GetAttributeTable( pAttributeRange, &nAttTableSize );

#ifdef CONV_BBOX
	{
		int numVertices = pMesh->GetNumVertices();
		D3DXVECTOR3* pVertices = new D3DXVECTOR3[numVertices];
		float* pCurrentFloatPos = (float*)pVerts;
		for ( DWORD iVert = 0 ; iVert < numVertices; iVert++ )
		{
			pVertices[iVert].x = pCurrentFloatPos[0];
			pVertices[iVert].y = pCurrentFloatPos[1];
			pVertices[iVert].z = pCurrentFloatPos[2];
			pCurrentFloatPos += 8;
		}
		D3DXVECTOR3 min, max;
		D3DXComputeBoundingBox( pVertices, numVertices, sizeof(D3DXVECTOR3), &min, &max );
		delete[] pVertices;

		FILE *pGeoFile = fopen( "buld2_.geo", "wb" );
		if ( pGeoFile )
		{	
			fwrite( &min, sizeof( min ), 1, pGeoFile );
			fwrite( &max, sizeof( max ), 1, pGeoFile );
			
			fclose( pGeoFile );
		}
	}
#endif //CONV_BBOX

	for (DWORD iAttrib = 0 ; iAttrib < nAttTableSize ; iAttrib++ )
	{
		Geometry* pGeometry = new Geometry;
		pGeometry->m_bIsCulling = false;

		pGeometry->m_nNumFace = pAttributeRange[ iAttrib ].FaceCount;
		pGeometry->m_nNumVertices = pAttributeRange[ iAttrib ].VertexCount;

		pGeometry->m_pVertices = new float[9*pAttributeRange[ iAttrib ].VertexCount];
		pGeometry->m_pNormals = new float[9*pAttributeRange[ iAttrib ].VertexCount];
		pGeometry->m_pTexCoords = new float[6*pAttributeRange[ iAttrib ].VertexCount];

		LPBYTE* pCurrentPos = pVerts + pAttributeRange[ iAttrib ].VertexStart*NumOfVertex/4;
		float* pCurrentFloatPos;

		for ( DWORD iVert = 0 ; iVert < pAttributeRange[ iAttrib ].VertexCount ; iVert++ )
		{
			int iDecl = 0;
			pCurrentFloatPos = (float*)pCurrentPos;
			pGeometry->m_pVertices[3*iVert] = pCurrentFloatPos[0];
			pGeometry->m_pVertices[3*iVert+1] = pCurrentFloatPos[1];
			pGeometry->m_pVertices[3*iVert+2] = pCurrentFloatPos[2];

			iDecl++;
			pCurrentPos += (pDecl[iDecl].Offset-pDecl[iDecl-1].Offset)/4;
			pCurrentFloatPos = (float*)pCurrentPos;
			pGeometry->m_pNormals[3*iVert] = pCurrentFloatPos[0];
			pGeometry->m_pNormals[3*iVert+1] = pCurrentFloatPos[1];
			pGeometry->m_pNormals[3*iVert+2] = pCurrentFloatPos[2];

			iDecl++;
			pCurrentPos += (pDecl[iDecl].Offset-pDecl[iDecl-1].Offset)/4;
			pCurrentFloatPos = (float*)pCurrentPos;

			pGeometry->m_pTexCoords[2*iVert] = pCurrentFloatPos[0];
			pGeometry->m_pTexCoords[2*iVert+1] = pCurrentFloatPos[1];
								   
			pCurrentPos += (NumOfVertex-pDecl[iDecl].Offset)/4;
		}

		pGeometry->m_pIndices = new short[pAttributeRange[iAttrib].FaceCount*3];
		memset( pGeometry->m_pIndices, 0, sizeof(pAttributeRange[iAttrib].FaceCount*3) );

		if ( pMesh->GetOptions() & D3DXMESH_32BIT )
		{
			for ( DWORD iIndex = 0 ; iIndex < pAttributeRange[iAttrib].FaceCount*3 ; iIndex++ )
				pGeometry->m_pIndices[iIndex] = 
				((int*)pIndics)[iIndex+pAttributeRange[iAttrib].FaceStart*3] - (short)pAttributeRange[iAttrib].VertexStart;
		}
		else
		{
			for ( DWORD iIndex = 0 ; iIndex < pAttributeRange[iAttrib].FaceCount*3 ; iIndex++ )
				pGeometry->m_pIndices[iIndex] = ((short*)pIndics)[iIndex+pAttributeRange[iAttrib].FaceStart*3] - (short)pAttributeRange[iAttrib].VertexStart;
		}

		CMaterial* mat = new CMaterial;

		mat->ambient[0] = 0.7f*pMaterials[iAttrib].MatD3D.Diffuse.r;
		mat->ambient[1] = 0.7f*pMaterials[iAttrib].MatD3D.Diffuse.g;
		mat->ambient[2] = 0.7f*pMaterials[iAttrib].MatD3D.Diffuse.b;
		mat->ambient[3] = 0.7f*pMaterials[iAttrib].MatD3D.Diffuse.a;

		mat->diffuse[0] = pMaterials[iAttrib].MatD3D.Diffuse.r;
		mat->diffuse[1] = pMaterials[iAttrib].MatD3D.Diffuse.g;
		mat->diffuse[2] = pMaterials[iAttrib].MatD3D.Diffuse.b;
		mat->diffuse[3] = pMaterials[iAttrib].MatD3D.Diffuse.a;

		mat->specular[0] = pMaterials[iAttrib].MatD3D.Specular.r;
		mat->specular[1] = pMaterials[iAttrib].MatD3D.Specular.g;
		mat->specular[2] = pMaterials[iAttrib].MatD3D.Specular.b;
		mat->specular[3] = pMaterials[iAttrib].MatD3D.Specular.a;

		mat->emission[0] = pMaterials[iAttrib].MatD3D.Emissive.r;
		mat->emission[1] = pMaterials[iAttrib].MatD3D.Emissive.g;
		mat->emission[2] = pMaterials[iAttrib].MatD3D.Emissive.b;
		mat->emission[3] = pMaterials[iAttrib].MatD3D.Emissive.a;

		mat->shininess = (short)pMaterials[iAttrib].MatD3D.Power;

		mat->pImage = 0;
		
		vecMats.push_back( mat );
		pGeometry->mat = mat;

		if ( pMaterials[iAttrib].pTextureFilename && lstrlen( pMaterials[iAttrib].pTextureFilename ) > 0)
		{
			LPDIRECT3DTEXTURE9 pTexture;
			D3DXCreateTextureFromFile( g_pd3dDevice, pMaterials[iAttrib].pTextureFilename, &pTexture );

			CBnsImage *image = new CBnsImage;
			pGeometry->mat->pImage = image;
			vecImages.push_back( image );

			D3DSURFACE_DESC desc0;
			pTexture->GetLevelDesc(0, &desc0);

			image->img_width = desc0.Width;
			image->img_height = desc0.Height;

			D3DLOCKED_RECT texturerc;
			pTexture->LockRect(0, &texturerc, 0, D3DLOCK_READONLY);

			image->pPixels = new unsigned char[desc0.Height*desc0.Width*4];
			int bpp = texturerc.Pitch / desc0.Width;

			if ( bpp == 4 )
			{
				unsigned char* pBits = (unsigned char*)texturerc.pBits;
				for ( UINT pi = 0 ; pi < desc0.Height*desc0.Width ; pi++ )
				{
					image->pPixels[4*pi] = pBits[4*pi+2];
					image->pPixels[4*pi+1] = pBits[4*pi+1];
					image->pPixels[4*pi+2] = pBits[4*pi];

					if ( desc0.Format == D3DFMT_A8R8G8B8 )
						image->pPixels[4*pi+3] = pBits[4*pi+3];
					else
						image->pPixels[4*pi+3] = 255;
				}
			}
			else if ( bpp == 8 )
			{
				unsigned short* pBits = (unsigned short*)texturerc.pBits;
				for ( UINT pi = 0 ; pi < desc0.Height*desc0.Width ; pi++ )
				{
					image->pPixels[4*pi] = pBits[4*pi+2]/256;
					image->pPixels[4*pi+1] = pBits[4*pi+1]/256;
					image->pPixels[4*pi+2] = pBits[4*pi]/256;
					if ( desc0.Format == D3DFMT_A16B16G16R16 )
						image->pPixels[4*pi+3] = 255;
					else
						image->pPixels[4*pi+3] = pBits[4*pi+3]/256;
				}
			}

			pTexture->UnlockRect(0);
			pTexture->Release();
		}

		vecGeos.push_back( pGeometry );
	}

	pMesh->UnlockVertexBuffer( );
	pMesh->UnlockIndexBuffer( );

	
	pMesh->Release();
	pMatBuffer->Release();

	FILE *pGeoFile = fopen( "buld1_.geo", "wb" );

	if ( pGeoFile )
	{
		size_t images = vecImages.size();
		fwrite( &images, sizeof(size_t), 1, pGeoFile );
		for ( size_t i = 0 ; i < vecImages.size() ;i++)
			vecImages[i]->WriteToFile( pGeoFile );
		
		size_t mats = vecMats.size();
		fwrite( &mats, sizeof(size_t), 1, pGeoFile );
		for ( size_t i = 0 ; i < vecMats.size() ;i++)
			vecMats[i]->WriteToFile( pGeoFile, vecImages );

		size_t geos = vecGeos.size();
		fwrite( &geos, sizeof(size_t), 1, pGeoFile );
  		for ( size_t i = 0 ; i < vecGeos.size() ;i++)
  			vecGeos[i]->WriteToFile( pGeoFile, vecMats );
		fclose( pGeoFile );
	}

	for ( size_t i = 0 ; i < vecImages.size() ;i++)
		delete vecImages[i];

	for ( size_t i = 0 ; i < vecMats.size() ;i++)
		delete vecMats[i];

	for ( size_t i = 0 ; i < vecGeos.size() ;i++)
		delete vecGeos[i];
}

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "X2PSHConverter", NULL };
    RegisterClassEx( &wc );

    // Create the application's window
    HWND hWnd = CreateWindow( "X2PSHConverter", "X2PSHConverter", 
                              WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                              NULL, NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		//MyMethod();

		D3DXFRAME_DERIVED* pRoot = 0;
		ID3DXAnimationController* pAnimController = NULL;
		SkinNode* pSkinNodeRoot;
		PSGemNode* pGemNodeRoot;

		LoadSkinMeshXFile( "1.x", g_pd3dDevice, &pRoot, &pAnimController );
		pSkinNodeRoot = ConvertXToSkinNode( pRoot, "1\\", g_pd3dDevice, 0 );
		WriteToPSHFile( pSkinNodeRoot, "1.psh" );
		SAFE_DELETE( pSkinNodeRoot );
		DestroyFrame( pRoot );
		//pAnimController->Release();

// 		strcpy( sPrefix, "Aragor_" );
// 		strcpy( sPostfix, "_Aragor" );
// 		strcpy( alterTexture, "maleSkin" );
// 
// 		LoadSkinMeshXFile( "psh\\male.x", g_pd3dDevice, &pRoot, &pAnimController );
// 		pSkinNodeRoot = ConvertXToSkinNode( pRoot, "psh\\", g_pd3dDevice, 0 );
// 		WriteToPSHFile( pSkinNodeRoot, MALE_PSH_FILEPATH );
// 		SAFE_DELETE( pSkinNodeRoot );
// 		DestroyFrame( pRoot );
// 		pAnimController->Release();
// 				
// 		strcpy( sPrefix, "Mia_" );
// 		strcpy( sPostfix, "_Mia" );
// 		strcpy( alterTexture, "femaleSkin" );
// 
// 		LoadSkinMeshXFile( "psh\\female.x", g_pd3dDevice, &pRoot, &pAnimController );
// 		pSkinNodeRoot = ConvertXToSkinNode( pRoot, "psh\\", g_pd3dDevice, 0 );
// 		WriteToPSHFile( pSkinNodeRoot, FEMALE_PSH_FILEPATH );
// 		SAFE_DELETE( pSkinNodeRoot );
// 		DestroyFrame( pRoot );
// 		pAnimController->Release();
// 
// 		LoadSkinMeshXFile( "psb\\01.x", g_pd3dDevice, &pRoot, &pAnimController );
// 		pGemNodeRoot = ConvertXToGemNode( pRoot, "psb\\", g_pd3dDevice, 0, g_BuildingImagePool );
// 		WriteToPSFile( pGemNodeRoot, "psb\\01.psb", g_BuildingImagePool);
// 		SAFE_DELETE( pGemNodeRoot );
// 		DestroyFrame( pRoot );
// 
//    		LoadSkinMeshXFile( "psb\\02.x", g_pd3dDevice, &pRoot, &pAnimController );
//   		pGemNodeRoot = ConvertXToGemNode( pRoot, "psb\\", g_pd3dDevice, 0, g_BuildingImagePool );
//   		WriteToPSFile( pGemNodeRoot, "psb\\02.psb", g_BuildingImagePool);
//   		SAFE_DELETE( pGemNodeRoot );
// 		DestroyFrame( pRoot );
// 
// 		LoadSkinMeshXFile( "psb\\03.x", g_pd3dDevice, &pRoot, &pAnimController );
// 		pGemNodeRoot = ConvertXToGemNode( pRoot, "psb\\", g_pd3dDevice, 0, g_BuildingImagePool );
// 		WriteToPSFile( pGemNodeRoot, "psb\\03.psb", g_BuildingImagePool);
// 		SAFE_DELETE( pGemNodeRoot );
// 		DestroyFrame( pRoot );
// 
// 		LoadSkinMeshXFile( "pse\\I-s.x", g_pd3dDevice, &pRoot, &pAnimController );
// 		pGemNodeRoot = ConvertXToGemNode( pRoot, "pse\\", g_pd3dDevice, 0 , g_EquipNodeImagePool );
// 		WriteToPSFile( pGemNodeRoot, "pse\\I-s.pse", g_EquipNodeImagePool );
// 		SAFE_DELETE( pGemNodeRoot );
// 		DestroyFrame( pRoot );
// 
// 		LoadSkinMeshXFile( "pse\\U-s.x", g_pd3dDevice, &pRoot, &pAnimController );
// 		pGemNodeRoot = ConvertXToGemNode( pRoot, "pse\\", g_pd3dDevice, 0, g_EquipNodeImagePool );
// 		WriteToPSFile( pGemNodeRoot, "pse\\U-s.pse", g_EquipNodeImagePool );
// 		SAFE_DELETE( pGemNodeRoot );
// 		DestroyFrame( pRoot );
// 
// 		LoadSkinMeshXFile( "pse\\I-t.x", g_pd3dDevice, &pRoot, &pAnimController );
// 		pGemNodeRoot = ConvertXToGemNode( pRoot, "pse\\", g_pd3dDevice, 0, g_EquipNodeImagePool );
// 		WriteToPSFile( pGemNodeRoot, "pse\\I-t.pse", g_EquipNodeImagePool );
// 		SAFE_DELETE( pGemNodeRoot );
// 		DestroyFrame( pRoot );
// 
// 		LoadSkinMeshXFile( "pse\\U-t.x", g_pd3dDevice, &pRoot, &pAnimController );
// 		pGemNodeRoot = ConvertXToGemNode( pRoot, "pse\\", g_pd3dDevice, 0, g_EquipNodeImagePool );
// 		WriteToPSFile( pGemNodeRoot, "pse\\U-t.pse", g_EquipNodeImagePool );
// 		SAFE_DELETE( pGemNodeRoot );
// 		DestroyFrame( pRoot );
	}

    UnregisterClass( "D3D Tutorial", wc.hInstance );
    return 0;
}



