#include "KDTree.h"
#include "PSGeometry.h"

void BuildGeoFromGemNode( PSGemNode* pNode, vector<vec3> &vecVertices, vector<short> &vecIndices, bool bPropagetChild )
{
	if ( pNode == 0 )
		return;

	P3DGeometry* pMesh = pNode->pFirstMesh;
	
	while ( pMesh )
	{
		for ( int i = 0 ; i < pMesh->m_nNumFace*3 ; i++ )
			vecIndices.push_back( pMesh->m_pIndices[i]+(short)vecVertices.size() );

		for ( int v = 0 ; v < pMesh->m_nNumVertices ; v++ )
		{
			vec3 vert;
			if ( bPropagetChild )
				vert = pNode->WTrans * pMesh->m_pVertices[v];
			else
				vert = pMesh->m_pVertices[v];

			vecVertices.push_back( vert );
		}

		pMesh = pMesh->m_pNextMesh;
	}

	if ( bPropagetChild )
	{
		BuildGeoFromGemNode( pNode->pFirstChild, vecVertices, vecIndices, bPropagetChild );
		BuildGeoFromGemNode( pNode->pSibling, vecVertices, vecIndices, bPropagetChild );
	}
}

void BuildGeoFromGemNode( PSGemNode* pRoot, Geometry *pGeo, bool bPropagetChild )
{
	vector<vec3> vecVertices;
	BuildGeoFromGemNode( pRoot, vecVertices, pGeo->m_vecIndices, bPropagetChild );

	pGeo->m_nVeritces = (int)vecVertices.size();
	pGeo->m_pVertices = new float[pGeo->m_nVeritces*3];
	for ( int v = 0 ; v < pGeo->m_nVeritces ; v++ )
	{
		pGeo->m_pVertices[3*v] = vecVertices[v].x;
		pGeo->m_pVertices[3*v+1] = vecVertices[v].y;
		pGeo->m_pVertices[3*v+2] = vecVertices[v].z;
	}
}

 void Render( float* pVertices, int nIndices, short* pIndics )
 {
 	glEnableClientState ( GL_VERTEX_ARRAY );
 	glEnableClientState ( GL_NORMAL_ARRAY );
 	glEnableClientState ( GL_TEXTURE_COORD_ARRAY );
 
 	glVertexPointer(3, GL_FLOAT, 0, pVertices );
 	glDrawElements ( GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, pIndics );
 
 	glDisableClientState(GL_VERTEX_ARRAY);
 	glDisableClientState ( GL_NORMAL_ARRAY );
 	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
 }

 void RenderKdNode( KdNode* pNode )
 {
	 if ( pNode == 0 )
		 return;

	 if ( pNode->m_AxixId == LEAF )
	 {
		 Geometry* pGeo = (Geometry*)pNode->m_pChildren;

		 glDisable( GL_TEXTURE_2D );
		 glDisable( GL_LIGHTING );
		 glEnable( GL_BLEND );
		 glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		 glColor4f( 0.5f+pGeo->r, 0.5f+pGeo->g, 0.5f+pGeo->b, 0.1f );

		 for ( size_t f = 0 ; f < pGeo->m_vecIndices.size() / 3 ; f++ )
		 {
			 glBegin( GL_TRIANGLES );

			 glVertex3fv( pGeo->m_pVertices + 3*pGeo->m_vecIndices[3*f] );
			 glVertex3fv( pGeo->m_pVertices + 3*pGeo->m_vecIndices[3*f+1] );
			 glVertex3fv( pGeo->m_pVertices + 3*pGeo->m_vecIndices[3*f+2] );

			 glEnd();
		 }

		 glEnable( GL_LIGHTING );
		 glEnable( GL_TEXTURE_2D );
	 }
	 else
	 {
		 RenderKdNode( pNode->m_pChildren );
		 RenderKdNode( pNode->m_pChildren+1 );
	 }
 }