#include "stdafx.h"
#include "PSGame.h"
#include "MainFrm.h"
#include "DlgRenderConfig.h"
#ifdef SUPPORT_HTTP
#include "Global.h"
#include "HttpClient.h"
#include "DlgDownLoad.h"
#include "DlgUpLoad.h"
#include "PS3DViewer.h"
#endif SUPPORT_HTTP

PSGame g_PSGame;
PSGame::PSGame()
{
	m_GameStage = INIT;
	m_EquipStage = EQUIP_OP_NONE;
	m_nHelpState = HELP_NONE;
	m_nAviFps = 24;
	m_bToPlay = false;
	m_bExportAvi = false;

	ZeroMemory(&m_aviOpts, sizeof m_aviOpts);
	m_aviOpts.fccType = streamtypeVIDEO;
	m_aviOpts.dwKeyFrameEvery = (DWORD) -1; // Default
	m_aviOpts.dwQuality = (DWORD) ICQUALITY_DEFAULT;
	m_aviOpts.dwFlags = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES;
	m_aviOpts.dwInterleaveEvery = 1;
	m_pFrameBuffer = 0;
}

PSGame::~PSGame()
{
	m_GameStage = EXIT;
	m_Scene.UnLoadEquip();
	m_Building.Unload();
	if(m_aviOpts.lpParms)
		free(m_aviOpts.lpParms);
	m_aviOpts.lpParms = NULL;

	SAFE_DELETE(m_pFrameBuffer);

	if(m_pVideoMgr)
		m_pVideoMgr->CloseAVIVideo();
	
	if(m_bExportAvi)
		DeleteFile( m_szAviPath );

	SAFE_DELETE( m_pVideoMgr );
}

void PSGame::CalcCursorPoint(CPSEquipActionRecord* pFirst, CPSEquipActionRecord* pSecond, float fCurrentTime, CPoint* pPoint)
{
	if(pFirst == NULL || pPoint == NULL)
		return;

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	float curX = 0.f;
	float curY = 0.f;

	if(pSecond == NULL)
	{
		curX = pFirst->fMouseX;
		curY = pFirst->fMouseY;
	}
	else
	{
		float fTotalTime = pSecond->fTimeAt - pFirst->fTimeAt;
		float fElapsedTime = fCurrentTime - pFirst->fTimeAt;
		float fLerp = fElapsedTime / fTotalTime;
		curX = pFirst->fMouseX * (1.f-fLerp) + pSecond->fMouseX * fLerp;
		curY = pFirst->fMouseY * (1.f-fLerp) + pSecond->fMouseY * fLerp;
	}

 	pPoint->x = (LONG)(curX * (float)viewport[2]);
 	pPoint->y = (LONG)(curY * (float)viewport[3]);
}
void PSGame::Idle( float fElapsedTime )
{
	if ( g_PSGame.m_nHelpState != HELP_NONE )
		return;

	if ( g_PSGame.m_Scene.m_pCurEquip )
	{
		static bool bDamaged = false;
		switch( g_PSGame.m_Scene.m_pCurEquip->m_state )
		{
		case EQUIP_STATE_EXPLOSION:
			if ( g_PSGame.m_Scene.m_pCurEquip->m_fTime < EXPLOSION_TIME - 1.0f )
			{
				m_fFade = 0.5f - g_PSGame.m_Scene.m_pCurEquip->m_fTime / EXPLOSION_TIME;
				if ( m_fFade < 0 )
					m_fFade = 0;
			}
			if ( g_PSGame.m_Scene.m_pCurEquip->m_fTime >= EXPLOSION_TIME - 1.0f )
			{
				m_fFade += fElapsedTime;
				if ( m_fFade >= 1.0f )
					m_fFade = 1.0f;
				bDamaged = true;
			}

			break;
		case EQUIP_STATE_INIT:
			if ( bDamaged )
			{
				if ( m_fFade < 1.0f )
					m_fFade += fElapsedTime;
				
				if ( m_fFade >= 1.0f )
				{
					m_fFade = 1.0f;
					g_PSGame.m_Scene.m_pCurEquip->m_fTime += fElapsedTime;
				}
			}

			if ( g_PSGame.m_Scene.m_pCurEquip->m_fTime > 0.5f )
			{
				g_PSGame.m_Scene.m_pCurEquip->m_fTime = 0.5f;
				bDamaged = false;
			}

			if ( !bDamaged )
				m_fFade -= 0.2f*fElapsedTime;
			
			if ( m_fFade < 0.0f )
				m_fFade = 0.0f;
			break;
		}
	}

	bool bExiting = false;

	static CPSEquipActionRecord equipPrevAction = {0.f, EQUIP_ACTION_NONE, 0.f, 0.f, 0, 0.f};
	static CPSEquipActionRecord equipNextAction = {0.f, EQUIP_ACTION_NONE, 0.f, 0.f, 0, 0.f};

	CPoint point(0, 0);

	if( g_PSGame.m_GameStage == PLAYING )
	{
		CPSHumenActionRecord humenAction;
		bExiting = m_HumenRecordManager.GetAction( humenAction, m_fElapsedFrameTime );
		if ( !bExiting )
		{
			m_Humen.m_iMotionId = humenAction.iMotionId;
			m_Humen.m_fAngleY = humenAction.fAngleY;
			m_Camera.m_fAngleX = humenAction.fAngleX;
		}

		CPSEquipActionRecord equipAction1, equipAction2;
		int nActionNum = m_EquipRecordManager.GetAction( equipAction1, equipAction2, m_fElapsedFrameTime );
		if ( nActionNum > 0 )
		{
			memset(&equipPrevAction, 0x00, sizeof(equipPrevAction));
			memset(&equipNextAction, 0x00, sizeof(equipNextAction));

			equipPrevAction = equipAction1;
			if(nActionNum == 2)
				equipNextAction = equipAction2;

			switch ( equipAction1.opType )
			{
			case EQUIP_ACTION_STARTOP:
				m_Scene.StartOperateWithEquip();
				break;
			case EQUIP_ACTION_ENDOP:
				m_Scene.EndOperateWithEquip();
				break;
 			case WM_KEYDOWN:
 				CalcCursorPoint(&equipAction1, NULL, m_fElapsedFrameTime, &point);
 				m_Scene.OnMouseMove(0, point);
				m_Scene.SetCtrlValue(equipAction1.opType, equipAction1.iCtrlID, equipAction1.fCtrlValue);
 				break;
 			case WM_LBUTTONDOWN:
 				CalcCursorPoint(&equipAction1, NULL, m_fElapsedFrameTime, &point);
 				m_Scene.OnLButtonDown(0, point);
				m_Scene.SetCtrlValue(equipAction1.opType, equipAction1.iCtrlID, equipAction1.fCtrlValue);
 				break;
 			case WM_LBUTTONUP:
 				CalcCursorPoint(&equipAction1, NULL, m_fElapsedFrameTime, &point);
 				m_Scene.OnLButtonUp(0, point);
				m_Scene.SetCtrlValue(equipAction1.opType, equipAction1.iCtrlID, equipAction1.fCtrlValue);
 				break;
 			case WM_MOUSEMOVE:
 				CalcCursorPoint(&equipAction1, NULL, m_fElapsedFrameTime, &point);
 				m_Scene.OnMouseMove(MK_LBUTTON, point);
				m_Scene.SetCtrlValue(equipAction1.opType, equipAction1.iCtrlID, equipAction1.fCtrlValue);
 				break;
			}
		}
		else
		{
			switch(equipNextAction.opType)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_MOUSEMOVE:
				{
					CalcCursorPoint(&equipPrevAction, &equipNextAction, m_fElapsedFrameTime, &point);
					if(equipNextAction.opType == WM_MOUSEMOVE)
						m_Scene.OnMouseMove(MK_LBUTTON, point);
					else
						m_Scene.OnMouseMove(0, point);
				}
				break;
			}
		}
	}
	else
	{
		memset(&equipPrevAction, 0x00, sizeof(equipPrevAction));
		memset(&equipNextAction, 0x00, sizeof(equipNextAction));
	}

	m_Camera.Idle( fElapsedTime );
	m_Building.Idle( fElapsedTime );
	m_Scene.Idle(fElapsedTime);
	m_Humen.Idle( fElapsedTime );

	if ( g_PSGame.m_GameStage == NAVIGATING )
	{
		CPSHumenActionRecord record;
		record.fTimeAt = m_fElapsedFrameTime;
		record.iMotionId = m_Humen.m_iMotionId;
		record.fAngleX = m_Camera.m_fAngleX;
		record.fAngleY = m_Humen.m_fAngleY;
		record.fPosX = m_Humen.m_Pos.x;
		record.fPosZ = m_Humen.m_Pos.z;
		m_HumenRecordManager.PushAction( record );
	}

	if ( bExiting )
		g_PSGame.GoExiting();

	m_fElapsedFrameTime += fElapsedTime;
	
	if ( !m_bExportAvi && m_bCalcExportFrameTime )
		m_fExportFrameTime += fElapsedTime;
}

void PSGame::NewScenario()
{
	assert( m_GameStage == INIT || m_GameStage == STAND );
	m_GameStage = SELECT_SCENE;
	m_fElapsedFrameTime = 0;
	m_bLoaded = false;
}

void PSGame::SelectScene()
{
	assert( m_GameStage == SELECT_HUMEN );
	m_GameStage = SELECT_SCENE;
	m_fElapsedFrameTime = 0;
}

void PSGame::SelectHumen()
{
	CloseHandle ( CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)LoadHumenData, 0, 0, 0 ) );

	assert( m_GameStage == SELECT_SCENE || m_GameStage == SHOW_PREVIEW );
	m_Humen.m_RotSign = 1.0f;
	m_Humen.m_fAngleY = 0;
	m_Humen.m_bRotating = false;

	m_GameStage = SELECT_HUMEN;
}

void PSGame::ShowPreview()
{
	m_GameStage = SHOW_PREVIEW;
}

void PSGame::SaveScenario( const char* path )
{
	FILE* pFile = fopen( path, "w+b" );
	if ( pFile )
	{
		fwrite ( &m_fExportFrameTime, sizeof(float), 1, pFile );
		fwrite ( &m_Humen.m_iSexId, sizeof(int), 1, pFile );
		fwrite ( &m_Humen.m_iSkinId, sizeof(int), 1, pFile );

		int BuildingId = 0;
		if ( m_stageMgr.GetSelScene() == HITID_BUILDING1 )
			BuildingId = 1;
		if ( m_stageMgr.GetSelScene() == HITID_BUILDING2 )
			BuildingId = 2;
		if ( m_stageMgr.GetSelScene() == HITID_BUILDING3 )
			BuildingId = 3;
		
		fwrite ( &BuildingId, sizeof(int), 1, pFile );

		m_HumenRecordManager.SaveToFile( pFile );
		m_EquipRecordManager.SaveToFile( pFile );

		fclose( pFile );
	}
}

void PSGame::OpenScenario( const char* path )
{
	ShowPreview();

	m_Humen.Load();

	FILE* pFile = fopen( path, "rb" );
	if ( pFile )
	{
		fread ( &m_fExportFrameTime, sizeof(float), 1, pFile );
		fread ( &m_Humen.m_iSexId, sizeof(int), 1, pFile );
		fread ( &m_Humen.m_iSkinId, sizeof(int), 1, pFile );

		int BuildingId = 0;
		fread  ( &BuildingId, sizeof(int), 1, pFile );

		switch ( BuildingId )
		{
		case 1:
			m_stageMgr.SetScene(HITID_BUILDING1);
			break;
		case 2:
			m_stageMgr.SetScene(HITID_BUILDING2);
			break;
		case 3:
			m_stageMgr.SetScene(HITID_BUILDING3);
			break;
		}

		m_HumenRecordManager.ReadFromFile( pFile );
		m_EquipRecordManager.ReadFromFile( pFile );

		fclose( pFile );

		LoadScenario(0);
	}
}

void PSGame::StartScenario( )
{
	m_GameStage = OVERVIEW_SCENE;
	m_fElapsedFrameTime = 0;
	m_Humen.m_Pos = m_Building.m_StartPos;
	m_Humen.m_fAngleY = 0;
}

void PSGame::UnLoadAll( )
{
	m_Building.Unload();
	m_Scene.UnLoadEquip();
}

void PSGame::LoadScenario( LOADINGCALLBACK pLoadingProc )
{
	m_bLoaded = false;
	
	CString path = GetAppDirectory() + "\\";
	if ( m_stageMgr.GetSelScene() == HITID_BUILDING1 )
		m_Building.Load( path + BUILDING1_PATH );
	if ( m_stageMgr.GetSelScene() == HITID_BUILDING2 )
		m_Building.Load( path + BUILDING2_PATH );
	if ( m_stageMgr.GetSelScene() == HITID_BUILDING3 )
		m_Building.Load( path + BUILDING3_PATH );
	
	m_Scene.LoadEquipment();

	m_Humen.m_fAngleY = -ps_pi / 3.0f;
	m_bLoaded = true;
}

void PSGame::ScenarioLoaded()
{

}

void PSGame::ReadyToPlay()
{
	m_GameStage = STAND;
}

void PSGame::RecordState()
{
	m_fElapsedFrameTime = 0;
	m_HumenRecordManager.m_InitHumenPos = m_Humen.m_Pos;
}

void PSGame::InitState()
{
	m_fElapsedFrameTime = 0;
	m_Humen.m_Pos = m_HumenRecordManager.m_InitHumenPos;
	m_EquipRecordManager.m_iCurActionId = 0;
}

void PSGame::PlayScenario()
{
	m_bToPlay = true;
	if ( m_bExportAvi )
		m_fExportedTime = 0;
	EnterIndoor();
}

void PSGame::MainMenu()
{
	m_GameStage = INIT;
}

void PSGame::Quit()
{
	PostMessage( AfxGetMainWnd()->m_hWnd, WM_CLOSE, 0, 0 );
}

bool PSGame::IsEmptyScenario()
{
	return m_HumenRecordManager.m_vecHumenActions.empty();
}

void PSGame::ExportToAvi()
{
	CDlgRenderConfig dlg;
	dlg.m_nFps = m_nAviFps;
	dlg.m_aviPath = m_szAviPath;
	dlg.m_aviOpts = m_aviOpts;

	if(dlg.DoModal() == IDOK)
	{
		m_szAviPath = dlg.m_aviPath;
		m_nAviFps = dlg.m_nFps;
		m_aviOpts = dlg.m_aviOpts;
		m_bExportAvi = true;

		SAFE_DELETE( m_pVideoMgr );
		CSize size(m_nWidth, m_nHeight);
		m_pVideoMgr = new CAVIVideo(m_szAviPath, size, m_nAviFps, m_aviOpts);
		m_pVideoMgr->CreateAVIVideo();

		PlayScenario();
	}
}

void PSGame::GLSetupRC( )
{
	m_stageMgr.LoadTexture();
	CPSEquipment::LoadTextures();
}

void PSGame::RenderNormal()
{
	switch ( m_GameStage )
	{
	case EXITING:
	case SHOW_PREVIEW:
	case OVERVIEW_SCENE:
	case STAND:
	case NAVIGATING:
	case PLAYING:
		{
			glPushAttrib( GL_ALL_ATTRIB_BITS );

			glShadeModel( GL_SMOOTH );
			glEnable( GL_CULL_FACE );
			glDisable( GL_CULL_FACE );
			glFrontFace( GL_CW );
			glEnable( GL_DEPTH_TEST );

			glMatrixMode( GL_PROJECTION );
			glPushMatrix();
			glLoadMatrixf( m_Camera.getProjectMat().mat_array );

			glMatrixMode( GL_MODELVIEW );
			glPushMatrix();
			glLoadMatrixf( m_Camera.getViewMat().mat_array );

			GLSetupLight();
			m_Building.Render( false, true );
			m_Scene.Render( false );
			m_Humen.Render( m_GameStage );
			m_Building.Render( false, false );

			glMatrixMode( GL_MODELVIEW );
			glPopMatrix();
			glMatrixMode( GL_PROJECTION );
			glPopMatrix();
			glPopAttrib( );
		}
		break;
	case SELECT_HUMEN:
		{
			glPushAttrib( GL_ALL_ATTRIB_BITS );

			glShadeModel( GL_SMOOTH );
			glEnable( GL_CULL_FACE );
			glDisable( GL_CULL_FACE );
			glFrontFace( GL_CW );
			glEnable( GL_DEPTH_TEST );

			glMatrixMode( GL_PROJECTION );
			glPushMatrix();
			glLoadMatrixf( m_Camera.getProjectMat().mat_array );

			glMatrixMode( GL_MODELVIEW );
			glPushMatrix();
			glLoadMatrixf( m_Camera.getViewMat().mat_array );

			GLSetupLight();
			m_Humen.Render( m_GameStage );

			glMatrixMode( GL_MODELVIEW );
			glPopMatrix();
			glMatrixMode( GL_PROJECTION );
			glPopMatrix();
			glPopAttrib( );
		}
		break;
	}
}

void PSGame::RenderFade()
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	glColor4f( 0, 0, 0, 0 );
	if ( g_PSGame.m_Scene.m_pCurEquip )
	{
		switch( g_PSGame.m_Scene.m_pCurEquip->m_state )
		{
		case EQUIP_STATE_EXPLOSION:
			if ( g_PSGame.m_Scene.m_pCurEquip->m_fTime < EXPLOSION_TIME - 1.0f )
				glColor4f( 1, 0, 0, m_fFade );
			else if ( g_PSGame.m_Scene.m_pCurEquip->m_fTime >= EXPLOSION_TIME - 1.0f )
				glColor4f( 0, 0, 0, m_fFade );
			break;
		case EQUIP_STATE_INIT:
				glColor4f( 0, 0, 0, m_fFade );
			break;
		}
		
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_LIGHTING );

		glBegin(GL_QUADS);

		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f((GLfloat)m_nWidth, 0.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f((GLfloat)m_nWidth, (GLfloat)m_nHeight);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.0f, (GLfloat)m_nHeight);
		glEnd();
	}

	glPopAttrib( );
}

void PSGame::RenderMap()
{
	glClear(GL_DEPTH_BUFFER_BIT);
	
	if ( m_EquipStage == EQUIP_OP_NONE && (m_GameStage == NAVIGATING || m_GameStage == PLAYING) )
	{
		glPushAttrib( GL_ALL_ATTRIB_BITS );

		glShadeModel( GL_SMOOTH );
		glEnable( GL_CULL_FACE );
		glDisable( GL_CULL_FACE );
		glFrontFace( GL_CW );
		glEnable( GL_LIGHTING );
		glEnable( GL_LIGHT0 );

		glEnable( GL_DEPTH_TEST );

		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		glViewport( viewport[2] - viewport[2]/3, viewport[3] - viewport[3]/3, viewport[2]/3, viewport[3]/3 );

		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadMatrixf( m_Camera.getMapProjectMat().mat_array );

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadMatrixf( m_Camera.getMapViewMat().mat_array );

		glLineWidth(2);
		glColor3f(1, 1, 0);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		glBegin(GL_LINES);
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMax.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMax.y, m_Building.m_InsideMin.z );
		glEnd(); 

		glBegin(GL_LINES);
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMax.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMax.y, m_Building.m_InsideMin.z );
		glEnd(); 

		glBegin(GL_LINES);
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMax.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMax.y, m_Building.m_InsideMin.z );
		glEnd(); 

		glBegin(GL_LINES);
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMax.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMax.y, m_Building.m_InsideMax.z );
		glEnd(); 

		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);

#ifdef SHOW_MAP_BBBOX
		glLineWidth(1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glColor3f(1, 0, 0);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);

		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMin.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMin.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMax.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMax.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMin.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMin.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMax.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMax.y, m_Building.m_InsideMin.z );

		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMin.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMin.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMin.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMin.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMin.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMin.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMin.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMin.y, m_Building.m_InsideMin.z );

		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMin.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMax.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMax.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMin.y, m_Building.m_InsideMin.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMin.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMin.x, m_Building.m_InsideMax.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMax.y, m_Building.m_InsideMax.z );
		glVertex3f( m_Building.m_InsideMax.x, m_Building.m_InsideMin.y, m_Building.m_InsideMax.z );

		glEnd(); 
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
		m_Building.Render( true, true );
		m_Scene.Render( true );
		
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );
		glColor3f( 1, 0, 0 );
		glPointSize( 8 );
		glBegin( GL_POINTS );
		glVertex3f( m_Humen.m_Pos.x, m_Humen.m_Pos.y+METER, m_Humen.m_Pos.z );
		glEnd();
		glEnable( GL_LIGHTING );
		glEnable( GL_TEXTURE_2D );

		m_Building.Render( true, false );

		glMatrixMode( GL_MODELVIEW );
		glPopMatrix();
		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		glViewport( viewport[0], viewport[1], viewport[2], viewport[3] );
		glPopAttrib( );
	}
}

void PSGame::GLSetupLight()
{
	glEnable( GL_LIGHTING );
	glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

	vec4 pos = m_Camera.m_eyePos;
	pos.w = 1;

	static vec4 vec4_7( 0.7f, 0.7f, 0.7f, 1.0f );
	static vec4 vec4_3( 0.3f, 0.3f, 0.3f, 1.0f );

	glLightfv( GL_LIGHT0, GL_POSITION, pos.vec_array );
	glLightfv( GL_LIGHT0, GL_AMBIENT, vec4_7.vec_array );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, vec4_7.vec_array );
	glLightfv( GL_LIGHT0, GL_SPECULAR, vec4_7.vec_array );
	glEnable( GL_LIGHT0 );

	glLightfv( GL_LIGHT1, GL_POSITION, vec4_y.vec_array );
	glLightfv( GL_LIGHT1, GL_AMBIENT, vec4_3.vec_array );
	glLightfv( GL_LIGHT1, GL_DIFFUSE, vec4_3.vec_array );
	glLightfv( GL_LIGHT1, GL_SPECULAR, vec4_3.vec_array );
	glEnable( GL_LIGHT1 );
}

void PSGame::GLRenderProc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 	m_stageMgr.RenderBackground();
	glClear(GL_DEPTH_BUFFER_BIT);

	RenderNormal();
	if ( !m_bExportAvi )
		RenderMap();

 	RenderFade();

	if ( !m_bExportAvi )
	{
		if ( m_bEnded )
		{
			ShowCursor( TRUE );
			m_bCalcExportFrameTime = false;
			m_bEnded = false;
			m_fElapsedFrameTime = 0;
		}
	}

	glClear(GL_DEPTH_BUFFER_BIT);
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, m_nWidth, 0, m_nHeight);
	m_stageMgr.Render();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();

	if ( !m_bExportAvi && m_nHelpState != HELP_NONE )
	{
		PSStage oldStage = m_GameStage;
		m_GameStage = HELP;
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		glDisable( GL_DEPTH_TEST );
		m_stageMgr.Render();
		glEnable( GL_DEPTH_TEST );
		glPopAttrib();
		m_GameStage = oldStage;
	}
}

void PSGame::GLResize(int cx, int cy)
{
	m_Camera.m_SCY = cy / 2;
	m_Camera.m_SCX = cx / 2;

	// Viewport
	if(cy == 0)
		cy = 1;
	glViewport(0, 0, cx, cy);

	switch(m_GameStage)
	{
	case INIT:
	case SELECT_SCENE:
	case SELECT_HUMEN:
	case SHOW_PREVIEW:
	case STAND:
		{
			m_stageMgr.Resize(cx, cy);
		}
		break;
	case NAVIGATING:
	case PLAYING:
		break;
	default:
		break;
	}

	glMatrixMode(GL_MODELVIEW);
}

void PSGame::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ( m_GameStage == SELECT_HUMEN )
		m_Humen.RotateStart( point.x );

	m_stageMgr.OnLButtonDown(nFlags, point);

	if ( m_EquipStage == EQUIP_OP_OPERATING )
		m_Scene.OnLButtonDown(nFlags, point);
}

void PSGame::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_stageMgr.OnRButtonDown(nFlags, point);
}

void PSGame::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ( m_GameStage == SELECT_HUMEN )
		m_Humen.RotateEnd( point.x );

	m_stageMgr.OnLButtonUp(nFlags, point);
	m_Scene.OnLButtonUp(nFlags, point);
}

void PSGame::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( m_nHelpState != HELP_NONE )
		return;

 	if ( m_GameStage == OVERVIEW_SCENE || m_GameStage == PLAYING )
 		return; 

	if ( m_GameStage == SELECT_HUMEN && ( nFlags & MK_LBUTTON ) && m_Humen.m_bLoaded )
		m_Humen.Rotate( point.x );
	
	if ( m_GameStage == NAVIGATING && m_EquipStage == EQUIP_OP_NONE )
	{
		m_Humen.RotateY( point.x );
		m_Camera.RotateX( point.y );
	}

	if ( m_GameStage == NAVIGATING && m_EquipStage == EQUIP_OP_OPERATING )
		m_Scene.OnMouseMove(nFlags, point);

	m_stageMgr.OnMouseMove(nFlags, point);
}

void PSGame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(m_nHelpState == HELP_NONE)
	{
		if ( m_GameStage == NAVIGATING )
		{
			if ( nChar == KEY_RUN )
				m_Humen.Run();

			if ( nChar == KEY_WALK )
				m_Humen.Walk();
		}

		m_Scene.OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void PSGame::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
 	if ( nChar == VK_ESCAPE && m_GameStage == OVERVIEW_SCENE )
		m_Camera.SetStand();

	if(m_nHelpState == HELP_NONE)
	{
		if ( m_GameStage == NAVIGATING )
		{
			if ( nChar == KEY_RUN )
				m_Humen.Stand();

			if ( nChar == KEY_WALK )
				m_Humen.Stand();
		}

		if ( m_GameStage == NAVIGATING )
		{
			if ( nChar == 'H' )
			{
				if ( m_EquipStage != EQUIP_OP_NONE )
				{
					m_nHelpState = HELP_EQUIPU;
				}
				else
					m_nHelpState = HELP_DEFAULT;
			}
		}
		
		if ( nChar == VK_SPACE && m_GameStage == STAND )
		{
			EnterIndoor();
		}

		if ( nChar == VK_ESCAPE && ( m_GameStage == NAVIGATING || m_GameStage == PLAYING ) )
			CancelPlayScenario();
		
		if ( nChar == VK_SPACE && m_GameStage == NAVIGATING )
		{
			if ( m_EquipStage == EQUIP_OP_NONE )
			{
				GoExiting();
				m_Scene.StartOperateWithEquip();
				return;
			}
			if ( m_EquipStage == EQUIP_OP_OPERATING )
			{
				m_Scene.EndOperateWithEquip();
				return;
			}
		}
	}
	else
	{
		switch(nChar)
		{
		case VK_ESCAPE:
		case 'H':
			m_nHelpState = HELP_NONE;
			break;
		default:
			break;
		}
	}
}

void PSGame::CancelPlayScenario()
{
	if ( m_EquipStage != EQUIP_OP_NONE )
		return;

	ShowCursor( TRUE );
	char message[64] = "Do you really cancel Play Scenario?";
	if ( m_bExportAvi )
		strcpy( message, "Do you really cancel Export Scenario?" );
	if ( m_GameStage == NAVIGATING )
		strcpy( message, "Do you really cancel making Scenario?" );

	if ( IDYES == AfxMessageBox( message, MB_YESNO ) )
	{
		if ( m_GameStage == NAVIGATING )
		{
			m_HumenRecordManager.Init();
			m_EquipRecordManager.Init();
			m_Scene.ResetEquips();
		}

		m_GameStage = STAND;
		m_bEnded = true;
		m_Humen.m_Pos = m_Building.m_StartPos;
		m_Humen.m_fAngleY = 0;
		m_Humen.m_iMotionId = MOTION_STAND_ID;

		if ( m_bExportAvi )
		{
			m_fExportedTime = 0; 
			m_bExportAvi = false;
			((CMainFrame*)AfxGetMainWnd())->SetStatusBarText( "Ending Export..." );

			SAFE_DELETE(m_pFrameBuffer);

			if(m_pVideoMgr)
				m_pVideoMgr->CloseAVIVideo();

			 DeleteFile( m_szAviPath );
		}
	}

	ShowCursor( FALSE );
}

void PSGame::EnterIndoor()
{
	if ( m_Building.m_DoorStatus != CPSBuilding::CLOSED )
		return;

	if ( !m_bToPlay )
	{
		if ( !m_bExportAvi )
		{
			m_fExportedTime = 0; 
			m_bExportAvi = false;
			((CMainFrame*)AfxGetMainWnd())->SetStatusBarText( "Ending Export..." );

			SAFE_DELETE(m_pFrameBuffer);

			if(m_pVideoMgr)
				m_pVideoMgr->CloseAVIVideo();
		}
		m_fExportFrameTime = 0;
		m_bCalcExportFrameTime = true;
		m_HumenRecordManager.Init();
		m_EquipRecordManager.Init();
	}

	m_Scene.ResetEquips();
	m_Building.OpenDoor();
	SetCursorPos( m_Camera.m_SCX, m_Camera.m_SCY );
	ShowCursor( FALSE );
}

void PSGame::GoExiting()
{
	m_Humen.Stand();

	g_pIntersectedNode = 0;
	g_pIntersectedGeo = 0;

	PSLineSeg lsg;
	lsg.start = m_Humen.m_Pos + METER * vec3_y;
	lsg.dir = - m_Camera.m_eyePos + lsg.start;
	lsg.dist = OPEN_DOOR_DIST;
	lsg.dir.normalize();

	float fRDist = TestCollision( g_PSGame.m_Building.m_pGemNodeRoot, lsg );
	if ( fRDist < OPEN_DOOR_DIST && ( IsChildNode( m_Building.m_pLeftDoorNode, g_pIntersectedNode ) || IsChildNode( m_Building.m_pRightDoorNode, g_pIntersectedNode ) ) )
	{
		m_GameStage = EXITING;
		m_Building.OpenDoor();
	}
}

void PSGame::OnInitialize()
{		
	_chdir( GetAppDirectory() );
	_mkdir( "psb" );
	_mkdir( "pse" );
	_mkdir( "psh" );
	_mkdir( "psm" );
	_chdir( GetAppDirectory() );
}

void PSGame::GetHintTxt( char* strName )
{
	if ( m_bExportAvi )
		sprintf( strName, "Exporting To AVI : FPS %d [ Elapsed Time : %.3fs (TotalTime : %.3fs) ]", m_nAviFps, m_fExportedTime, m_fExportFrameTime );

	switch ( m_GameStage )
	{
	case STAND:
		if ( m_Building.m_DoorStatus == CPSBuilding::CLOSED )
			strcpy( strName, "Press the Space Key for making new Scenario.");
		else
			strcpy( strName, "");
		break;
	case PLAYING:
		if ( m_bExportAvi )
			strcat( strName, " ( Press Esc Key for cancel export.)" );
		else
			strcpy( strName, "Now it plays the recorded Scenario( Press Esc Key for cancel playing.).");
		break;
	case NAVIGATING:
		if ( m_EquipStage == EQUIP_OP_NONE )
			strcpy( strName, "Navigating methods : H Key[Help], W Key[Walk], R Key[Run], Space Key[Open Doors], Esc Key[Cancel], Mouse Move[Rotate View]");
		else
			strcpy( strName, "Operating methods : H Key[Help], Space Key[End Operation], Confirm Button[after all settings], Reset Button[Reset status]");
		break;
	case OVERVIEW_SCENE:
		strcpy( strName, "Press Esc Key for cancel overview.");
		break;
	case EXITING:
		if ( m_bExportAvi )
			strcpy( strName, "Wait until the Exporting is ended." );
		else
			strcpy( strName, "");
		break;
	default:
		strcpy( strName, "");
		break;
	}
}

void PSGame::GetName( char* strName )
{
	switch ( m_GameStage )
	{
	case PLAYING:
		strcpy( strName, "PLAYING : ");
		break;
	case NAVIGATING:
		strcpy( strName, "NAVIGATING : ");
		break;
	case INIT:
		strcpy( strName, "INIT : ");
		break;
	case SELECT_SCENE:
		strcpy( strName, "SELECT_SCENE : ");
		break;
	case SELECT_HUMEN:
		strcpy( strName, "SELECT_HUMEN : ");
		break;
	case SHOW_PREVIEW:
		strcpy( strName, "SHOW_PREVIEW : ");
		break;
	case LOADING:
		strcpy( strName, "LOADING : ");
		break;
	case OVERVIEW_SCENE:
		strcpy( strName, "OVERVIEW_SCENE : ");
		break;
	case STAND:
		strcpy( strName, "STAND : ");
		break;
	case EXITING:
		strcpy( strName, "EXITING : ");
		break;
	case HELP:
		strcpy( strName, "HELP : ");
		break;
	case EXIT:
		strcpy( strName, "EXIT : ");
		break;
	}

	switch ( m_EquipStage )
	{
	case EQUIP_OP_START:
			strcat( strName, " : EQUIP_OP_START : ");
			break;
	case EQUIP_OP_OPERATING:
			strcat( strName, " : EQUIP_OP_OPERATING : ");
			break;
	}
}

void PSGame::MakeEnvDir()
{
	CString sPath = GetAppDirectory();
	// Export AVI Path
	_mkdir(sPath + "\\Movies");
	
#ifndef SUPPORT_HTTP
	// Save File Path
	_mkdir(sPath + "\\Scenes");
#endif
}

void CPSHumenRecordManager::Init()
{
	m_vecHumenActions.clear();
}

void CPSEquipRecordManager::Init( )
{
	m_vecEquipActions.clear();
	m_iCurActionId = 0;
}

void CPSHumenRecordManager::PushAction( CPSHumenActionRecord action )
{
	m_vecHumenActions.push_back( action );
}

bool CPSHumenRecordManager::GetHumenPos( vec3 &pos, float fCurrentTime )
{
	if ( m_vecHumenActions.empty() )
		return true;

	for( size_t i = 0 ; i < m_vecHumenActions.size()-1 ; i++ )
	{
		if ( m_vecHumenActions[i].fTimeAt <= fCurrentTime && fCurrentTime < m_vecHumenActions[i+1].fTimeAt )
		{
			float s = (fCurrentTime - m_vecHumenActions[i].fTimeAt) / (m_vecHumenActions[i+1].fTimeAt - m_vecHumenActions[i].fTimeAt);
			float e = 1.0f - s;

			pos.x = e * m_vecHumenActions[i].fPosX + s * m_vecHumenActions[i+1].fPosX;
			pos.z = e * m_vecHumenActions[i].fPosZ + s * m_vecHumenActions[i+1].fPosZ;
			
			return false;
		}
	}

	return true;
}

bool CPSHumenRecordManager::GetAction( CPSHumenActionRecord &action, float fCurrentTime )
{
	if ( m_vecHumenActions.empty() )
		return true;

	for( size_t i = 0 ; i < m_vecHumenActions.size()-1 ; i++ )
	{
		if ( m_vecHumenActions[i].fTimeAt <= fCurrentTime && fCurrentTime < m_vecHumenActions[i+1].fTimeAt )
		{
			action = m_vecHumenActions[i];
			return false;
		}
	}

	return true;
}

void CPSHumenRecordManager::SaveToFile( FILE* pFile )
{
	assert( pFile );

	fwrite ( m_InitHumenPos.vec_array, sizeof(vec3), 1, pFile );

	size_t recordSize = m_vecHumenActions.size();
	fwrite ( &recordSize, sizeof(size_t), 1, pFile );

	for( size_t i = 0 ; i < m_vecHumenActions.size() ; i++ )
		fwrite ( &m_vecHumenActions[i], sizeof(CPSHumenActionRecord), 1, pFile );
}

void CPSHumenRecordManager::ReadFromFile( FILE* pFile )
{
	assert( pFile );

	fread ( m_InitHumenPos.vec_array, sizeof(vec3), 1, pFile );

	size_t recordSize;
	fread ( &recordSize, sizeof(size_t), 1, pFile );
	m_vecHumenActions.resize( recordSize );

	for( size_t i = 0 ; i < m_vecHumenActions.size() ; i++ )
		fread ( &m_vecHumenActions[i], sizeof(CPSHumenActionRecord), 1, pFile );
}

void CPSEquipRecordManager::PushAction( CPSEquipActionRecord action )
{
	m_vecEquipActions.push_back( action );
}

int CPSEquipRecordManager::GetAction( CPSEquipActionRecord &action1, CPSEquipActionRecord &action2, float fCurrentTime )
{
	for( size_t i = m_iCurActionId ; i < m_vecEquipActions.size() ; i++ )
	{
		if ( m_vecEquipActions[i].fTimeAt <= fCurrentTime )
		{
			int r = 1;
			action1 = m_vecEquipActions[i];
			if ( i+1 < m_vecEquipActions.size() )
			{
				r = 2;
				action2 = m_vecEquipActions[i+1];
			}
			m_iCurActionId = i+1;
			return r;
		}
	}

	return 0;
}

void CPSEquipRecordManager::SaveToFile( FILE* pFile )
{
	assert( pFile );

	size_t recordSize = m_vecEquipActions.size();
	fwrite ( &recordSize, sizeof(size_t), 1, pFile );

	for( size_t i = 0 ; i < m_vecEquipActions.size() ; i++ )
		fwrite ( &m_vecEquipActions[i], sizeof(CPSEquipActionRecord), 1, pFile );
}

void CPSEquipRecordManager::ReadFromFile( FILE* pFile )
{
	assert( pFile );

	size_t recordSize;
	fread ( &recordSize, sizeof(size_t), 1, pFile );
	m_vecEquipActions.resize( recordSize );

	for( size_t i = 0 ; i < m_vecEquipActions.size() ; i++ )
		fread ( &m_vecEquipActions[i], sizeof(CPSEquipActionRecord), 1, pFile );

}

void LoadHumenData()
{
	g_PSGame.m_Humen.Load( );
}

void LoadSceneData()
{
	g_PSGame.LoadScenario(0);
}

void LoadScenario(  )
{
#ifdef SUPPORT_HTTP
 	CStringArray* scenes = HttpListScene( );
	if ( scenes == 0 )
		return;

	CDlgDownLoad dlgList( theApp.m_pMainWnd );
	dlgList.m_strNames.Copy(*scenes);
	delete scenes;	

 	if ( dlgList.DoModal() == IDOK )
 	{
		_mkdir( "Scenes" );
		CString scenePath = "Scenes\\" + dlgList.m_strName;
		HttpDownLoad( scenePath );
		_chdir( GetAppDirectory() );		
		g_PSGame.OpenScenario( scenePath );
		DeleteFile ( scenePath );
		_rmdir( "Scenes" );
	}
#else
	CString path = GetAppDirectory() + "\\Scenes\\*.pss";
	CFileDialog dlg(TRUE, "pss", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Scenario File(*.PSS)|*.pss|", AfxGetMainWnd() );
	if(dlg.DoModal() == IDOK)
	{
		_chdir( GetAppDirectory() );		
		g_PSGame.OpenScenario( dlg.GetPathName().GetBuffer() );
	}
#endif
}

void SaveScenario()
{
#ifdef SUPPORT_HTTP
	CDlgUpLoad dlgUpload( theApp.m_pMainWnd );
	if ( dlgUpload.DoModal() == IDOK )
	{
		_mkdir( "Scenes" );
		CString path = "Scenes\\" + dlgUpload.m_strScenario + ".pss";
		g_PSGame.SaveScenario( path );
		HttpUpload( path );
		DeleteFile ( path );
		_rmdir( "Scenes" );
	}
#else
	CString path = GetAppDirectory() + "\\Scenes\\Scene.pss";
	CFileDialog dlg(FALSE, "pss", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Scenario File(*.PSS)|*.pss|");
	if(dlg.DoModal() == IDOK)
		g_PSGame.SaveScenario(dlg.GetPathName().GetBuffer());
#endif
}