#include <stdafx.h>
#include <Windows.h>
#include <gl/GL.h>
#include "PSGame.h"
#include "PSBuilding.h"
#include "PSGeometry.h"

CPSBuilding::CPSBuilding()
{
	m_pGemNodeRoot = 0;
	m_pGeoRoot = 0;
	m_pGeoIndices = 0;
	m_pLeftDoorNode = 0;
	m_pRightDoorNode = 0;
	m_DoorStatus = CLOSED;
	m_DoorOpenedWidth = 0;

	m_Radius = INIT_RADIUS;
}

CPSBuilding::~CPSBuilding()
{
	SAFE_DELETE( m_pGemNodeRoot );
	
	if ( m_pGeoRoot )
	{
		delete[] m_pGeoRoot->m_pVertices;
		delete m_pGeoRoot;
	}
	SAFE_DELETE( m_KdTree );

	SAFE_DELETE_ARRAY ( m_pGeoIndices );
}

void CPSBuilding::Unload()
{
	for ( size_t i = 0 ; i < g_BuildingImagePool.size() ; i++ )
	{
		g_BuildingImagePool[i]->DestroyGLTextures();
		SAFE_DELETE( g_BuildingImagePool[i] );
	}
	g_BuildingImagePool.clear();

	SAFE_DELETE( m_pGemNodeRoot );

	m_pLeftDoorNode = 0;
	m_pRightDoorNode = 0;
	m_pStandPointNode = 0;

	m_DoorStatus = CLOSED;
	m_Radius = INIT_RADIUS;
}

void CPSBuilding::Load( const char* psbfilepath )
{
	Unload();
	ReadFromPSFile( &m_pGemNodeRoot, psbfilepath, g_BuildingImagePool );
	if ( m_pGeoRoot )
	{
		delete[] m_pGeoRoot->m_pVertices;
		delete m_pGeoRoot;
	}
	SAFE_DELETE_ARRAY ( m_pGeoIndices );
	
	m_pGeoRoot = new Geometry;
	BuildGeoFromGemNode( m_pGemNodeRoot, m_pGeoRoot, true );
	m_pGeoIndices = new short[m_pGeoRoot->m_vecIndices.size()];
	for (size_t i = 0 ; i < m_pGeoRoot->m_vecIndices.size() ; i++)
		m_pGeoIndices[i] = m_pGeoRoot->m_vecIndices[i];

	SAFE_DELETE( m_KdTree );
	m_KdTree = new KdTree(10);
 	m_KdTree->BuildKdTree( m_pGeoRoot );

	m_pLeftDoorNode = FindNodeByName( m_pGemNodeRoot, BUILDING_LEFT_DOOR_NAME );
	m_pRightDoorNode = FindNodeByName( m_pGemNodeRoot, BUILDING_RIGHT_DOOR_NAME );
	
	m_LeftDoorMat = m_pLeftDoorNode->Trans;
	m_RightDoorMat = m_pRightDoorNode->Trans;

	m_pStandPointNode = FindNodeByName( m_pGemNodeRoot, BUILDING_STANDPOINT_NAME );
	m_StartPos = 0.5f*(m_pStandPointNode->bbMax + m_pStandPointNode->bbMin);

	vec3 bbBoxMin( FLT_MAX, FLT_MAX, FLT_MAX ), bbBoxMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	GetNodeBBox( m_pGemNodeRoot, bbBoxMin, bbBoxMax );
	m_Center = 0.5f*(bbBoxMax + bbBoxMin);
	vec3 diff = bbBoxMax - bbBoxMin;
	m_Radius = diff.norm();

	vec3 bbBoxMinInside( FLT_MAX, FLT_MAX, FLT_MAX ), bbBoxMaxInside( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	GetNodeBBoxMap( m_pGemNodeRoot, bbBoxMinInside, bbBoxMaxInside );
	m_CenterInside = 0.5f*(bbBoxMaxInside + bbBoxMinInside);
	diff = bbBoxMaxInside - bbBoxMinInside;
	m_RadiusInside = diff.norm();
	m_InsideMin = bbBoxMinInside;
	m_InsideMax = bbBoxMaxInside;

	vec3 DoorBoxMin( FLT_MAX, FLT_MAX, FLT_MAX ), DoorBoxMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	GetNodeBBox( m_pLeftDoorNode, DoorBoxMin, DoorBoxMax );
	m_DoorWidth = fabsf( DoorBoxMax.x - DoorBoxMin.x );
}

void Render( float* pVertices, int nIndices, short* pIndics );
void RenderKdNode( KdNode* pNode );

void CPSBuilding::Render( bool bMap, bool bOpaque )
{
	if ( g_PSGame.m_bLoaded )
	{
		RenderNode( bMap, m_pGemNodeRoot, bOpaque );
		//::Render( m_pGeoRoot->m_pVertices, m_pGeoRoot->m_vecIndices.size(), m_pGeoIndices );
		//RenderKdNode( m_KdTree->m_Root );
	}
}

void CPSBuilding::Idle( float fElapsedTime )
{
	if ( m_DoorStatus == OPENING )
	{
		mat4 LeftTran = mat4_id;
		LeftTran.set_translation( vec3(-m_DoorOpenedWidth, 0, 0) );
		m_pLeftDoorNode->Trans = m_LeftDoorMat * LeftTran;
		
		mat4 RightTran = mat4_id;
		RightTran.set_translation( vec3(m_DoorOpenedWidth, 0, 0) );
		m_pRightDoorNode->Trans = m_RightDoorMat * RightTran;
		
		m_DoorOpenedWidth += m_DoorWidth * fElapsedTime / BUILDING_DOOR_OPENNING_TIME;
		if ( m_DoorOpenedWidth >= m_DoorWidth )
		{
			m_DoorStatus = OPENED;
			g_PSGame.m_Humen.Walk();
		}
	}

	if ( m_DoorStatus == CLOSING )
	{
		mat4 LeftTran = mat4_id;
		LeftTran.set_translation( vec3(-m_DoorOpenedWidth, 0, 0) );
		m_pLeftDoorNode->Trans = m_LeftDoorMat * LeftTran;

		mat4 RightTran = mat4_id;
		RightTran.set_translation( vec3(m_DoorOpenedWidth, 0, 0) );
		m_pRightDoorNode->Trans = m_RightDoorMat * RightTran;

		m_DoorOpenedWidth -= m_DoorWidth * fElapsedTime / BUILDING_DOOR_OPENNING_TIME;
		if ( m_DoorOpenedWidth <= 0 )
		{
			m_DoorStatus = CLOSED;
			m_pLeftDoorNode->Trans = m_LeftDoorMat;
			m_pRightDoorNode->Trans = m_RightDoorMat;
			if ( g_PSGame.m_GameStage == EXITING )
			{
				g_PSGame.m_bEnded = true;
				if ( !g_PSGame.m_bExportAvi )
					g_PSGame.m_GameStage = STAND;
			}
		}
	}
}


void CPSBuilding::OpenDoor( )
{
	if ( m_DoorStatus == CLOSED )
		m_DoorStatus = OPENING;
}

void CPSBuilding::CloseDoor( )
{
	if ( m_DoorStatus == OPENED )
		m_DoorStatus = CLOSING;
}

void CPSBuilding::RenderNode( bool bMap, PSGemNode* pNode, bool bOpaque )
{
	if ( pNode == 0 )
		return;

	bool bShowChildren = true;
	if ( bMap )
	{
		if ( strstr( pNode->Name, BUILDING_ROOF_NAME ) || strstr( pNode->Name, BUILDING_OUTSIDE_NAME ) )
			bShowChildren = false;
	}

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glMultMatrixf( pNode->Trans.mat_array );
	
	if ( bShowChildren && pNode != m_pStandPointNode )
	{
		RenderGeo( pNode->pFirstMesh, bOpaque );
		RenderNode( bMap, pNode->pFirstChild, bOpaque );
	}
	
	glPopMatrix();
	RenderNode( bMap, pNode->pSibling, bOpaque );
}

void CPSBuilding::RenderGeo( P3DGeometry* pGeo, bool bOpaque )
{
	if ( pGeo == 0 )
		return;

	glDisable(GL_TEXTURE_2D );

	bool bTransparent = ( pGeo->m_Material.ambient[3] < 1 ) || ( pGeo->m_Material.emission[3] < 1 ) || ( pGeo->m_Material.diffuse[3] < 1 ) || ( pGeo->m_Material.specular[3] < 1 );
	
	if ( bOpaque )
	{
		if ( bTransparent )
			return;
		glDisable( GL_BLEND );
		glDepthMask( GL_TRUE );
	}
	else
	{
		if ( !bTransparent )
			return;
		glDepthMask( GL_FALSE );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}
	
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT , pGeo->m_Material.ambient );
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION , pGeo->m_Material.emission );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE , pGeo->m_Material.diffuse );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR ,  pGeo->m_Material.specular );
	glMaterialf ( GL_FRONT_AND_BACK , GL_SHININESS , pGeo->m_Material.shininess );

	if ( pGeo->m_pImage )
	{
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);

		pGeo->m_pImage->GenGLTextures();

		if ( pGeo->m_pImage->glTexId )
		{
			glBindTexture ( GL_TEXTURE_2D, pGeo->m_pImage->glTexId );
			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR);
			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_REPEAT );
			glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_REPEAT );
		}
	}

	glEnableClientState ( GL_VERTEX_ARRAY );
	glEnableClientState ( GL_NORMAL_ARRAY );
	glEnableClientState ( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer(3, GL_FLOAT, 0, pGeo->m_pVertices );
	glNormalPointer(GL_FLOAT, 0, pGeo->m_pNormals);
	glTexCoordPointer( 2, GL_FLOAT, 0, pGeo->m_pTexCoords );
	glDrawElements ( GL_TRIANGLES, pGeo->m_nNumFace * 3 , GL_UNSIGNED_SHORT, pGeo->m_pIndices );

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState ( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

#ifdef SHOW_BBOX
	glLineWidth(1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(1, 0, 0);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMin.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMin.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMax.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMax.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMin.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMin.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMax.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMax.y, pGeo->m_bbMin.z );

	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMin.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMin.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMin.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMin.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMin.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMin.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMin.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMin.y, pGeo->m_bbMin.z );

	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMin.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMax.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMax.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMin.y, pGeo->m_bbMin.z );
	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMin.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMin.x, pGeo->m_bbMax.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMax.y, pGeo->m_bbMax.z );
	glVertex3f( pGeo->m_bbMax.x, pGeo->m_bbMin.y, pGeo->m_bbMax.z );

	glEnd(); 
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

	RenderGeo( pGeo->m_pNextMesh, bOpaque );
}