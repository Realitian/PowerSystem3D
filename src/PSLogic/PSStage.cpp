#include "stdafx.h"
#include "MainFrm.h"
#include "PS3DViewerDoc.h"
#include "PS3DViewerView.h"
#include "PSStage.h"
#include "PSGame.h"
#include "DlgRenderConfig.h"

CPSStage::CPSStage()
{
	m_nDrawState = HITID_NONE;
	m_nClickState = HITID_NONE;
	m_nSelScene = HITID_BUILDING1;
	m_nSelSex = HITID_TXT_MAN;
	m_nSelUniform = HITID_MAN_UNIFORM1;
}

void CPSStage::Resize(int cx, int cy)
{
	g_PSGame.m_nWidth = cx;
	g_PSGame.m_nHeight = cy;
	
	m_nWidth = cx;
	m_nHeight = cy;

	// Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, cx, 0, cy);
}

void CPSStage::LoadTexture()
{
	GLbyte *pBytes;
	GLint nWidth, nHeight, nComponents;
	GLenum eFormat;
	CString appPath = GetAppDirectory() + "\\";
	CString path;

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glGenTextures(TEXID_COUNT, m_nTextures);
	for(int i = TEXID_FIRST; i < TEXID_COUNT; i++)
	{
		glBindTexture(GL_TEXTURE_2D, m_nTextures[i]);
		path = appPath + szTextureFiles[i];
		pBytes = LoadImage(path, &nWidth, &nHeight, &nComponents, &eFormat, true);
		gluBuild2DMipmaps(GL_TEXTURE_2D, nComponents, nWidth, nHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		free(pBytes);
	}
}

GLint CPSStage::GetSelectHits( int nX, int nY, GLuint* selectBuf )
{
	GLint hits, viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(BUFFER_LENGTH, selectBuf);
	glRenderMode(GL_SELECT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	
	glLoadIdentity();
	gluPickMatrix(nX, viewport[3] - nY, 2, 2, viewport);
	gluOrtho2D(0, viewport[2], 0, viewport[3]);

	glMatrixMode(GL_MODELVIEW);
	Render();
	
	hits = glRenderMode(GL_RENDER);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	return hits;
}

void CPSStage::Render()
{
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	if(g_PSGame.m_GameStage != HELP)
	{
		glInitNames();
		glPushName(HITID_NONE);

		switch(g_PSGame.m_GameStage)
		{
		case INIT:			// Main Menu
			{
				RenderMainMenu();
				break;
			}
		case SELECT_SCENE:			// Select Scene
			{
				RenderSceneSelect();
				break;
			}
		case SELECT_HUMEN:			// Select Humen
			{
				RenderHumenSelect();
				break;
			}
		case SHOW_PREVIEW:			// Show Preview
			{
				ShowPreview();
				break;
			}
		case LOADING:
			{
				break;
			}
		case STAND:
			{
				if ( !g_PSGame.m_bExportAvi && g_PSGame.m_Building.m_DoorStatus == CPSBuilding::CLOSED )
					RenderStand();
				break;
			}
		}

		glPopName();
	}
	else
	{
		ShowHelp(g_PSGame.m_nHelpState);
	}
}

void CPSStage::RenderHighLight( int nIndex )
{
	if(m_nDrawState != nIndex/* && TEXID_BACKGROUND == m_nClickState*/)
	{
		switch(nIndex)
		{
		case HITID_NEW:
			{
				m_nDrawState = HITID_NEW_OVER;
				break;
			}
		case HITID_SAVED:
			{
				m_nDrawState = HITID_SAVED_OVER;
				break;
			}
		case HITID_EXIT:
			{
				m_nDrawState = HITID_EXIT_OVER;
				break;
			}
		case HITID_BUILDING1:
			{
				m_nDrawState = HITID_BUILDING1_SELECT;
				break;
			}
		case HITID_BUILDING2:
			{
				m_nDrawState = HITID_BUILDING2_SELECT;
				break;
			}
		case HITID_BUILDING3:
			{
				m_nDrawState = HITID_BUILDING3_SELECT;
				break;
			}
		case HITID_NEXT:
			{
				m_nDrawState = HITID_NEXT_OVER;
				break;
			}
		case HITID_PREVIOUS:
			{
				m_nDrawState = HITID_PREVIOUS_OVER;
				break;
			}
		case HITID_START:
			{
				m_nDrawState = HITID_START_OVER;
				break;
			}
		case HITID_TXT_MAN:
			{
				m_nDrawState = HITID_TXT_MAN_OVER;
				break;
			}
		case HITID_TXT_WOMEN:
			{
				m_nDrawState = HITID_TXT_WOMEN_OVER;
				break;
			}
		case HITID_MAN_UNIFORM1:
			{
				if(m_nDrawState == HITID_MAN_UNIFORM1_SELECT)
					return;
				m_nDrawState = HITID_MAN_UNIFORM1_SELECT;
				break;
			}
		case HITID_MAN_UNIFORM2:
			{
				if(m_nDrawState == HITID_MAN_UNIFORM2_SELECT)
					return;
				m_nDrawState = HITID_MAN_UNIFORM2_SELECT;
				break;
			}
		case HITID_WOMEN_UNIFORM1:
			{
				if(m_nDrawState == HITID_WOMEN_UNIFORM1_SELECT)
					return;
				m_nDrawState = HITID_WOMEN_UNIFORM1_SELECT;
				break;
			}
		case HITID_WOMEN_UNIFORM2:
			{
				if(m_nDrawState == HITID_WOMEN_UNIFORM2_SELECT)
					return;
				m_nDrawState = HITID_WOMEN_UNIFORM2_SELECT;
				break;
			}
		case HITID_SAVE:
			{
				m_nDrawState = HITID_SAVE_OVER;
				break;
			}
		case HITID_PLAY:
			{
				m_nDrawState = HITID_PLAY_OVER;
				break;
			}
		case HITID_EXPORT:
			{
				m_nDrawState = HITID_EXPORT_OVER;
				break;
			}
		case HITID_MAINMENU:
			{
				m_nDrawState = HITID_MAINMENU_OVER;
				break;
			}
		case HITID_MAN_UNIFORM1_SELECT:
			{
				if(m_nSelUniform == HITID_MAN_UNIFORM1)
					return;
			}
		case HITID_MAN_UNIFORM2_SELECT:
			{
				if(m_nSelUniform == HITID_MAN_UNIFORM2)
					return;
			}
		case HITID_WOMEN_UNIFORM1_SELECT:
			{
				if(m_nSelUniform == HITID_WOMEN_UNIFORM1)
					return;
			}
		case HITID_WOMEN_UNIFORM2_SELECT:
			{
				if(m_nSelUniform == HITID_WOMEN_UNIFORM2)
					return;
			}
		case HITID_TXT_MAN_SELECT:
			{
				if(m_nSelSex == HITID_TXT_MAN)
					return;
			}
		case HITID_TXT_WOMEN_SELECT:
			{
				if(m_nSelSex == HITID_TXT_WOMEN)
					return;
			}
		default:
			m_nDrawState = HITID_NONE;
			break;
		}
		((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
	}
}

void CPSStage::RenderBackground()
{
	glEnable(GL_TEXTURE_2D);
	glDepthMask( GL_FALSE );
	glDisable( GL_TEXTURE_2D );
	
	glBegin(GL_QUADS);
	glColor3f( 0.1f, 0.1f, 0.1f );
	glVertex2f(0.0f, 0.0f);

	glColor3f( 0.3f, 0.3f, 0.3f );
	glVertex2f((GLfloat)m_nWidth, 0.0f);

	glColor3f( 0.3f, 0.3f, 0.3f );
	glVertex2f((GLfloat)m_nWidth, (GLfloat)m_nHeight);

	glColor3f( 0.1f, 0.1f, 0.1f );
	glVertex2f(0.0f, (GLfloat)m_nHeight);
	glEnd();

	glDepthMask( GL_TRUE );
	glEnable( GL_TEXTURE_2D );
}

// Main Menu
void CPSStage::RenderMainMenu()
{
	glLoadName(HITID_NONE);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// BGIcon
	glBlendFunc( GL_ONE, GL_ONE );
	glTranslatef(0.0f, 0.0f, -0.3f);
	glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_BGICON]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f((GLfloat)m_nWidth, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f((GLfloat)m_nWidth, 0.9f * (GLfloat)m_nHeight);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, 0.9f * (GLfloat)m_nHeight);
	glEnd();
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Title
	glTranslatef(0.0f, 0.0f, 0.1f);
	glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_TITLE]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.1f * (GLfloat)m_nWidth, 0.7f * (GLfloat)m_nHeight);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(0.9f * (GLfloat)m_nWidth, 0.7f * (GLfloat)m_nHeight);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(0.9f * (GLfloat)m_nWidth, (GLfloat)m_nHeight);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.1f * (GLfloat)m_nWidth, (GLfloat)m_nHeight);
	glEnd();
	//
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if(m_nDrawState == HITID_NEW_OVER)			// Drawing New Button
	{
		glLoadName(HITID_NEW_OVER);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_NEW_OVER]); 
	}
	else if(m_nDrawState == HITID_NEW_DOWN)
	{
		glLoadName(HITID_NEW_DOWN);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_NEW_DOWN]); 
	}
	else
	{
		glLoadName(HITID_NEW);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_NEW]); 
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f((GLfloat)m_nWidth - 550, (GLfloat)180);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)180);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)240);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f((GLfloat)m_nWidth - 550, (GLfloat)240);
	glEnd();

	if(m_nDrawState == HITID_SAVED_OVER)			// Drawing Saved Button
	{
		glLoadName(HITID_SAVED_OVER);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_SAVED_OVER]); 
	}
	else if(m_nDrawState == HITID_SAVED_DOWN)
	{
		glLoadName(HITID_SAVED_DOWN);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_SAVED_DOWN]); 
	}
	else
	{
		glLoadName(HITID_SAVED);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_SAVED]); 
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f((GLfloat)m_nWidth - 600, (GLfloat)110);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)110);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)170);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f((GLfloat)m_nWidth - 600, (GLfloat)170);
	glEnd();

	if(m_nDrawState == HITID_EXIT_OVER)			// Drawing Exit Button
	{
		glLoadName(HITID_EXIT_OVER);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_EXIT_OVER]); 
	}
	else if(m_nDrawState == HITID_EXIT_DOWN)
	{
		glLoadName(HITID_EXIT_DOWN);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_EXIT_DOWN]); 
	}
	else
	{
		glLoadName(HITID_EXIT);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_EXIT]); 
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f((GLfloat)m_nWidth - 240, (GLfloat)40);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)40);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)100);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f((GLfloat)m_nWidth - 240, (GLfloat)100);
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CPSStage::RenderSceneSelect()					// Select Scene
{
	glLoadName(HITID_NONE);				// Drawing Background
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Title
	glTranslatef(0.0f, 0.0f, -0.1f);
	glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_SCENE]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.2f * (GLfloat)m_nWidth, 0.85f * (GLfloat)m_nHeight);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(0.8f * (GLfloat)m_nWidth, 0.85f * (GLfloat)m_nHeight);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(0.8f * (GLfloat)m_nWidth, 0.95f * (GLfloat)m_nHeight);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.2f * (GLfloat)m_nWidth, 0.95f * (GLfloat)m_nHeight);
	glEnd();
	//
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if(m_nSelScene == HITID_BUILDING1)			// Drawing Building1
	{
		glLoadName(HITID_BUILDING1_SELECT);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_BUILDING1_SELECT]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.1f * (GLfloat)m_nWidth, 0.45f * (GLfloat)m_nHeight);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(0.4f * (GLfloat)m_nWidth, 0.45f * (GLfloat)m_nHeight);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(0.4f * (GLfloat)m_nWidth, 0.75f * (GLfloat)m_nHeight);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.1f * (GLfloat)m_nWidth, 0.75f * (GLfloat)m_nHeight);
		glEnd();
	}
	else
	{
		glLoadName(HITID_BUILDING1);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_BUILDING1]);
		glBegin(GL_QUADS);
		if(m_nDrawState == HITID_BUILDING1_SELECT)
		{
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(0.09f * m_nWidth, 0.44f * m_nHeight);

			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(0.41f * m_nWidth, 0.44f * m_nHeight);

			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(0.41f * m_nWidth, 0.76f * m_nHeight);

			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(0.09f * m_nWidth, 0.76f * m_nHeight);
		}
		else
		{
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(0.1f * m_nWidth, 0.45f * m_nHeight);

			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(0.4f * m_nWidth, 0.45f * m_nHeight);

			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(0.4f * m_nWidth, 0.75f * m_nHeight);

			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(0.1f * m_nWidth, 0.75f * m_nHeight);
		}
		glEnd();
	}

	if(m_nSelScene == HITID_BUILDING2)			// Drawing Building2
	{
		glLoadName(HITID_BUILDING2_SELECT);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_BUILDING2_SELECT]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.6f * m_nWidth, 0.45f * m_nHeight);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(0.9f * m_nWidth, 0.45f * m_nHeight);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(0.9f * m_nWidth, 0.75f * m_nHeight);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.6f * m_nWidth, 0.75f * m_nHeight);
		glEnd();
	}
	else
	{
		glLoadName(HITID_BUILDING2);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_BUILDING2]);
		glBegin(GL_QUADS);
		if(m_nDrawState == HITID_BUILDING2_SELECT)
		{
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(0.59f * m_nWidth, 0.44f * m_nHeight);

			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(0.91f * m_nWidth, 0.44f * m_nHeight);

			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(0.91f * m_nWidth, 0.76f * m_nHeight);

			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(0.59f * m_nWidth, 0.76f * m_nHeight);
		}
		else
		{
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(0.6f * m_nWidth, 0.45f * m_nHeight);

			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(0.9f * m_nWidth, 0.45f * m_nHeight);

			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(0.9f * m_nWidth, 0.75f * m_nHeight);

			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(0.6f * m_nWidth, 0.75f * m_nHeight);
		}
		glEnd();
	}

	if(m_nSelScene == HITID_BUILDING3)				// Drawing Building3
	{
		glLoadName(HITID_BUILDING3_SELECT);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_BUILDING3_SELECT]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.35f * m_nWidth, 0.05f * m_nHeight);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(0.65f * m_nWidth, 0.05f * m_nHeight);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(0.65f * m_nWidth, 0.35f * m_nHeight);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.35f * m_nWidth, 0.35f * m_nHeight);
		glEnd();
	}
	else
	{
		glLoadName(HITID_BUILDING3);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_BUILDING3]);
		glBegin(GL_QUADS);
		if(m_nDrawState == HITID_BUILDING3_SELECT)
		{
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(0.34f * m_nWidth, 0.04f * m_nHeight);

			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(0.66f * m_nWidth, 0.04f * m_nHeight);

			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(0.66f * m_nWidth, 0.36f * m_nHeight);

			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(0.34f * m_nWidth, 0.36f * m_nHeight);
		}
		else
		{
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(0.35f * m_nWidth, 0.05f * m_nHeight);

			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(0.65f * m_nWidth, 0.05f * m_nHeight);

			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(0.65f * m_nWidth, 0.35f * m_nHeight);

			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(0.35f * m_nWidth, 0.35f * m_nHeight);
		}
		glEnd();
	}

	if(m_nDrawState == HITID_NEXT_OVER)			// Drawing Next Button
	{
		glLoadName(HITID_NEXT_OVER);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_NEXT_OVER]); 
	}
	else if(m_nDrawState == HITID_NEXT_DOWN)
	{
		glLoadName(HITID_NEXT_DOWN);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_NEXT_DOWN]); 
	}
	else
	{
		glLoadName(HITID_NEXT);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_NEXT]); 
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f((GLfloat)m_nWidth - 150, (GLfloat)30);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)30);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)70);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f((GLfloat)m_nWidth - 150, (GLfloat)70);
	glEnd();

	if(m_nDrawState == HITID_PREVIOUS_OVER)			// Drawing Previous Button
	{
		glLoadName(HITID_PREVIOUS_OVER);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PREVIOUS_OVER]); 
	}
	else if(m_nDrawState == HITID_PREVIOUS_DOWN)
	{
		glLoadName(HITID_PREVIOUS_DOWN);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PREVIOUS_DOWN]); 
	}
	else
	{
		glLoadName(HITID_PREVIOUS);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PREVIOUS]); 
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f((GLfloat)70, (GLfloat)30);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f((GLfloat)150, (GLfloat)30);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f((GLfloat)150, (GLfloat)70);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f((GLfloat)70, (GLfloat)70);
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

// Select Human
void CPSStage::RenderHumenSelect()
{
	glLoadName(HITID_NONE);				// Drawing Background
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Title
	glTranslatef(0.0f, 0.0f, 0.1f);
	glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_WORKER]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.35f * (GLfloat)m_nWidth, 0.85f * (GLfloat)m_nHeight);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(0.65f * (GLfloat)m_nWidth, 0.85f * (GLfloat)m_nHeight);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(0.65f * (GLfloat)m_nWidth, 0.95f * (GLfloat)m_nHeight);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.35f * (GLfloat)m_nWidth, 0.95f * (GLfloat)m_nHeight);
	glEnd();

	if( g_PSGame.m_Humen.m_bLoaded )
	{
		// Txt Sex
		glTranslatef(0.0f, 0.0f, 0.1f);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_SEX]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.5f * (GLfloat)m_nWidth, 0.65f * (GLfloat)m_nHeight);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(0.5f * (GLfloat)m_nWidth + 120, 0.65f * (GLfloat)m_nHeight);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(0.5f * (GLfloat)m_nWidth + 120, 0.65f * (GLfloat)m_nHeight + 50);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.5f * (GLfloat)m_nWidth, 0.65f * (GLfloat)m_nHeight + 50);
		glEnd();
		// Txt Clothes
		glTranslatef(0.0f, 0.0f, 0.1f);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_CLOTHES]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.5f * (GLfloat)m_nWidth, 0.42f * (GLfloat)m_nHeight);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(0.5f * (GLfloat)m_nWidth + 250, 0.42f * (GLfloat)m_nHeight);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(0.5f * (GLfloat)m_nWidth + 250, 0.42f * (GLfloat)m_nHeight + 50);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.5f * (GLfloat)m_nWidth, 0.42f * (GLfloat)m_nHeight + 50);
		glEnd();
		//
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		if(m_nSelSex == HITID_TXT_MAN)			// Drawing TxtMan
		{
			glLoadName(HITID_TXT_MAN_SELECT);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_MAN_SELECT]);
		}
		else
		{
			if(m_nDrawState == HITID_TXT_MAN_OVER)			
			{
				glLoadName(HITID_TXT_MAN_OVER);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_MAN_OVER]); 
			}
			else if(m_nDrawState == HITID_TXT_MAN_DOWN)
			{
				glLoadName(HITID_TXT_MAN_DOWN);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_MAN_DOWN]); 
			}
			else
			{
				glLoadName(HITID_TXT_MAN);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_MAN]); 
			}
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.55f * m_nWidth, 0.55f * m_nHeight);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(0.55f * m_nWidth + 150, 0.55f * m_nHeight);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(0.55f * m_nWidth + 150, 0.55f * m_nHeight + 45);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.55f * m_nWidth, 0.55f * m_nHeight + 45);
		glEnd();

		if(m_nSelSex == HITID_TXT_WOMEN)			// Drawing TxtWomen
		{
			glLoadName(HITID_TXT_WOMEN_SELECT);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_WOMEN_SELECT]);
		}
		else
		{
			if(m_nDrawState == HITID_TXT_WOMEN_OVER)			
			{
				glLoadName(HITID_TXT_WOMEN_OVER);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_WOMEN_OVER]); 
			}
			else if(m_nDrawState == HITID_TXT_WOMEN_DOWN)
			{
				glLoadName(HITID_TXT_WOMEN_DOWN);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_WOMEN_DOWN]); 
			}
			else
			{
				glLoadName(HITID_TXT_WOMEN);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_TXT_WOMEN]); 
			}
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.75f * m_nWidth, 0.55f * m_nHeight);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(0.75f * m_nWidth + 210, 0.55f * m_nHeight);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(0.75f * m_nWidth + 210, 0.55f * m_nHeight + 45);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.75f * m_nWidth, 0.55f * m_nHeight + 45);
		glEnd();

		if(m_nSelSex == HITID_TXT_MAN)						// Draw Man Uniform
		{
			if(m_nSelUniform == HITID_MAN_UNIFORM1)			// Man Uniform1
			{
				glLoadName(HITID_MAN_UNIFORM1_SELECT);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_MAN_UNIFORM1_SELECT]);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.55f * m_nWidth, 0.35f * m_nHeight - 150);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(0.55f * m_nWidth + 120, 0.35f * m_nHeight - 150);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(0.55f * m_nWidth + 120, 0.35f * m_nHeight);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.55f * m_nWidth, 0.35f * m_nHeight);
				glEnd();
			}
			else
			{
				glLoadName(HITID_MAN_UNIFORM1);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_MAN_UNIFORM1]);
				glBegin(GL_QUADS);
				if(m_nDrawState == HITID_MAN_UNIFORM1_SELECT)
				{
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth - 15, 0.35f * m_nHeight - 165);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 135, 0.35f * m_nHeight - 165);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 135, 0.35f * m_nHeight + 15);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth - 15, 0.35f * m_nHeight + 15);
				}
				else
				{
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth, 0.35f * m_nHeight - 150);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 120, 0.35f * m_nHeight - 150);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 120, 0.35f * m_nHeight);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth, 0.35f * m_nHeight);
				}
				glEnd();
			}

			if(m_nSelUniform == HITID_MAN_UNIFORM2)			// Man Uniform2
			{
				glLoadName(HITID_MAN_UNIFORM2_SELECT);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_MAN_UNIFORM2_SELECT]);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.55f * m_nWidth + 200, 0.35f * m_nHeight - 150);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(0.55f * m_nWidth + 320, 0.35f * m_nHeight - 150);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(0.55f * m_nWidth + 320, 0.35f * m_nHeight);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.55f * m_nWidth + 200, 0.35f * m_nHeight);
				glEnd();
			}
			else
			{
				glLoadName(HITID_MAN_UNIFORM2);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_MAN_UNIFORM2]);
				glBegin(GL_QUADS);
				if(m_nDrawState == HITID_MAN_UNIFORM2_SELECT)
				{
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 185, 0.35f * m_nHeight - 165);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 335, 0.35f * m_nHeight - 165);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 335, 0.35f * m_nHeight + 15);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 185, 0.35f * m_nHeight + 15);
				}
				else
				{
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 200, 0.35f * m_nHeight - 150);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 320, 0.35f * m_nHeight - 150);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 320, 0.35f * m_nHeight);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 200, 0.35f * m_nHeight);
				}
				glEnd();
			}
		}
		else if(m_nSelSex == HITID_TXT_WOMEN)				// Draw Women Uniform
		{
			if(m_nSelUniform == HITID_WOMEN_UNIFORM1)			// Women Uniform1
			{
				glLoadName(HITID_WOMEN_UNIFORM1_SELECT);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_WOMEN_UNIFORM1_SELECT]);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.55f * m_nWidth, 0.35f * m_nHeight - 150);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(0.55f * m_nWidth + 120, 0.35f * m_nHeight - 150);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(0.55f * m_nWidth + 120, 0.35f * m_nHeight);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.55f * m_nWidth, 0.35f * m_nHeight);
				glEnd();
			}
			else
			{
				glLoadName(HITID_WOMEN_UNIFORM1);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_WOMEN_UNIFORM1]);
				glBegin(GL_QUADS);
				if(m_nDrawState == HITID_WOMEN_UNIFORM1_SELECT)
				{
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth - 15, 0.35f * m_nHeight - 165);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 135, 0.35f * m_nHeight - 165);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 135, 0.35f * m_nHeight + 15);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth - 15, 0.35f * m_nHeight + 15);
				}
				else
				{
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth, 0.35f * m_nHeight - 150);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 120, 0.35f * m_nHeight - 150);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 120, 0.35f * m_nHeight);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth, 0.35f * m_nHeight);
				}
				glEnd();
			}

			if(m_nSelUniform == HITID_WOMEN_UNIFORM2)			// Women Uniform2
			{
				glLoadName(HITID_WOMEN_UNIFORM2_SELECT);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_WOMEN_UNIFORM2_SELECT]);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.55f * m_nWidth + 200, 0.35f * m_nHeight - 150);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(0.55f * m_nWidth + 320, 0.35f * m_nHeight - 150);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(0.55f * m_nWidth + 320, 0.35f * m_nHeight);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.55f * m_nWidth + 200, 0.35f * m_nHeight);
				glEnd();
			}
			else
			{
				glLoadName(HITID_WOMEN_UNIFORM2);
				glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_WOMEN_UNIFORM2]);
				glBegin(GL_QUADS);
				if(m_nDrawState == HITID_WOMEN_UNIFORM2_SELECT)
				{
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 185, 0.35f * m_nHeight - 165);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 335, 0.35f * m_nHeight - 165);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 335, 0.35f * m_nHeight + 15);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 185, 0.35f * m_nHeight + 15);
				}
				else
				{
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 200, 0.35f * m_nHeight - 150);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(0.55f * m_nWidth + 320, 0.35f * m_nHeight - 150);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 320, 0.35f * m_nHeight);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0.55f * m_nWidth + 200, 0.35f * m_nHeight);
				}
				glEnd();
			}
		}

		if(m_nDrawState == HITID_NEXT_OVER)			// Drawing Next Button
		{
			glLoadName(HITID_NEXT_OVER);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_NEXT_OVER]); 
		}
		else if(m_nDrawState == HITID_NEXT_DOWN)
		{
			glLoadName(HITID_NEXT_DOWN);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_NEXT_DOWN]); 
		}
		else
		{
			glLoadName(HITID_NEXT);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_NEXT]); 
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f((GLfloat)m_nWidth - 150, (GLfloat)30);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)30);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f((GLfloat)m_nWidth - 70, (GLfloat)70);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f((GLfloat)m_nWidth - 150, (GLfloat)70);
		glEnd();

		if(m_nDrawState == HITID_PREVIOUS_OVER)			// Drawing Previous Button
		{
			glLoadName(HITID_PREVIOUS_OVER);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PREVIOUS_OVER]); 
		}
		else if(m_nDrawState == HITID_PREVIOUS_DOWN)
		{
			glLoadName(HITID_PREVIOUS_DOWN);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PREVIOUS_DOWN]); 
		}
		else
		{
			glLoadName(HITID_PREVIOUS);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PREVIOUS]); 
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f((GLfloat)70, (GLfloat)30);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f((GLfloat)150, (GLfloat)30);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f((GLfloat)150, (GLfloat)70);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f((GLfloat)70, (GLfloat)70);
		glEnd();
	}
	else
	{
		static float fWidth = 0.54f;
		
		static DWORD diff = 0;
		
		static DWORD old = GetTickCount();
		diff += GetTickCount() - old;
		old = GetTickCount();
		
		if ( diff > 500 )
		{
			fWidth += 0.45f/8.0f;
			diff = 0;
		}

		if ( fWidth >= 1.0f )
			fWidth = 0.54f;

		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_LOADING]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.25f * (GLfloat)m_nWidth, 0.15f * (GLfloat)m_nHeight);

		glTexCoord2f(fWidth, 0.0f);
		glVertex2f((0.5f*fWidth+0.25f) * (GLfloat)m_nWidth, 0.15f * (GLfloat)m_nHeight);

		glTexCoord2f(fWidth, 1.0f);
		glVertex2f((0.5f*fWidth+0.25f) * (GLfloat)m_nWidth, 0.35f * (GLfloat)m_nHeight);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.25f * (GLfloat)m_nWidth, 0.35f * (GLfloat)m_nHeight);
		glEnd();
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
// Show Preview
void CPSStage::ShowPreview()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if( g_PSGame.m_bLoaded )
	{
		if(m_nDrawState == HITID_START_OVER)			// Drawing Start Button
		{
			glLoadName(HITID_START_OVER);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_START_OVER]); 
		}
		else if(m_nDrawState == HITID_START_DOWN)
		{
			glLoadName(HITID_START_DOWN);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_START_DOWN]); 
		}
		else
		{
			glLoadName(HITID_START);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_START]); 
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f((GLfloat)m_nWidth - 250, (GLfloat)25);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f((GLfloat)m_nWidth - 50, (GLfloat)25);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f((GLfloat)m_nWidth - 50, (GLfloat)80);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f((GLfloat)m_nWidth - 250, (GLfloat)80);
		glEnd();

		if(m_nDrawState == HITID_PREVIOUS_OVER)			// Drawing Previous Button
		{
			glLoadName(HITID_PREVIOUS_OVER);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PREVIOUS_OVER]); 
		}
		else if(m_nDrawState == HITID_PREVIOUS_DOWN)
		{
			glLoadName(HITID_PREVIOUS_DOWN);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PREVIOUS_DOWN]); 
		}
		else
		{
			glLoadName(HITID_PREVIOUS);
			glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PREVIOUS]); 
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f((GLfloat)70, (GLfloat)30);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f((GLfloat)150, (GLfloat)30);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f((GLfloat)150, (GLfloat)70);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f((GLfloat)70, (GLfloat)70);
		glEnd();
	}
	else
	{
		static float fWidth = 0.54f;

		static DWORD diff = 0;

		static DWORD old = GetTickCount();
		diff += GetTickCount() - old;
		old = GetTickCount();

		if ( diff > 500 )
		{
			fWidth += 0.45f/8.0f;
			diff = 0;
		}

		if ( fWidth >= 1.0f )
			fWidth = 0.54f;

		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_LOADING]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.25f * (GLfloat)m_nWidth, 0.15f * (GLfloat)m_nHeight);

		glTexCoord2f(fWidth, 0.0f);
		glVertex2f((0.5f*fWidth+0.25f) * (GLfloat)m_nWidth, 0.15f * (GLfloat)m_nHeight);

		glTexCoord2f(fWidth, 1.0f);
		glVertex2f((0.5f*fWidth+0.25f) * (GLfloat)m_nWidth, 0.35f * (GLfloat)m_nHeight);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.25f * (GLfloat)m_nWidth, 0.35f * (GLfloat)m_nHeight);
		glEnd();
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CPSStage::RenderStand()
{
	GLfloat nCenterX = (GLfloat)m_nWidth / 2;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if(m_nDrawState == HITID_SAVE_OVER)			// Save
	{
		glLoadName(HITID_SAVE_OVER);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_SAVE_OVER]); 
	}
	else if(m_nDrawState == HITID_SAVE_DOWN)
	{
		glLoadName(HITID_SAVE_DOWN);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_SAVE_DOWN]); 
	}
	else
	{
		glLoadName(HITID_SAVE);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_SAVE]); 
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(nCenterX - 260, 0.7f * m_nHeight);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(nCenterX + 260, 0.7f * m_nHeight);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(nCenterX + 260, 0.7f * m_nHeight + 60);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(nCenterX - 260, 0.7f * m_nHeight + 60);
	glEnd();

	if(m_nDrawState == HITID_PLAY_OVER)			// Play
	{
		glLoadName(HITID_PLAY_OVER);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PLAY_OVER]); 
	}
	else if(m_nDrawState == HITID_PLAY_DOWN)
	{
		glLoadName(HITID_PLAY_DOWN);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PLAY_DOWN]); 
	}
	else
	{
		glLoadName(HITID_PLAY);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_PLAY]); 
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(nCenterX - 260, 0.55f * m_nHeight);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(nCenterX + 260, 0.55f * m_nHeight);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(nCenterX + 260, 0.55f * m_nHeight + 60);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(nCenterX - 260, 0.55f * m_nHeight + 60);
	glEnd();

	if(m_nDrawState == HITID_EXPORT_OVER)			// Export to Movie
	{
		glLoadName(HITID_EXPORT_OVER);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_EXPORT_OVER]); 
	}
	else if(m_nDrawState == HITID_EXPORT_DOWN)
	{
		glLoadName(HITID_EXPORT_DOWN);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_EXPORT_DOWN]); 
	}
	else
	{
		glLoadName(HITID_EXPORT);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_EXPORT]); 
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(nCenterX - 280, 0.4f * m_nHeight);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(nCenterX + 280, 0.4f * m_nHeight);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(nCenterX + 280, 0.4f * m_nHeight + 60);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(nCenterX - 280, 0.4f * m_nHeight + 60);
	glEnd();

	if(m_nDrawState == HITID_MAINMENU_OVER)			// MainMenu
	{
		glLoadName(HITID_MAINMENU_OVER);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_MAINMENU_OVER]); 
	}
	else if(m_nDrawState == HITID_MAINMENU_DOWN)
	{
		glLoadName(HITID_MAINMENU_DOWN);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_MAINMENU_DOWN]); 
	}
	else
	{
		glLoadName(HITID_MAINMENU);
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_MAINMENU]); 
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(nCenterX - 180, 0.25f * m_nHeight);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(nCenterX + 180, 0.25f * m_nHeight);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(nCenterX + 180, 0.25f * m_nHeight + 60);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(nCenterX - 180, 0.25f * m_nHeight + 60);
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CPSStage::ShowHelp(int state)
{
	GLfloat fCenterX = (GLfloat)m_nWidth / 2;
	GLfloat fCenterY = (GLfloat)m_nHeight / 2;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, m_nWidth, 0, m_nHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor4f( 1.0f, 1.0f, 1.0f, 0.8f );
	glEnable( GL_TEXTURE_2D );
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	switch(state)
	{
	case HELP_EQUIPU:
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_HELP_EQUIPU]);
		break;
	case HELP_EQUIPI:
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_HELP_EQUIPI]);
		break;
	case HELP_DEFAULT:
	default:
		glBindTexture(GL_TEXTURE_2D, m_nTextures[TEXID_HELP]);
		break;
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(fCenterX - 400, fCenterY - 300);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(fCenterX + 400, fCenterY - 300);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(fCenterX + 400, fCenterY + 300);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(fCenterX - 400, fCenterY + 300);
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CPSStage::OnLButtonDown(UINT nFlags, CPoint point)
{
	switch(g_PSGame.m_GameStage)
	{
	case SELECT_HUMEN:
		if ( !g_PSGame.m_Humen.m_bLoaded )
			return;
	case INIT:
	case SELECT_SCENE:
	case SHOW_PREVIEW:
	case STAND:
		{
			GLuint selectBuf[BUFFER_LENGTH];
			GLint hits = GetSelectHits(point.x, point.y, selectBuf);
			if(hits != 0)
			{
				int nClicked = selectBuf[hits * 4 - 1];
				switch (nClicked)
				{
				case HITID_NEW_OVER:
					{
						m_nClickState = HITID_NEW;
						m_nDrawState = HITID_NEW_DOWN;
						break;
					}
				case HITID_SAVED_OVER:
					{
						m_nClickState = HITID_SAVED;
						m_nDrawState = HITID_SAVED_DOWN;
						break;
					}
				case HITID_EXIT_OVER:
					{
						m_nClickState = HITID_EXIT;
						m_nDrawState = HITID_EXIT_DOWN;
						break;
					}
				case HITID_BUILDING1:
					{
						m_nSelScene = HITID_BUILDING1;
						break;
					}
				case HITID_BUILDING2:
					{
						m_nSelScene = HITID_BUILDING2;
						break;
					}
				case HITID_BUILDING3:
					{
						m_nSelScene = HITID_BUILDING3;
						break;
					}
				case HITID_NEXT_OVER:
					{
						m_nClickState = HITID_NEXT;
						m_nDrawState = HITID_NEXT_DOWN;
						break;
					}
				case HITID_PREVIOUS_OVER:
					{
						m_nClickState = HITID_PREVIOUS;
						m_nDrawState = HITID_PREVIOUS_DOWN;
						break;
					}
				case HITID_START_OVER:
					{
						m_nClickState = HITID_START;
						m_nDrawState = HITID_START_DOWN;
						break;
					}
				case HITID_TXT_MAN_OVER:
					{
						m_nDrawState = HITID_TXT_MAN_DOWN;
						m_nClickState = HITID_TXT_MAN;
						break;
					}
				case HITID_TXT_WOMEN_OVER:
					{
						m_nDrawState = HITID_TXT_WOMEN_DOWN;
						m_nClickState = HITID_TXT_WOMEN;
						break;
					}
				case HITID_MAN_UNIFORM1:
					{
						m_nSelUniform = HITID_MAN_UNIFORM1;
						g_PSGame.m_Humen.m_iSkinId = 0;
						break;
					}
				case HITID_MAN_UNIFORM2:
					{
						m_nSelUniform = HITID_MAN_UNIFORM2;
						g_PSGame.m_Humen.m_iSkinId = 1;
						break;
					}
				case HITID_WOMEN_UNIFORM1:
					{
						m_nSelUniform = HITID_WOMEN_UNIFORM1;
						g_PSGame.m_Humen.m_iSkinId = 0;
						break;
					}
				case HITID_WOMEN_UNIFORM2:
					{
						m_nSelUniform = HITID_WOMEN_UNIFORM2;
						g_PSGame.m_Humen.m_iSkinId = 1;
						break;
					}
				case HITID_SAVE_OVER:
					{
						m_nClickState = HITID_SAVE;
						m_nDrawState = HITID_SAVE_DOWN;
						break;
					}
				case HITID_PLAY_OVER:
					{
						m_nClickState = HITID_PLAY;
						m_nDrawState = HITID_PLAY_DOWN;
						break;
					}
				case HITID_EXPORT_OVER:
					{
						m_nClickState = HITID_EXPORT;
						m_nDrawState = HITID_EXPORT_DOWN;
						break;
					}
				case HITID_MAINMENU_OVER:
					{
						m_nClickState = HITID_MAINMENU;
						m_nDrawState = HITID_MAINMENU_DOWN;
						break;
					}
				}
				((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			}
		}
		break;
	default:
		break;
	}
}
void CPSStage::OnRButtonDown(UINT nFlags, CPoint point)
{

}
void CPSStage::OnMouseMove(UINT nFlags, CPoint point)
{
	switch(g_PSGame.m_GameStage)
	{
	case SELECT_HUMEN:
		if ( !g_PSGame.m_Humen.m_bLoaded )
			return;
	case INIT:
	case SELECT_SCENE:
	case SHOW_PREVIEW:
	case STAND:
		{
			GLuint selectBuf[BUFFER_LENGTH];
			GLint hits = GetSelectHits(point.x, point.y, selectBuf);
			if(hits != 0)
			{
				int nOverIndex = selectBuf[hits * 4 - 1];
				RenderHighLight(nOverIndex);
			}
			else
			{
				m_nClickState = HITID_NONE;
				RenderHighLight(HITID_NONE);
			}
		}
		break;
	default:
		break;
	}
}

void CPSStage::OnLButtonUp(UINT nFlags, CPoint point)
{
	switch (m_nClickState)
	{
	case HITID_NEW:
		{
			g_PSGame.NewScenario();
			((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			break;
		}
	case HITID_SAVED:
		{
			CloseHandle ( CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)LoadScenario, 0, 0, 0 ) );
			m_nClickState = HITID_NONE;
			m_nDrawState = HITID_NONE;
			((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			break;
		}
	case HITID_EXIT:
		{
			g_PSGame.Quit();
			break;
		}
	case HITID_TXT_MAN:
		{
			g_PSGame.m_Humen.m_iSexId = false;
			m_nSelSex = HITID_TXT_MAN;
			if(m_nSelUniform == HITID_WOMEN_UNIFORM1)
				m_nSelUniform = HITID_MAN_UNIFORM1;
			else if(m_nSelUniform == HITID_WOMEN_UNIFORM2)
				m_nSelUniform = HITID_MAN_UNIFORM2;
			g_PSGame.m_Humen.m_iSkinId = (m_nSelUniform == HITID_MAN_UNIFORM1) ? 0 : 1;
			((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			break;
		}
	case HITID_TXT_WOMEN:
		{
			g_PSGame.m_Humen.m_iSexId = true;
			m_nSelSex = HITID_TXT_WOMEN;
			if(m_nSelUniform == HITID_MAN_UNIFORM1)
				m_nSelUniform = HITID_WOMEN_UNIFORM1;
			else if(m_nSelUniform == HITID_MAN_UNIFORM2)
				m_nSelUniform = HITID_WOMEN_UNIFORM2;
			g_PSGame.m_Humen.m_iSkinId = (m_nSelUniform == HITID_WOMEN_UNIFORM1) ? 0 : 1;
			((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			break;
		}
	case HITID_PREVIOUS:
		{
			switch(g_PSGame.m_GameStage)
			{
			case SELECT_SCENE:
				g_PSGame.MainMenu();
				break;
			case SELECT_HUMEN:
				g_PSGame.SelectScene();
				m_nDrawState = HITID_PREVIOUS_OVER;
				break;
			case SHOW_PREVIEW:
				g_PSGame.SelectHumen();
				m_nDrawState = HITID_PREVIOUS_OVER;
				break;
			default:
				break;
			}
			((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			break;
		}
	case HITID_NEXT:
		{
			switch(g_PSGame.m_GameStage)
			{
			case SELECT_SCENE:
				g_PSGame.SelectHumen();
				m_nDrawState = HITID_NEXT_OVER;
				break;
			case SELECT_HUMEN:
				CloseHandle ( CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)LoadSceneData, 0, 0, 0 ) );
				g_PSGame.ShowPreview();
				m_nDrawState = HITID_START_OVER;
				break;
			default:
				break;
			}
			((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			break;
		}
	case HITID_START:
		{
			g_PSGame.StartScenario();
			break;
		}
	case HITID_SAVE:
		{
			if(g_PSGame.IsEmptyScenario())
			{
				AfxMessageBox("You can't Save Scenario.\nBecause There is no any Recorded Scenario.\nYou try make new Scenario by Pressing the Space Key.", MB_OK | MB_ICONEXCLAMATION);
				break;
			}

			CloseHandle ( CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)SaveScenario, 0, 0, 0 ) );

			m_nClickState = HITID_NONE;
			m_nDrawState = HITID_NONE;
 			((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			break;
		}
	case HITID_PLAY:
		{
			if(g_PSGame.IsEmptyScenario())
			{
				AfxMessageBox("You can't Play Scenario.\nBecause There is no any Recorded Scenario.\nYou try make new Scenario by Pressing the Space Key.", MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			g_PSGame.PlayScenario();
			((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			break;
		}
	case HITID_EXPORT:
		{
			if(g_PSGame.IsEmptyScenario())
			{
				AfxMessageBox("You can't Export Scenario.\nBecause There is no any Recorded Scenario.\nYou try make new Scenario by Pressing the Space Key.", MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			g_PSGame.ExportToAvi();
			m_nClickState = HITID_NONE;
			m_nDrawState = HITID_NONE;
			break;
		}
	case HITID_MAINMENU:
		{
			g_PSGame.MainMenu();
			((CMainFrame*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
			break;
		}
	}
	m_nClickState = HITID_NONE;
}


