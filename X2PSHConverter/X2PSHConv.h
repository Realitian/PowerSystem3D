#include "D3DSkinMesh.h"
#include "PSSkining.h"
#include "PSGeometry.h"

void LoadSkinMeshXFile( char* xfilepath, IDirect3DDevice9* pd3dDevice, D3DXFRAME_DERIVED** ppRoot, ID3DXAnimationController** ppAnimController );
SkinNode* ConvertXToSkinNode( D3DXFRAME_DERIVED* pD3DRoot, char* mediapath, IDirect3DDevice9* pd3dDevice, mat4* pParentMatrix );
void ConvertXMeshToSkinMesh( SkinMesh** pSkinMesh, D3DXMESHCONTAINER_DERIVED* pD3DMesh, char* mediapath, IDirect3DDevice9* pd3dDevice );
void DestroyFrame( D3DXFRAME_DERIVED* pRoot );

PSGemNode* ConvertXToGemNode( D3DXFRAME_DERIVED* pD3DRoot, char* mediapath, IDirect3DDevice9* pd3dDevice, mat4* pParentMatrix, vector< P3DImage* > &imagePool );
void ConvertXMeshToGeometry( P3DGeometry** pSkinMesh, D3DXMESHCONTAINER_DERIVED* pD3DMesh, char* mediapath, IDirect3DDevice9* pd3dDevice, vector< P3DImage* > &imagePool );