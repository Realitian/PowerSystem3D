#include "stdafx.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "PSHumen.h"
#include "PSGame.h"
#include "PSSkining.h"

PSHumen::PSHumen()
{
	m_bLoaded = false;
	m_pSkinNode[0] = 0;
	m_pSkinNode[1] = 0;
	m_Pos = vec3_null;
	m_fAngleY = 0;
	m_iSkinId = 0;
	m_iMotionId = MOTION_STAND_ID;
	m_iSexId = 0;
}

PSHumen::~PSHumen()
{
	DestroySkinNodeImages();
	SAFE_DELETE( m_pSkinNode[0] );
	SAFE_DELETE( m_pSkinNode[1] );
}

void PSHumen::Idle( float fElapsedTime )
{
	if ( !m_bLoaded )
		return;

	static float fKnockOutTime = 0.0f;

	switch ( g_PSGame.m_EquipStage )
	{
	case EQUIP_OP_START:
		fKnockOutTime = 0;
	case EQUIP_OP_OPERATING:
		m_iMotionId = MOTION_STAND_ID;
		break;
	}

	if ( g_PSGame.m_Scene.m_pCurEquip && ( g_PSGame.m_Scene.m_pCurEquip->m_state == EQUIP_STATE_EXPLOSION /*|| (g_PSGame.m_Scene.m_pCurEquip->m_state == EQUIP_STATE_SMOKE && g_PSGame.m_Scene.m_pCurEquip->m_fTime >= SMOKE_TIME - 0.9f )*/ ) )
	{
		mat4 matIdentity = mat4_id;
		
		int nFrameNum = (int)( fKnockOutTime / m_Motions[MOTION_KNOCKOUT_ID].fFrameInterval);
		if ( nFrameNum > m_Motions[MOTION_KNOCKOUT_ID].nFrameNum-1 )
			nFrameNum = m_Motions[MOTION_KNOCKOUT_ID].nFrameNum-1;

		AdvanceMotionMatrix( true, MOTION_KNOCKOUT_ID, nFrameNum, m_pSkinNode[m_iSexId], 0, matIdentity );
		UpdateBoneMatrix( m_pSkinNode[m_iSexId] );

		if ( fKnockOutTime < m_Motions[MOTION_KNOCKOUT_ID].nFrameNum * m_Motions[MOTION_KNOCKOUT_ID].fFrameInterval )
			fKnockOutTime += fElapsedTime;

		return;
	}

	switch ( g_PSGame.m_GameStage )
	{
	case SELECT_HUMEN:
		{
			if ( !m_bRotating )
				m_fAngleY += m_RotSign * fElapsedTime;
			mat4 matIdentity = mat4_id;
			AdvanceMotionMatrix( false, 0, 0, m_pSkinNode[m_iSexId], 0, matIdentity );
			UpdateBoneMatrix( m_pSkinNode[m_iSexId] );
		}
		break;
	case SHOW_PREVIEW:
	case OVERVIEW_SCENE:
		{
			Stand();
			mat4 matIdentity = mat4_id;

			int nFrameNum = (int)( g_PSGame.m_fElapsedFrameTime / m_Motions[m_iMotionId].fFrameInterval);
			nFrameNum %= m_Motions[m_iMotionId].nFrameNum;
			AdvanceMotionMatrix( true, m_iMotionId, nFrameNum, m_pSkinNode[m_iSexId], 0, matIdentity );
			UpdateBoneMatrix( m_pSkinNode[m_iSexId] );
		}
		break;
	case EXITING:
		{
			mat4 matIdentity = mat4_id;

			int nFrameNum = (int)( g_PSGame.m_fElapsedFrameTime / m_Motions[m_iMotionId].fFrameInterval);
			nFrameNum %= m_Motions[m_iMotionId].nFrameNum;
			AdvanceMotionMatrix( true, m_iMotionId, nFrameNum, m_pSkinNode[m_iSexId], 0, matIdentity );
			UpdateBoneMatrix( m_pSkinNode[m_iSexId] );

			//vec3_z : back->front dir.
			if ( m_Pos.z > g_PSGame.m_Building.m_StartPos.z )
			{
				Stand();
				g_PSGame.m_Building.CloseDoor();
				m_Pos = g_PSGame.m_Building.m_StartPos;
				g_PSGame.m_Camera.m_eyeDir = vec3_neg_z;
				m_fAngleY = 0;
				g_PSGame.m_Camera.m_eyePos = g_PSGame.m_Building.m_StartPos + HUMEN_EYE_DIST_MAX * vec3_z + METER * vec3_y;
			}

			if ( m_iMotionId == MOTION_WALK_ID )
			{
				m_Pos.x -= sinf( m_fAngleY ) * WALK_STEP * fElapsedTime;
				m_Pos.z -= cosf( m_fAngleY ) * WALK_STEP * fElapsedTime;
			}
		}
		break;
	case STAND:
		{
			mat4 matIdentity = mat4_id;

			int nFrameNum = (int)( g_PSGame.m_fElapsedFrameTime / m_Motions[m_iMotionId].fFrameInterval);
			nFrameNum %= m_Motions[m_iMotionId].nFrameNum;
			AdvanceMotionMatrix( true, m_iMotionId, nFrameNum, m_pSkinNode[m_iSexId], 0, matIdentity );
			UpdateBoneMatrix( m_pSkinNode[m_iSexId] );

			if ( m_Pos.z < g_PSGame.m_Building.m_StartPos.z-3*METER )
			{
				Stand();
				
				if ( g_PSGame.m_bToPlay )
				{
					g_PSGame.m_GameStage = PLAYING;
					g_PSGame.InitState();
					g_PSGame.m_bToPlay = false;
				}
				else
				{
					g_PSGame.m_GameStage = NAVIGATING;
					g_PSGame.RecordState();
				}

				g_PSGame.m_Building.CloseDoor();
				SetCursorPos( g_PSGame.m_Camera.m_SCX, g_PSGame.m_Camera.m_SCY );
			}

			if ( m_iMotionId == MOTION_WALK_ID )
			{
				m_Pos.x -= sinf( m_fAngleY ) * WALK_STEP * fElapsedTime;
				m_Pos.z -= cosf( m_fAngleY ) * WALK_STEP * fElapsedTime;
			}
		}
		break;
	case NAVIGATING:
	case PLAYING:
		{
			mat4 matIdentity = mat4_id;

			int nFrameNum = (int)( g_PSGame.m_fElapsedFrameTime / m_Motions[m_iMotionId].fFrameInterval);
			nFrameNum %= m_Motions[m_iMotionId].nFrameNum;
			AdvanceMotionMatrix( true, m_iMotionId, nFrameNum, m_pSkinNode[m_iSexId], 0, matIdentity );
			UpdateBoneMatrix( m_pSkinNode[m_iSexId] );

			if ( m_iMotionId == MOTION_WALK_ID )
				Forward( -sinf( m_fAngleY ) * WALK_STEP * fElapsedTime, -cosf( m_fAngleY ) * WALK_STEP * fElapsedTime );

			if ( m_iMotionId == MOTION_RUN_ID )
				Forward( -sinf( m_fAngleY ) * RUN_STEP * fElapsedTime, -cosf( m_fAngleY ) * RUN_STEP * fElapsedTime );
		}
		break;
	}
}

void PSHumen::Forward( float dx, float dz )
{
	PSLineSeg lsg;
	lsg.start = m_Pos;
	lsg.dir = vec3( dx, 0, dz );
	lsg.dir.normalize();
	lsg.start = m_Pos + METER * vec3_y;
	lsg.dist = HUMEN_OBJECT_DIST;

	float fDist = TestCollision( g_PSGame.m_Building.m_pGemNodeRoot, lsg );

	CPSEquipment* pInterEquip = g_PSGame.m_Scene.TestCollisionBB( lsg );
	
	if ( g_PSGame.m_GameStage == PLAYING )
	{
		vec3 humenPos;
		bool bExiting = false;
		bExiting = g_PSGame.m_HumenRecordManager.GetHumenPos( humenPos, g_PSGame.m_fElapsedFrameTime );
		if ( !bExiting )
		{
			m_Pos.x = humenPos.x;
			m_Pos.z = humenPos.z;
		}
	}
	else if ( fDist > HUMEN_OBJECT_DIST && pInterEquip == 0 )
	{
		m_Pos.x += dx;
		m_Pos.z += dz;
	}
}

void PSHumen::Render( PSStage stage )
{
	if ( !m_bLoaded )
		return;

	if ( g_PSGame.m_EquipStage != EQUIP_OP_NONE )
	{
		if ( g_PSGame.m_Scene.m_pCurEquip->m_state < EQUIP_STATE_EXPLOSION )
			return;
	}

	switch ( stage )
	{
	case SHOW_PREVIEW:
		if ( !g_PSGame.m_bLoaded )
			break;
	case SELECT_HUMEN:
		{
			glPushAttrib( GL_ALL_ATTRIB_BITS );

			glViewport( 0, -(GLint)(g_PSGame.m_Camera.m_SCX*2*0.1f), g_PSGame.m_Camera.m_SCX*2*4/10, (GLsizei)(g_PSGame.m_Camera.m_SCY*2*0.98) );

			float vp = 0.8f;
			float aspect = ((float)(g_PSGame.m_Camera.m_SCX*2*4/10)) / ((float)(g_PSGame.m_Camera.m_SCY*2*0.98));

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glFrustum(-vp, vp, -vp / aspect, vp / aspect, 3, 10*METER);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			glTranslatef( 0, HUMEN_INIT_Y_DIFF, HUMEN_INIT_Z_DIFF );

			if ( m_iSexId )
				glScalef( 1.75f/1.6f, 1.75f/1.6f, 1.75f/1.6f );
			glRotatef( ps_to_deg * m_fAngleY, 0, 1, 0);
			DrawNode( m_iSkinId, m_pSkinNode[m_iSexId] );

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();

			glMatrixMode( GL_PROJECTION );
			glPopMatrix();

			glPopAttrib();
		}
		break;

	case OVERVIEW_SCENE:
	case NAVIGATING:
	case PLAYING:
	case STAND:
	case EXITING:
		glPushMatrix();
		glTranslatef( m_Pos.x, m_Pos.y, m_Pos.z );
		glRotatef( ps_to_deg * m_fAngleY, 0, 1, 0);
		DrawNode( m_iSkinId, m_pSkinNode[m_iSexId] );
		glPopMatrix();
		break;
	}
}

void PSHumen::Walk()
{
	if ( m_iMotionId != MOTION_WALK_ID )
		m_iMotionId = MOTION_WALK_ID;
}

void PSHumen::Run()
{
	if ( m_iMotionId != MOTION_RUN_ID )
		m_iMotionId = MOTION_RUN_ID;
}

void PSHumen::Stand()
{
	if ( m_iMotionId != MOTION_STAND_ID )
		m_iMotionId = MOTION_STAND_ID;
}

void PSHumen::Knokout()
{
	m_iMotionId = MOTION_KNOCKOUT_ID;
}

void PSHumen::SelectSkin( int id )
{
	m_iSkinId = id;
}

void PSHumen::SelectSex( bool bMale )
{
	if ( bMale )
		m_iSexId = 0;
	else
		m_iSexId = 1;
}

void PSHumen::Load( )
{
	if ( m_bLoaded )
		return;

	CString apppath = GetAppDirectory() + "\\";

	m_Motions[MOTION_WALK_ID].ReadFromPSMFile( apppath + MOTION_WALK_PATH );
	m_Motions[MOTION_RUN_ID].ReadFromPSMFile( apppath + MOTION_RUN_PATH );
	m_Motions[MOTION_STAND_ID].ReadFromPSMFile( apppath + MOTION_STAND_PATH );
	m_Motions[MOTION_KNOCKOUT_ID].ReadFromPSMFile( apppath + MOTION_KNOCKOUT_PATH );

	ReadFromPSHFile( &m_pSkinNode[0], apppath + MALE_PSH_FILEPATH );
	ReadFromPSHFile( &m_pSkinNode[1], apppath + FEMALE_PSH_FILEPATH );
	
	SetBoneMatrix( m_pSkinNode[0], m_pSkinNode[0] );
	SetBoneMatrix( m_pSkinNode[1], m_pSkinNode[1] );
	
	for ( int m = 0 ; m < MOTION_COUNT ; m++ )
	{
		MatchingJoints( m, m_pSkinNode[0], m_Motions[m].pJoints, m_Motions[m].nJointNum );
		MatchingJoints( m, m_pSkinNode[1], m_Motions[m].pJoints, m_Motions[m].nJointNum );
	}

	m_bLoaded = true;
}

void PSHumen::RotateStart( int x )
{
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	if ( g_PSGame.m_GameStage == SELECT_HUMEN && x > viewport[2]*4/10 )
		return;

	m_RotX = x;
	m_bRotating = true;
}

void PSHumen::Rotate( int x )
{
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	int cx = viewport[2]*2/10;

	if ( g_PSGame.m_GameStage == SELECT_HUMEN && x > viewport[2]*4/10 )
		return;
	
	m_RotSign = 0.1f*(x-m_RotX);
	m_fAngleY += ps_half_pi*(x-m_RotX)/(float)cx;
	m_RotX = x;
}

void PSHumen::RotateEnd( int x )
{
	m_bRotating = false;
}

void PSHumen::RotateY( int x )
{
	static int prex = x;
	if ( x < g_PSGame.m_Camera.m_SCX/2 || x > g_PSGame.m_Camera.m_SCX*2 - g_PSGame.m_Camera.m_SCX/2 )
	{
		POINT pt;
		GetCursorPos( &pt );
		SetCursorPos( g_PSGame.m_Camera.m_SCX, pt.y );
		prex = g_PSGame.m_Camera.m_SCX;
		return;
	}

	m_fAngleY += ps_pi * ( prex - x ) / (float)g_PSGame.m_Camera.m_SCX;
	prex = x;
}