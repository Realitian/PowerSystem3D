#include "stdafx.h"
#include "Global.h"
#include "gl/GLU.h"
#include "PSEquipment.h"
#include "PSGame.h"

GLuint CPSEquipment::s_nTexHands[HAND_MAX];
GLuint CPSEquipment::s_nTexSpark;
GLuint CPSEquipment::s_nTexSmoke;
GLuint CPSEquipment::s_nTexShine;
GLuint CPSEquipment::s_nTexCoronas[CORONA_TEX_NUM];
GLuint CPSEquipment::s_nTexFume;

#define USE_ALPHA 0

CPSEquipment::CPSEquipment()
{
	// Init Member Variables
	m_pRoot = NULL;
	m_pCtrlPanel = NULL;
	m_pPanel = NULL;
	m_pBody = NULL;

	m_bboxMinW = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
	m_bboxMaxW = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	m_bboxPanelMinW = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
	m_bboxPanelMaxW = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	m_bboxBodyMinW = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
	m_bboxBodyMaxW = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	m_matModel = mat4_id;
	m_state = EQUIP_STATE_INIT;
	m_result = EQUIP_RESULT_NONE;

	m_fFrameTime = 0.f;

	m_curLamp = 0;
	m_fTime = 0.f;
	m_fLampTime = 0.f;
	m_curCtrl = CTRL_NONE;
	m_curHand = HAND_DEFAULT;
	m_posHand = vec3(0.f, 0.f, 0.f);
	InitCtrl();
}

CPSEquipment::~CPSEquipment()
{
	DeleteSpark();
	DeleteCorona();
	DeleteSmoke();
}

void CPSEquipment::Init(PSGemNode* pRoot)
{
	m_pRoot = pRoot;
	m_state = EQUIP_STATE_INIT;
	m_result = EQUIP_RESULT_NONE;

	SetCtrlGeo();
	InitStateMap();	
}

void CPSEquipment::SetType(EQUIP_TYPE type)
{
	m_type = type;
}

void CPSEquipment::CalcBoundingBox()
{
	if(m_pRoot == NULL)
		return;

	GetNodeBBox( m_pRoot, m_bboxMinW, m_bboxMaxW );
	m_bboxMinW = m_matModel * m_bboxMinW;
	m_bboxMaxW = m_matModel * m_bboxMaxW;
	SwapMinMax(m_bboxMinW, m_bboxMaxW);

	GetNodeBBox(m_pPanel, m_bboxPanelMinW, m_bboxPanelMaxW );
	m_bboxPanelMinW = m_matModel * m_bboxPanelMinW;
	m_bboxPanelMaxW = m_matModel * m_bboxPanelMaxW;
	SwapMinMax( m_bboxPanelMinW, m_bboxPanelMaxW );
	m_PanelCenter = 0.5f*(m_bboxPanelMinW + m_bboxPanelMaxW);
	vec3 panelSize = m_bboxPanelMaxW - m_bboxPanelMinW;
	m_PanelSize = panelSize.norm();

	GetNodeBBox(m_pBody, m_bboxBodyMinW, m_bboxBodyMaxW );
	m_bboxBodyMinW = m_matModel * m_bboxBodyMinW;
	m_bboxBodyMaxW = m_matModel * m_bboxBodyMaxW;
	SwapMinMax( m_bboxBodyMinW, m_bboxBodyMaxW );
	m_BodyCenter = 0.5f*(m_bboxBodyMinW + m_bboxBodyMaxW);
	m_OpViewDir = m_BodyCenter - m_PanelCenter;
	m_OpViewDir.normalize();
}

void CPSEquipment::InitCtrl()
{
	InitCtrlGeo();
	InitCtrlValue();
}
void CPSEquipment::InitCtrlGeo()
{
	// clear control node
	int i=0;

	for(i=0; i<BUTTON_NUM; i++)
	{
		m_buttons[i].m_nID = CTRL_BUTTON_1 + i;
		m_buttons[i].m_pNode = NULL;
		memset(m_buttons[i].m_pName, 0x00, sizeof(m_buttons[i].m_pName));
		sprintf(m_buttons[i].m_pName, "button%02d", i);
	}

	m_buttonOk.m_nID = CTRL_BUTTON_OK;
	m_buttonOk.m_pNode = NULL;
	memset(m_buttonOk.m_pName, 0x00, sizeof(m_buttonOk.m_pName));
	strcpy(m_buttonOk.m_pName, "button_ok");

	m_buttonRetry.m_nID = CTRL_BUTTON_RETRY;
	m_buttonRetry.m_pNode = NULL;
	memset(m_buttonRetry.m_pName, 0x00, sizeof(m_buttonRetry.m_pName));
	strcpy(m_buttonRetry.m_pName, "button_reset");

	for(i=0; i<SWITCH_NUM; i++)
	{
		m_switches[i].m_nID = CTRL_SWITCH_1 + i;
		m_switches[i].m_pNode = NULL;
		memset(m_switches[i].m_pName, 0x00, sizeof(m_switches[i].m_pName));
		sprintf(m_switches[i].m_pName, "switch%02d", i);
	}

	for(i=0; i<LAMP_NUM; i++)
	{
		m_lamps[i].m_pNode = NULL;
		memset(m_lamps[i].m_pName, 0x00, sizeof(m_lamps[i].m_pName));
		sprintf(m_lamps[i].m_pName, "light%02d", i);
	}

	for(i=0; i<DOOR_NUM; i++)
	{
		m_doors[i].m_pNode = NULL;
		memset(m_doors[i].m_pName, 0x00, sizeof(m_lamps[i].m_pName));
		if(i == DOOR_LEFT)
		{
			strcpy(m_doors[i].m_pName, "left_door");
		}
		else if(i == DOOR_RIGHT)
		{
			strcpy(m_doors[i].m_pName, "right_door");
		}
	}
}

void CPSEquipment::InitCtrlValue()
{
	// Init Controls
	int i=0;

	for(i=0; i<BUTTON_NUM; i++)
	{
		m_buttons[i].m_fValue = 0.f;
		m_buttons[i].m_matTrans = mat4_id;
	}

	m_buttonOk.m_fValue = 0.f;
	m_buttonOk.m_matTrans = mat4_id;

	m_buttonRetry.m_fValue = 0.f;
	m_buttonRetry.m_matTrans = mat4_id;

	for(i=0; i<SWITCH_NUM; i++)
	{
		m_switches[i].m_fValue = 0.f;
		m_switches[i].m_matTrans = mat4_id;
	}

	for(i=0; i<LAMP_NUM; i++)
	{
		m_lamps[i].m_fValue = 1.f;
		m_lamps[i].m_vColor = INIT_LAMP_COLOR; // blue color on init
	}

	for(i=0; i<DOOR_NUM; i++)
	{
		m_doors[i].m_fValue = 0.f;
		m_doors[i].m_matTrans = mat4_id;
	}
}

void CPSEquipment::SetCtrlGeo()
{
	int i=0;

	if(m_pRoot == NULL)
		return;

	for(i=0; i<BUTTON_NUM; i++)
	{
		if(m_buttons[i].m_pNode != NULL)
			continue;

		m_buttons[i].m_pNode = FindNodeByName(m_pRoot, m_buttons[i].m_pName);
	}

	m_buttonOk.m_pNode = FindNodeByName(m_pRoot, m_buttonOk.m_pName);
	m_buttonRetry.m_pNode = FindNodeByName(m_pRoot, m_buttonRetry.m_pName);

	for(i=0; i<SWITCH_NUM; i++)
	{
		if(m_switches[i].m_pNode != NULL)
			continue;

		m_switches[i].m_pNode = FindNodeByName(m_pRoot, m_switches[i].m_pName);
	}

	for(i=0; i<LAMP_NUM; i++)
	{
		if(m_lamps[i].m_pNode != NULL)
			continue;

		m_lamps[i].m_pNode = FindNodeByName(m_pRoot, m_lamps[i].m_pName);
	}

	for(i=0; i<DOOR_NUM; i++)
	{
		if(m_doors[i].m_pNode != NULL)
			continue;

		m_doors[i].m_pNode = FindNodeByName(m_pRoot, m_doors[i].m_pName);
	}

	m_pBody = FindNodeByName(m_pRoot, EQUIP_BODY_NAME);
	m_pCtrlPanel = FindNodeByName(m_pRoot, EQUIP_INSIDEGROUP_NAME);
	m_pPanel = FindNodeByName(m_pRoot, EQUIP_PANEL_NAME);
}

void CPSEquipment::InitStateMap()
{
	int i = 0;
	StateMap mapTemp;

	// success map
	for(i=CTRL_BUTTON_1; i<CTRL_MAX; i++)
	{
		mapTemp.insert(StateMap::value_type((CONTROL_ID)i, 1.f));
	}
	StateMap::iterator it = mapTemp.find(CTRL_BUTTON_2);
	if(it != mapTemp.end())
		(*it).second = 0.f;
	it = mapTemp.find(CTRL_BUTTON_4);
	if(it != mapTemp.end())
		(*it).second = 0.f;
	it = mapTemp.find(CTRL_BUTTON_6);
	if(it != mapTemp.end())
		(*it).second = 0.f;

	m_successTable.push_back(mapTemp);
	mapTemp.erase(mapTemp.begin(), mapTemp.end());

	// warning map
	for(i=CTRL_BUTTON_1; i<CTRL_MAX; i++)
	{
		mapTemp.insert(StateMap::value_type((CONTROL_ID)i, 0.f));
	}

	it = mapTemp.find(CTRL_BUTTON_1);
	if(it != mapTemp.end())
		(*it).second = 1.f;
	it = mapTemp.find(CTRL_BUTTON_6);
	if(it != mapTemp.end())
		(*it).second = 1.f;
	it = mapTemp.find(CTRL_SWITCH_2);
	if(it != mapTemp.end())
		(*it).second = 1.f;

	m_warningTable.push_back(mapTemp);
	mapTemp.erase(mapTemp.begin(), mapTemp.end());

	for(i=CTRL_BUTTON_1; i<CTRL_MAX; i++)
	{
		mapTemp.insert(StateMap::value_type((CONTROL_ID)i, 0.f));
	}

	it = mapTemp.find(CTRL_BUTTON_2);
	if(it != mapTemp.end())
		(*it).second = 1.f;
	it = mapTemp.find(CTRL_BUTTON_5);
	if(it != mapTemp.end())
		(*it).second = 1.f;
	it = mapTemp.find(CTRL_SWITCH_2);
	if(it != mapTemp.end())
		(*it).second = 1.f;

	m_warningTable.push_back(mapTemp);
	mapTemp.erase(mapTemp.begin(), mapTemp.end());

	for(i=CTRL_BUTTON_1; i<CTRL_MAX; i++)
	{
		mapTemp.insert(StateMap::value_type((CONTROL_ID)i, 0.f));
	}

	it = mapTemp.find(CTRL_BUTTON_3);
	if(it != mapTemp.end())
		(*it).second = 1.f;
	it = mapTemp.find(CTRL_BUTTON_4);
	if(it != mapTemp.end())
		(*it).second = 1.f;
	it = mapTemp.find(CTRL_SWITCH_2);
	if(it != mapTemp.end())
		(*it).second = 1.f;

	m_warningTable.push_back(mapTemp);
	mapTemp.erase(mapTemp.begin(), mapTemp.end());

	// fail map
	for(i=CTRL_BUTTON_1; i<CTRL_MAX; i++)
	{
		mapTemp.insert(StateMap::value_type((CONTROL_ID)i, 0.f));
	}

	for(i = CTRL_SWITCH_1; i<= CTRL_SWITCH_3; i++)
	{
		it = mapTemp.find((CONTROL_ID)i);
		if(it != mapTemp.end())
			(*it).second = 1.f;
	}

	m_failTable.push_back(mapTemp);
	mapTemp.erase(mapTemp.begin(), mapTemp.end());
}

void CPSEquipment::AddSpark()
{
	vec3 minPos = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
	vec3 maxPos = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	GetNodeBBox(m_pPanel, minPos, maxPos );

	vec3 centerPos = 0.5f * (minPos + maxPos);

	for(int i=0; i<SPARK_NUM; i++)
	{
		CSpark* pSpark = new CSpark;
		pSpark->Init(centerPos, this);

		m_vecSparks.push_back(pSpark);
	}
}

void CPSEquipment::DeleteSpark()
{
	while(!m_vecSparks.empty())
	{
		SAFE_DELETE(m_vecSparks.back());
		m_vecSparks.pop_back();
	}	
}

void CPSEquipment::AddCorona()
{
	vec3 minPos = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
	vec3 maxPos = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	GetNodeBBox(m_pPanel, minPos, maxPos );

	vec3 centerPos = 0.5f * (minPos + maxPos);
	for(int i=0; i<CORONA_NUM; i++)
	{
		CCorona* pCorona = new CCorona;

		pCorona->Init(centerPos, this);
		m_vecCoronas.push_back(pCorona);
	}
}

void CPSEquipment::DeleteCorona()
{
	while(!m_vecCoronas.empty())
	{
		SAFE_DELETE(m_vecCoronas.back());
		m_vecCoronas.pop_back();
	}	
}

void CPSEquipment::AddSmoke()
{
	for(int i=0; i<SMOKE_NUM; i++)
	{
		CSmoke* pSmoke = new CSmoke;

		vec3 minPos = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
		vec3 maxPos = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );
		GetNodeBBox(m_pBody, minPos, maxPos );
		minPos = m_matModel * minPos;
		maxPos = m_matModel * maxPos;
		SwapMinMax(minPos, maxPos);
		//minPos.z = maxPos.z;
		
		vec3 offset = maxPos - minPos;
#if USE_ALPHA
		float fLife = offset.norm()/(SMOKE_SPEED*4.f);
#else
		float fLife = offset.norm()/(SMOKE_SPEED/2.f);
#endif
		pSmoke->Init(minPos, maxPos, fLife, SMOKE_SIZE);

		m_vecSmokes.push_back(pSmoke);
	}
}

void CPSEquipment::DeleteSmoke()
{
	while(!m_vecSmokes.empty())
	{
		SAFE_DELETE(m_vecSmokes.back());
		m_vecSmokes.pop_back();
	}	
}

PSElement* CPSEquipment::GetControl(PSGemNode* pNode)
{
	int i = 0;

	if(pNode == NULL)
		return NULL;

	for(i=0; i<BUTTON_NUM; i++)
	{
		if(m_buttons[i].m_pNode == pNode)
		{
			return (PSElement*)&m_buttons[i];
		}
	}

	for(i=0; i<SWITCH_NUM; i++)
	{
		if(m_switches[i].m_pNode == pNode)
		{
			return (PSElement*)&m_switches[i];
		}
	}

	if(m_buttonOk.m_pNode == pNode)
	{
		return (PSElement*)&m_buttonOk;
	}

	if(m_buttonRetry.m_pNode == pNode)
	{
		return (PSElement*)&m_buttonRetry;
	}

	for(i=0; i<LAMP_NUM; i++)
	{
		if(m_lamps[i].m_pNode == pNode)
		{
			return (PSElement*)&m_lamps[i];
		}
	}

	for(i=0; i<DOOR_NUM; i++)
	{
		if(m_doors[i].m_pNode == pNode)
		{
			return (PSElement*)&m_doors[i];
		}
	}

	return NULL;
}

bool CPSEquipment::IsDoor(PSElement* pElem)
{
	if(pElem == NULL)
		return false;

	if((pElem >= m_doors) && (pElem < (PSElement*)((char*)m_doors + sizeof(m_doors))))
		return true;

	return false;
}

bool CPSEquipment::IsLamp(PSElement* pElem)
{
	if(pElem == NULL)
		return false;

	if((pElem >= (PSElement*)m_lamps) && (pElem < (PSElement*)((char*)m_lamps + sizeof(m_lamps))))
		return true;

	return false;
}

bool CPSEquipment::IsButton(PSElement* pElem)
{
	if(pElem == NULL)
		return false;

	if((pElem >= (PSElement*)m_buttons) && (pElem < (PSElement*)((char*)m_buttons + sizeof(m_buttons))))
		return true;

	return false;
}

bool CPSEquipment::IsSwitch(PSElement* pElem)
{
	if(pElem == NULL)
		return false;

	if((pElem >= (PSElement*)m_switches) && (pElem < (PSElement*)((char*)m_switches + sizeof(m_switches))))
		return true;

	return false;
}

void CPSEquipment::UpdateSpark(float fElapsedTime)
{
	for(vector<CSpark*>::iterator it = m_vecSparks.begin(); it != m_vecSparks.end(); it++)
	{
		CSpark* pSpark = (CSpark*)(*it);
		pSpark->Idle(fElapsedTime);
	}
}

void CPSEquipment::UpdateCorona(float fElapsedTime)
{
	for(vector<CCorona*>::iterator it = m_vecCoronas.begin(); it != m_vecCoronas.end(); it++)
	{
		CCorona* pCorona = (CCorona*)(*it);
		pCorona->Idle(fElapsedTime);
	}
}

void CPSEquipment::UpdateSmoke(float fElapsedTime)
{
	for(vector<CSmoke*>::iterator it = m_vecSmokes.begin(); it != m_vecSmokes.end(); it++)
	{
		CSmoke* pSmoke = (CSmoke*)(*it);
		pSmoke->Idle(fElapsedTime);
	}
}

void CPSEquipment::UpdateExplosion(float fElapsedTime)
{
}

void CPSEquipment::RenderHand()
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Projection
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport[2], 0, viewport[3], 0.f, 1.f);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

 	GLfloat fHandSize = HAND_SIZE * (GLfloat)viewport[2]/SCREEN_SIZE;
 	GLfloat fHandX = (GLfloat)m_posHand.x;
 	GLfloat fHandY = (GLfloat)viewport[3] - (GLfloat)m_posHand.y;
 
	glBindTexture(GL_TEXTURE_2D, s_nTexHands[m_curHand]);
 	if(m_curHand == HAND_DEFAULT)
 	{
 		glBegin(GL_QUADS);
 		// left bottom vertex
 		glTexCoord2f(0.f, 0.f);
 		glVertex3f(fHandX-fHandSize/2.f, fHandY-fHandSize, 0.f);
 		// right bottom vertex
 		glTexCoord2f(1.f, 0.f);
 		glVertex3f(fHandX+fHandSize/2.f, fHandY-fHandSize, 0.f);
 		// right top vertex
 		glTexCoord2f(1.f, 1.f);
 		glVertex3f(fHandX+fHandSize/2.f, fHandY, 0.f);
 		// left top vertex
 		glTexCoord2f(0.f, 1.f);
 		glVertex3f(fHandX-fHandSize/2.f, fHandY, 0.f);
 		glEnd();
 	}
 	else if(m_curHand == HAND_HOLD)
 	{
 		glBegin(GL_QUADS);
 		// left bottom vertex
 		glTexCoord2f(0.f, 0.f);
 		glVertex3f(fHandX-fHandSize/2.f, fHandY-fHandSize/3.f*2.f, 0.f);
 		// right bottom vertex
 		glTexCoord2f(1.f, 0.f);
 		glVertex3f(fHandX+fHandSize/2.f, fHandY-fHandSize/3.f*2.f, 0.f);
 		// right top vertex
 		glTexCoord2f(1.f, 1.f);
 		glVertex3f(fHandX+fHandSize/2.f, fHandY+fHandSize/3.f, 0.f);
 		// left top vertex
 		glTexCoord2f(0.f, 1.f);
 		glVertex3f(fHandX-fHandSize/2.f, fHandY+fHandSize/3.f, 0.f);
 		glEnd();
 	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CPSEquipment::RenderSmoke()
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);
	
#if USE_ALPHA
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#else
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#endif

	glBindTexture(GL_TEXTURE_2D, s_nTexSmoke);

	for(vector<CSmoke*>::iterator it = m_vecSmokes.begin(); it != m_vecSmokes.end(); it++)
	{
		CSmoke* pSmoke = (CSmoke*)(*it);
		pSmoke->Render();
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

void CPSEquipment::RenderSpark()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_DEPTH_TEST);

	for(vector<CSpark*>::iterator it = m_vecSparks.begin(); it != m_vecSparks.end(); it++)
	{
		CSpark* pSpark = (CSpark*)(*it);
		pSpark->Render();
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void CPSEquipment::RenderCorona()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	glDisable(GL_DEPTH_TEST);

	for(vector<CCorona*>::iterator it = m_vecCoronas.begin(); it != m_vecCoronas.end(); it++)
	{
		CCorona* pCorona = (CCorona*)(*it);
		pCorona->Render();
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void CPSEquipment::RenderExplosion()
{

}

void CPSEquipment::RenderGeo(P3DGeometry* pGeo, bool bOpaque, P3DMaterial* pMtrl)
{
	if ( pGeo == 0 )
		return;

	glDisable(GL_TEXTURE_2D );

	bool bTransparent = false;

	if(pMtrl)
		bTransparent = ( pMtrl->ambient[3] < 1 ) || ( pMtrl->emission[3] < 1 ) || ( pMtrl->diffuse[3] < 1 ) || ( pMtrl->specular[3] < 1 );
	else
		bTransparent = ( pGeo->m_Material.ambient[3] < 1 ) || ( pGeo->m_Material.emission[3] < 1 ) || ( pGeo->m_Material.diffuse[3] < 1 ) || ( pGeo->m_Material.specular[3] < 1 );

	if ( bOpaque )
	{
		if ( bTransparent )
			return;
		glDisable( GL_BLEND );
	}
	else
	{
		if ( !bTransparent )
			return;
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	if(pMtrl)
	{
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT , pMtrl->ambient );
		glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION , pMtrl->emission );
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE , pMtrl->diffuse );
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR ,  pMtrl->specular );
		glMaterialf ( GL_FRONT_AND_BACK , GL_SHININESS , pMtrl->shininess );
	}
	else
	{
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT , pGeo->m_Material.ambient );
		glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION , pGeo->m_Material.emission );
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE , pGeo->m_Material.diffuse );
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR ,  pGeo->m_Material.specular );
		glMaterialf ( GL_FRONT_AND_BACK , GL_SHININESS , pGeo->m_Material.shininess );
	}

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

	if ( pGeo->m_glDlistId )
		glCallList( pGeo->m_glDlistId );
	else
	{
		pGeo->m_glDlistId = glGenLists(	1 );
		glNewList( pGeo->m_glDlistId, GL_COMPILE );

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

		glEndList();
	}

	RenderGeo( pGeo->m_pNextMesh, bOpaque, pMtrl);
}

void CPSEquipment::RenderNode(PSGemNode* pNode, bool bOpaque)
{
	if(pNode == NULL)
		return;

	if (m_state == EQUIP_STATE_INIT && pNode == m_pCtrlPanel)
	{
		RenderNode(pNode->pSibling, bOpaque);
		return;
	}

	PSElement* pElement = GetControl(pNode);

	bool bLamp = IsLamp(pElement);
	bool bDoor = IsDoor(pElement);
	bool bControl = (pElement != NULL) && (bLamp == false) && (bDoor == false);
	P3DMaterial mtrl;

	glPushMatrix();
	glMultMatrixf(pNode->Trans.mat_array);
	if(bDoor || bControl)
	{
		PSCtrl* pCtrl = (PSCtrl*)pElement;
		glMultMatrixf(pCtrl->m_matTrans.mat_array);
	}
	if(bControl)
	{
		PSSelectCtrl* pSelecCtrl = (PSSelectCtrl*)pElement;
		glLoadName(pSelecCtrl->m_nID);
	}
	else
	{
		glLoadName(CTRL_NONE);
	}

	if(bLamp)
	{
		if(pNode->pFirstMesh)
		{
			PSLamp* pLamp = (PSLamp*)pElement;
			mtrl = pNode->pFirstMesh->m_Material;
//   			mtrl.ambient[0] = pLamp->m_fValue * pLamp->m_vColor[0];
//   			mtrl.ambient[1] = pLamp->m_fValue * pLamp->m_vColor[1];
//   			mtrl.ambient[2] = pLamp->m_fValue * pLamp->m_vColor[2];
 			mtrl.diffuse[0] = pLamp->m_fValue * pLamp->m_vColor[0];
 			mtrl.diffuse[1] = pLamp->m_fValue * pLamp->m_vColor[1];
 			mtrl.diffuse[2] = pLamp->m_fValue * pLamp->m_vColor[2];
 			mtrl.specular[0] = pLamp->m_fValue * pLamp->m_vColor[0];
 			mtrl.specular[1] = pLamp->m_fValue * pLamp->m_vColor[1];
 			mtrl.specular[2] = pLamp->m_fValue * pLamp->m_vColor[2];
//  			mtrl.emission[0] = pLamp->m_fValue * pLamp->m_vColor[0];
//  			mtrl.emission[1] = pLamp->m_fValue * pLamp->m_vColor[1];
//  			mtrl.emission[2] = pLamp->m_fValue * pLamp->m_vColor[2];

			RenderGeo(pNode->pFirstMesh, bOpaque, &mtrl);
		}
	}
	else
	{
		RenderGeo(pNode->pFirstMesh, bOpaque);
	}

	RenderNode(pNode->pFirstChild, bOpaque);
	glPopMatrix();

	RenderNode(pNode->pSibling, bOpaque);
}

void CPSEquipment::Render()
{
	glEnable(GL_LIGHTING);
	// Light values and coord inates
	GLfloat ambientLight[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat diffuseLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat specularLight[] = { 0.1f, 0.1f, 0.1f, 1.0f};
	
	// Setup and enable light 0
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specularLight);
	glDisable( GL_CULL_FACE );
	glEnable(GL_LIGHT0);
	
	RenderEquip();

	if ( g_PSGame.m_Scene.m_pCurEquip == this && g_PSGame.m_EquipStage == EQUIP_OP_OPERATING )
	{
		switch(m_state){
		case EQUIP_STATE_CONTROL:
		case EQUIP_STATE_RESULT:
		case EQUIP_STATE_WAITING:
			RenderShines();
			RenderHand();
			break;
		case EQUIP_STATE_SPARK:
			{
// 				RenderElectricLine();
 				if(m_type == EQUIP_TYPE_I)
 					RenderSpark();
 				else
					RenderCorona();
			}
			break;
		case EQUIP_STATE_SMOKE:
			{
// 				RenderElectricLine();
 				if(m_type == EQUIP_TYPE_I)
 					RenderSpark();
// 				else
					RenderCorona();
			}
			RenderSmoke();
			break;
		case EQUIP_STATE_EXPLOSION:
			RenderSmoke();
			RenderExplosion();
			break;
		}	
	}

	glMatrixMode(GL_MODELVIEW);
}

void CPSEquipment::RenderEquip()
{
#ifdef SHOW_EQUIP_BBOX
	glLineWidth(1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(1, 0, 0);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	glVertex3f( m_bboxMinW.x, m_bboxMinW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMinW.x, m_bboxMinW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMinW.x, m_bboxMaxW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMinW.x, m_bboxMaxW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMinW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMinW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMaxW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMaxW.y, m_bboxMinW.z );

	glVertex3f( m_bboxMinW.x, m_bboxMinW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMinW.x, m_bboxMinW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMinW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMinW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMinW.x, m_bboxMinW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMinW.x, m_bboxMinW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMinW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMinW.y, m_bboxMinW.z );

	glVertex3f( m_bboxMinW.x, m_bboxMinW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMinW.x, m_bboxMaxW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMaxW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMinW.y, m_bboxMinW.z );
	glVertex3f( m_bboxMinW.x, m_bboxMinW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMinW.x, m_bboxMaxW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMaxW.y, m_bboxMaxW.z );
	glVertex3f( m_bboxMaxW.x, m_bboxMinW.y, m_bboxMaxW.z );

	glEnd(); 
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

	glPushMatrix();
	glMultMatrixf(m_matModel.mat_array);
	// Initialize the names stack
	glInitNames();
	glPushName(CTRL_NONE);
	if(m_pRoot)
	{
		RenderNode(m_pRoot, true);
		RenderNode(m_pRoot, false);
	}
	glPopMatrix();
}

void CPSEquipment::RenderShine(int index)
{
	vec3 minPos = vec3( FLT_MAX, FLT_MAX, FLT_MAX );
	vec3 maxPos = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	vec4 modelPos;
	vec4 worldPos;

	if(m_lamps[index].m_pNode)
	{
		//GetNodeBBox(m_lamps[index].m_pNode, minPos, maxPos );
		minPos = m_lamps[index].m_pNode->bbMin;
		maxPos = m_lamps[index].m_pNode->bbMax;
 		vec3 centerPos = (minPos + maxPos)/2.f;
 		centerPos = m_matModel * centerPos;

		float xWidth = maxPos.x - minPos.x;
 		xWidth *= SHINE_SCALE;
 		float yWidth = maxPos.y - minPos.y;
 		yWidth *= SHINE_SCALE;
 		xWidth *= m_lamps[index].m_fValue;
 		yWidth *= m_lamps[index].m_fValue;

		vec3 color = m_lamps[index].m_fValue * m_lamps[index].m_vColor;

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixf(g_PSGame.m_Camera.getProjectMat().mat_array);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		// make Sprite
		mat4 matView = g_PSGame.m_Camera.getViewMat();
 		mat4 matViewInv = mat4_id;
 		invert(matViewInv, matView);
 		matViewInv.set_translation(centerPos);
 		glLoadMatrixf(matView.mat_array);
 		glMultMatrixf(matViewInv.mat_array);

		glBegin(GL_QUADS);

		glColor3f(color.x, color.y, color.z);
		glTexCoord2f(0.f, 0.f);
		glVertex3f(-xWidth/2.f, -yWidth/2.f, 0.f);
// 		glVertex3f(minPos.x, minPos.y, maxPos.z);

		glColor3f(color.x, color.y, color.z);
		glTexCoord2f(0.f, 1.f);
		glVertex3f(-xWidth/2.f, yWidth/2.f, 0.f);
// 		glVertex3f(minPos.x, maxPos.y, maxPos.z);

		glColor3f(color.x, color.y, color.z);
		glTexCoord2f(1.f, 1.f);
		glVertex3f(xWidth/2.f, yWidth/2.f, 0.f);
// 		glVertex3f(maxPos.x, maxPos.y, maxPos.z);

		glColor3f(color.x, color.y, color.z);
		glTexCoord2f(1.f, 0.f);
		glVertex3f(xWidth/2.f, -yWidth/2.f, 0.f);
// 		glVertex3f(maxPos.x, minPos.y, maxPos.z);
		glEnd();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
	
}

void CPSEquipment::RenderShines()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, s_nTexShine);

	for(int i=0; i<LAMP_NUM; i++)
	{
		RenderShine(i);
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

void CPSEquipment::Idle(float fElapsedTime)
{
	switch(m_state)
	{
	case EQUIP_STATE_OPEN_DOOR:
		{
			m_doors[DOOR_LEFT].m_fValue += fElapsedTime;
			m_doors[DOOR_RIGHT].m_fValue += fElapsedTime;
			if(m_doors[DOOR_LEFT].m_fValue >= 1.f || m_doors[DOOR_RIGHT].m_fValue >= 1.f)
			{
				m_doors[DOOR_LEFT].m_fValue = 1.f;
				m_doors[DOOR_RIGHT].m_fValue = 1.f;

				m_state = EQUIP_STATE_CONTROL;

				// Set cursor to center
				GLint viewPort[4];
				glGetIntegerv(GL_VIEWPORT, viewPort);
				m_posHand.x = (float)viewPort[2]/2.f;
				m_posHand.y = (float)viewPort[3]/2.f;
				m_posHand.z = 0.f;

				// Record
				CPoint point((int)m_posHand.x, (int)m_posHand.y);
				AddAction(WM_KEYDOWN, point, CTRL_NONE, 0.f);
			}

			float fDoorWidth = fabsf(m_doors[DOOR_LEFT].m_pNode->bbMax.x - m_doors[DOOR_LEFT].m_pNode->bbMin.x);
			vec3 trans(fDoorWidth*m_doors[DOOR_LEFT].m_fValue, 0.f, 0.f);
			m_doors[DOOR_LEFT].m_matTrans.set_translation(trans);

			fDoorWidth = fabsf(m_doors[DOOR_RIGHT].m_pNode->bbMax.x - m_doors[DOOR_RIGHT].m_pNode->bbMin.x);
			trans.x = fDoorWidth*m_doors[DOOR_RIGHT].m_fValue;
			m_doors[DOOR_RIGHT].m_matTrans.set_translation(trans);
		}
		break;
	case EQUIP_STATE_CLOSE_DOOR:
		{
			m_doors[DOOR_LEFT].m_fValue -= fElapsedTime;
			m_doors[DOOR_RIGHT].m_fValue -= fElapsedTime;
			if(m_doors[DOOR_LEFT].m_fValue <= 0.f || m_doors[DOOR_RIGHT].m_fValue <= 0.f)
			{
				OnRetry();
				m_doors[DOOR_LEFT].m_fValue = 0.f;
				m_doors[DOOR_RIGHT].m_fValue = 0.f;

				m_state = EQUIP_STATE_INIT;
			}

			float fDoorWidth = fabsf(m_doors[DOOR_LEFT].m_pNode->bbMax.x - m_doors[DOOR_LEFT].m_pNode->bbMin.x);
			vec3 trans(fDoorWidth*m_doors[DOOR_LEFT].m_fValue, 0.f, 0.f);
			m_doors[DOOR_LEFT].m_matTrans.set_translation(trans);

			fDoorWidth = fabsf(m_doors[DOOR_RIGHT].m_pNode->bbMax.x - m_doors[DOOR_RIGHT].m_pNode->bbMin.x);
			trans.x = fDoorWidth*m_doors[DOOR_RIGHT].m_fValue;
			m_doors[DOOR_RIGHT].m_matTrans.set_translation(trans);
		}
		break;
	case EQUIP_STATE_RESULT:
		{
			m_fTime += fElapsedTime;
			m_fLampTime += fElapsedTime;

			switch(m_result)
			{
			case EQUIP_RESULT_NONE:
				{
					for(int i=0; i<LAMP_NUM; i++)
					{
						if(m_fTime < BLINKING_INTERVAL)
							m_lamps[i].m_fValue = m_fTime / BLINKING_INTERVAL;
						else
							m_lamps[i].m_fValue = 2.f - m_fTime / BLINKING_INTERVAL;
						m_lamps[i].m_vColor = SUCCESS_LAMP_COLOR;
					}

					if(m_fTime >= BLINKING_INTERVAL*2.f)
					{
						m_fTime = 0.f;
						OnRetry();
					}

					return;
				}
				break;
			case EQUIP_RESULT_SUCCESS:
				{
					for(int i=0; i<LAMP_NUM; i++)
					{
						m_lamps[i].m_fValue = 1.f;
						m_lamps[i].m_vColor = INIT_LAMP_COLOR;
					}

					m_lamps[m_curLamp].m_fValue = (m_fLampTime / TURN_INTERVAL);
					m_lamps[m_curLamp].m_vColor = SUCCESS_LAMP_COLOR;

					if(m_fLampTime > TURN_INTERVAL)
					{
						m_fLampTime = 0.f;
						m_curLamp ++;
						m_curLamp %= LAMP_NUM;
					}
				}
				break;
			case EQUIP_RESULT_WARNING:
			case EQUIP_RESULT_FAIL:
				{
					for(int i=0; i<LAMP_NUM; i++)
					{
						m_lamps[i].m_fValue = m_fLampTime / BLINKING_INTERVAL;
						if(m_result == EQUIP_RESULT_WARNING)
							m_lamps[i].m_vColor = WARING_LAMP_COLOR;
						else if(m_result == EQUIP_RESULT_FAIL)
							m_lamps[i].m_vColor = FAIL_LAMP_COLOR;
					}

					if(m_fLampTime > BLINKING_INTERVAL)
					{
						m_fLampTime = 0.f;
					}
				}
				break;
			}

			if(m_fTime > LIGHTING_TIME)
			{
				m_fTime = 0.f;
				m_fLampTime = 0.f;

				for(int i=0; i<LAMP_NUM; i++)
				{
					m_lamps[i].m_fValue = 1.f;
					if(m_result == EQUIP_RESULT_SUCCESS)
						m_lamps[i].m_vColor = SUCCESS_LAMP_COLOR;
					else if(m_result == EQUIP_RESULT_WARNING)
						m_lamps[i].m_vColor = WARING_LAMP_COLOR;
					else if(m_result == EQUIP_RESULT_FAIL)
						m_lamps[i].m_vColor = FAIL_LAMP_COLOR;
				}

				m_state = EQUIP_STATE_WAITING;
			}
		}
		break;
	case EQUIP_STATE_WAITING:
		{
			if(m_result == EQUIP_RESULT_FAIL)
			{
				m_fTime += fElapsedTime;
				if(m_fTime > WAITING_TIME)
				{
					m_fTime = 0.f;
					for(int i=0; i<LAMP_NUM; i++)
					{
						m_lamps[i].m_fValue = 1.f;
						m_lamps[i].m_vColor = SCORCH_COLOR;
					}
					m_state = EQUIP_STATE_SPARK;

 					if(m_type == EQUIP_TYPE_I)
 						AddSpark();
 					else
						AddCorona();					
				}
			}
		}
		break;
	case EQUIP_STATE_SPARK:
		{
			m_fTime += fElapsedTime;
			
 			if(m_type == EQUIP_TYPE_I)
 				UpdateSpark(fElapsedTime);
 			else
				UpdateCorona(fElapsedTime);
			
			if(m_fTime > SPARKLE_TIME)
			{
				m_fTime = 0.f;
				m_state = EQUIP_STATE_SMOKE;
				AddSmoke();
			}
		}
		break;
	case EQUIP_STATE_SMOKE:
		{
			m_fTime += fElapsedTime;

 			if(m_type == EQUIP_TYPE_I)
 				UpdateSpark(fElapsedTime);
 			else
				UpdateCorona(fElapsedTime);

			UpdateSmoke(fElapsedTime);

			if(m_fTime > SMOKE_TIME)
			{
				m_fTime = 0.f;
				m_state = EQUIP_STATE_EXPLOSION;

 				if(m_type == EQUIP_TYPE_I)
 					DeleteSpark();
 				else
					DeleteCorona();
			}
		}
		break;
	case EQUIP_STATE_EXPLOSION:
		{
			m_fTime += fElapsedTime;
			UpdateSmoke(fElapsedTime);
			UpdateExplosion(fElapsedTime);

			if(m_fTime > EXPLOSION_TIME)
			{
				m_fTime = 0.f;
				DeleteSmoke();
				Reset();
				g_PSGame.m_EquipStage = EQUIP_OP_NONE;
			}
		}
		break;
	default:
		break;
	}
}

CONTROL_ID CPSEquipment::GetSelectHits(LONG x, LONG y)
{
	GLuint selectBuf[BUFFER_LENGTH];
	GLint hits = 0, viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(BUFFER_LENGTH, selectBuf);
	glRenderMode(GL_SELECT);
	glEnable(GL_DEPTH_TEST);

	// Projection
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix((double)x, (double)(viewport[3] - y), 2.f, 2.f, viewport);
	glMultMatrixf(g_PSGame.m_Camera.getProjectMat().mat_array);

	// Render
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixf(g_PSGame.m_Camera.getViewMat().mat_array);
	RenderEquip();
	hits = glRenderMode(GL_RENDER);

	// Restore
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	// GetControl ID
	CONTROL_ID ctrlID = CTRL_NONE;
	if(hits > 0)
	{

		int blockindex = 0;
		int itemindex = 0;
		for(blockindex=0; blockindex<hits; blockindex++)
		{
			int nCount = selectBuf[itemindex];
			ctrlID = (CONTROL_ID)selectBuf[itemindex + 3];

			if(ctrlID != CTRL_NONE)
				break;

			itemindex += 4;
		}
	}

	return ctrlID;
}

void CPSEquipment::AddAction(short actionType, CPoint point, CONTROL_ID ctrlID, float ctrlValue)
{
	if(g_PSGame.m_GameStage == NAVIGATING 
		&& g_PSGame.m_Scene.m_pCurEquip == this 
		&& g_PSGame.m_EquipStage == EQUIP_OP_OPERATING)
	{

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		float x = (float)point.x / (float)viewport[2];
		float y = (float)point.y / (float)viewport[3];
		CPSEquipActionRecord action;
		action.opType = actionType;

		if(actionType == WM_KEYDOWN) // Start of controling
		{
			m_fFrameTime = g_PSGame.m_fElapsedFrameTime;
		}
		else
		{
			m_fFrameTime += CONTROL_ELAPSED_TIME;
		}

		g_PSGame.m_fElapsedFrameTime = m_fFrameTime;
		action.fTimeAt = g_PSGame.m_fElapsedFrameTime;
		action.fMouseX = x;
		action.fMouseY = y;
		action.iCtrlID = ctrlID;
		action.fCtrlValue = ctrlValue;

		g_PSGame.m_EquipRecordManager.PushAction( action );
	}
}

void CPSEquipment::SetCtrlValue(short actionType, CONTROL_ID ctrlID, float ctrlValue)
{
	if(ctrlID >= CTRL_SWITCH_1 && ctrlID <= CTRL_SWITCH_3)
	{
		int index = ctrlID - CTRL_SWITCH_1;
		if(m_switches[index].m_fValue != ctrlValue)
		{
			m_switches[index].m_fValue = ctrlValue;
			vec3 Axis(0.f, 1.f, 0.f);
			m_switches[index].m_matTrans.set_rot(-m_switches[index].m_fValue * ps_pi, Axis);
		}
	}

	if(ctrlID >= CTRL_BUTTON_1 && ctrlID <= CTRL_BUTTON_6)
	{
		int index = ctrlID - CTRL_BUTTON_1;

		if(m_buttons[index].m_fValue != ctrlValue)
		{
			m_buttons[index].m_fValue = ctrlValue;
			float fsize = m_buttons[index].m_pNode->bbMax.z - m_buttons[index].m_pNode->bbMin.z;
			m_buttons[index].m_matTrans._42 = -fsize* m_buttons[index].m_fValue;

		}
	}

	if(ctrlID == CTRL_BUTTON_OK)
	{
		if(m_buttonOk.m_fValue != ctrlValue)
		{
			m_buttonOk.m_fValue = ctrlValue;
			float fsize = m_buttonOk.m_pNode->bbMax.z - m_buttonOk.m_pNode->bbMin.z;
			m_buttonOk.m_matTrans._42 = -fsize* m_buttonOk.m_fValue;

			if(actionType == WM_LBUTTONUP)
			{
				OnOK();
			}
		}
	}

	if(ctrlID == CTRL_BUTTON_RETRY)
	{
		if(m_buttonRetry.m_fValue != ctrlValue)
		{
			m_buttonRetry.m_fValue = ctrlValue;
			float fsize = m_buttonRetry.m_pNode->bbMax.z - m_buttonRetry.m_pNode->bbMin.z;
			m_buttonRetry.m_matTrans._42 = -fsize* m_buttonRetry.m_fValue;

			if(actionType == WM_LBUTTONUP)
			{
				OnRetry();
			}
		}
	}
}

void CPSEquipment::OnMouseMove(UINT nFlags, CPoint point)
{
	m_posHand.x = (float)point.x;
	m_posHand.y = (float)point.y;

	if(m_state == EQUIP_STATE_CONTROL)
	{
		if(nFlags & MK_LBUTTON)
		{
			if(m_curCtrl >= CTRL_SWITCH_1 && m_curCtrl <= CTRL_SWITCH_3)
			{
				int index = m_curCtrl - CTRL_SWITCH_1;
				CPoint offset = m_switchPos - point;
				float orgValue = m_switches[index].m_fValue;
				m_switches[index].m_fValue += (float)offset.y*0.4f;

				if(m_switches[index].m_fValue > 0.4f)
					m_switches[index].m_fValue = 0.4f;
				else if(m_switches[index].m_fValue < -0.4f)
					m_switches[index].m_fValue = -0.4f;

				// Record
				if((orgValue != m_switches[index].m_fValue) 
					&& (m_switches[index].m_fValue == 0.4f || m_switches[index].m_fValue == -0.4f))
				{
					AddAction(WM_MOUSEMOVE, point, m_curCtrl, m_switches[index].m_fValue);
				}

				vec3 Axis(0.f, 1.f, 0.f);
				m_switches[index].m_matTrans.set_rot(-m_switches[index].m_fValue * ps_pi, Axis);
				m_switchPos = point;
			}
		}
		else
		{
			CONTROL_ID curCtrl = GetSelectHits(point.x, point.y);
			if(curCtrl >= CTRL_SWITCH_1 && curCtrl <= CTRL_SWITCH_3)
				m_curHand = HAND_HOLD;
			else
				m_curHand = HAND_DEFAULT;
		}
	}
}

void CPSEquipment::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_posHand.x = (float)point.x;
	m_posHand.y = (float)point.y;
	m_posHand.z = 1.f;

	CONTROL_ID curCtrl = GetSelectHits(point.x, point.y);

	switch(m_state)
	{
	case EQUIP_STATE_CONTROL:
		{
			if(curCtrl >= CTRL_SWITCH_1 && curCtrl <= CTRL_SWITCH_3)
			{
				m_curCtrl = curCtrl;
				int index = curCtrl - CTRL_SWITCH_1;
				m_switchPos = point;
				// Record
				AddAction(WM_LBUTTONDOWN, point, curCtrl, m_switches[index].m_fValue);
			}
			else if(curCtrl >= CTRL_BUTTON_1 && curCtrl <= CTRL_BUTTON_6)
			{
				int index = curCtrl - CTRL_BUTTON_1;

				m_buttons[index].m_fValue = 1.f - m_buttons[index].m_fValue;
				float fsize = m_buttons[index].m_pNode->bbMax.z - m_buttons[index].m_pNode->bbMin.z;
				m_buttons[index].m_matTrans._42 = -fsize* m_buttons[index].m_fValue;
				// Record
				AddAction(WM_LBUTTONDOWN, point, curCtrl, m_buttons[index].m_fValue);
			}
			else if(curCtrl == CTRL_BUTTON_OK)
			{
				m_curCtrl = curCtrl;
				m_buttonOk.m_fValue = 1.f - m_buttonOk.m_fValue;
				float fsize = m_buttonOk.m_pNode->bbMax.z - m_buttonOk.m_pNode->bbMin.z;
				m_buttonOk.m_matTrans._42 = -fsize* m_buttonOk.m_fValue;
				// Record
				AddAction(WM_LBUTTONDOWN, point, CTRL_BUTTON_OK, m_buttonOk.m_fValue);
			}
			else if(curCtrl == CTRL_BUTTON_RETRY)
			{
				m_curCtrl = curCtrl;
				m_buttonRetry.m_fValue = 1.f - m_buttonRetry.m_fValue;
				float fsize = m_buttonRetry.m_pNode->bbMax.z - m_buttonRetry.m_pNode->bbMin.z;
				m_buttonRetry.m_matTrans._42 = -fsize* m_buttonRetry.m_fValue;
				// Record
				AddAction(WM_LBUTTONDOWN, point, CTRL_BUTTON_RETRY, m_buttonRetry.m_fValue);
			}
		}
		break;
	case EQUIP_STATE_RESULT:
	case EQUIP_STATE_WAITING:
		{
			if(curCtrl == CTRL_BUTTON_OK)
			{
				m_curCtrl = curCtrl;
				m_buttonOk.m_fValue = 1.f - m_buttonOk.m_fValue;
				float fsize = m_buttonOk.m_pNode->bbMax.z - m_buttonOk.m_pNode->bbMin.z;
				m_buttonOk.m_matTrans._42 = -fsize* m_buttonOk.m_fValue;
				// Record
				AddAction(WM_LBUTTONDOWN, point, CTRL_BUTTON_OK, m_buttonOk.m_fValue);
			}
			else if(curCtrl == CTRL_BUTTON_RETRY)
			{
				m_curCtrl = curCtrl;
				m_buttonRetry.m_fValue = 1.f - m_buttonRetry.m_fValue;
				float fsize = m_buttonRetry.m_pNode->bbMax.z - m_buttonRetry.m_pNode->bbMin.z;
				m_buttonRetry.m_matTrans._42 = -fsize* m_buttonRetry.m_fValue;
				// Record
				AddAction(WM_LBUTTONDOWN, point, CTRL_BUTTON_RETRY, m_buttonRetry.m_fValue);
			}
			else 
			{
				if(curCtrl > CTRL_NONE
					&& g_PSGame.m_GameStage == NAVIGATING 
					&& g_PSGame.m_Scene.m_pCurEquip == this 
					&& g_PSGame.m_EquipStage == EQUIP_OP_OPERATING)
				{
					ShowCursor( TRUE );
					AfxMessageBox("Push reset button at first for next operation of any controls of panel.", MB_OK | MB_ICONEXCLAMATION);
					ShowCursor( FALSE );
				}
			}
		}
		break;
	}
}

void CPSEquipment::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_posHand.x = (float)point.x;
	m_posHand.y = (float)point.y;
	m_posHand.z = 0.f;

	if(m_curCtrl == CTRL_BUTTON_OK)
	{
		m_buttonOk.m_fValue = 1.f - m_buttonOk.m_fValue;
		float fsize = m_buttonOk.m_pNode->bbMax.z - m_buttonOk.m_pNode->bbMin.z;
		m_buttonOk.m_matTrans._42 = -fsize* m_buttonOk.m_fValue;
		// Record
		AddAction(WM_LBUTTONUP, point, CTRL_BUTTON_OK, m_buttonOk.m_fValue);
		OnOK();
	}

	if(m_curCtrl == CTRL_BUTTON_RETRY)
	{
		m_buttonRetry.m_fValue = 1.f - m_buttonRetry.m_fValue;
		float fsize = m_buttonRetry.m_pNode->bbMax.z - m_buttonRetry.m_pNode->bbMin.z;
		m_buttonRetry.m_matTrans._42 = -fsize* m_buttonRetry.m_fValue;
		// Record
		AddAction(WM_LBUTTONUP, point, CTRL_BUTTON_RETRY, m_buttonRetry.m_fValue);
		OnRetry();
	}

	m_curCtrl = CTRL_NONE;
}

void CPSEquipment::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

}

void CPSEquipment::OnRetry()
{
	int i = 0;

	if(m_state == EQUIP_STATE_RESULT || m_state == EQUIP_STATE_WAITING || m_state == EQUIP_STATE_CLOSE_DOOR)
	{
		for(i=0; i<LAMP_NUM; i++)
		{
			m_lamps[i].m_fValue = 1.f;
			m_lamps[i].m_vColor = INIT_LAMP_COLOR; // blue color on init
		}

		m_state = EQUIP_STATE_CONTROL;
		m_result = EQUIP_RESULT_NONE;
		m_curCtrl = CTRL_NONE;
		m_curLamp = 0;
		m_fTime = 0.f;
		m_fLampTime = 0.f;
		m_curHand = HAND_DEFAULT;
	}
}

void CPSEquipment::Reset()
{
	m_state = EQUIP_STATE_INIT;
	m_result = EQUIP_RESULT_NONE;
	m_curCtrl = CTRL_NONE;
	m_curLamp = 0;
	m_fTime = 0.f;
	m_fLampTime = 0.f;
	m_curHand = HAND_DEFAULT;

	InitCtrlValue();
}

void CPSEquipment::ToggleDoor()
{
	if ( m_state == EQUIP_STATE_INIT )
	{
		m_state = EQUIP_STATE_OPEN_DOOR;
		return;
	}

	if ( m_state < EQUIP_STATE_SPARK)
	{
		m_state = EQUIP_STATE_CLOSE_DOOR;
		return;
	}
}

void CPSEquipment::OnOK()
{
	if(m_state == EQUIP_STATE_CONTROL)
	{
		m_curLamp = 0;
		m_fTime = 0.f;
		m_fLampTime = 0.f;

		if(IsSuccess())
		{
			m_state = EQUIP_STATE_RESULT;
			m_result = EQUIP_RESULT_SUCCESS;
		}
		else if(IsWarning())
		{
			m_state = EQUIP_STATE_RESULT;
			m_result = EQUIP_RESULT_WARNING;
		}
		else if(IsFail())
		{
			m_state = EQUIP_STATE_RESULT;
			m_result = EQUIP_RESULT_FAIL;
		}
		else
		{
			m_state = EQUIP_STATE_RESULT;
			m_result = EQUIP_RESULT_NONE;
		}
	}
}

bool CPSEquipment::IsSuccess()
{
	bool bResult = false;
	StateTable::iterator itTable;
	for(itTable = m_successTable.begin(); itTable != m_successTable.end(); itTable++)
	{
		bResult = true;
		
		StateMap successMap = (*itTable);
		StateMap::iterator itMap;

		int i = 0;

		// Check Buttons
		for(i=0; i<BUTTON_NUM; i++)
		{
			itMap = successMap.find((CONTROL_ID)m_buttons[i].m_nID);
			if(itMap != successMap.end())
			{
				GLfloat fValue = itMap->second;
				bResult = bResult && (m_buttons[i].m_fValue == fValue);
			}
		}

		// Check Switches
		for(i=0; i<SWITCH_NUM; i++)
		{
			itMap = successMap.find((CONTROL_ID)m_switches[i].m_nID);
			if(itMap != successMap.end())
			{
				GLfloat fValue = itMap->second;
				GLfloat fCtrlValue = (m_switches[i].m_fValue <= 0.f) ? 0.f : 1.f;
				bResult = bResult && (fCtrlValue == fValue);
			}
		}

		if(bResult)
			break;
	}

	return bResult;
}

bool CPSEquipment::IsWarning()
{
	bool bResult = false;
	StateTable::iterator itTable;
	for(itTable = m_warningTable.begin(); itTable != m_warningTable.end(); itTable++)
	{
		bResult = true;

		StateMap warningMap = (*itTable);
		StateMap::iterator itMap;

		int i = 0;

		// Check Buttons
		for(i=0; i<BUTTON_NUM; i++)
		{
			itMap = warningMap.find((CONTROL_ID)m_buttons[i].m_nID);
			if(itMap != warningMap.end())
			{
				GLfloat fValue = itMap->second;
				bResult = bResult && (m_buttons[i].m_fValue == fValue);
			}
		}

		// Check Switches
		for(i=0; i<SWITCH_NUM; i++)
		{
			itMap = warningMap.find((CONTROL_ID)m_switches[i].m_nID);
			if(itMap != warningMap.end())
			{
				GLfloat fValue = itMap->second;
				GLfloat fCtrlValue = (m_switches[i].m_fValue <= 0.f) ? 0.f : 1.f;
				bResult = bResult && (fCtrlValue == fValue);
			}
		}

		if(bResult)
			break;
	}

	return bResult;
}

bool CPSEquipment::IsFail()
{
	bool bResult = false;
	StateTable::iterator itTable;
	for(itTable = m_failTable.begin(); itTable != m_failTable.end(); itTable++)
	{
		bResult = true;

		StateMap failMap = (*itTable);
		StateMap::iterator itMap;

		int i = 0;

		// Check Buttons
		for(i=0; i<BUTTON_NUM; i++)
		{
			itMap = failMap.find((CONTROL_ID)m_buttons[i].m_nID);
			if(itMap != failMap.end())
			{
				GLfloat fValue = itMap->second;
				bResult = bResult && (m_buttons[i].m_fValue == fValue);
			}
		}

		// Check Switches
		for(i=0; i<SWITCH_NUM; i++)
		{
			itMap = failMap.find((CONTROL_ID)m_switches[i].m_nID);
			if(itMap != failMap.end())
			{
				GLfloat fValue = itMap->second;
				GLfloat fCtrlValue = (m_switches[i].m_fValue <= 0.f) ? 0.f : 1.f;
				bResult = bResult && (fCtrlValue == fValue);
			}
		}

		if(bResult)
			break;
	}

	return bResult;
}


float GetRandomFloat(float fMin, float fMax)
{
	if(fMax <= fMin)
		return fMin;

	float fInterval = (float)(rand())/(float)RAND_MAX;

	return fMin + (fMax-fMin)*fInterval;
}

void GetRandomVector(vec3 &out, vec3 minPos, vec3 maxPos)
{
	SwapMinMax(minPos, maxPos);
	out.x = GetRandomFloat(minPos.x, maxPos.x);
	out.y = GetRandomFloat(minPos.y, maxPos.y);
	out.z = GetRandomFloat(minPos.z, maxPos.z);
}

//////////////////////////////////////////////////////////////////////////
// Class CEquipParticle
//////////////////////////////////////////////////////////////////////////
CEquipParticle::CEquipParticle()
{
	m_pParent = NULL;
	m_pPoints = NULL;
}

CEquipParticle::~CEquipParticle()
{
	SAFE_DELETE_ARRAY(m_pPoints);
}

void CEquipParticle::Init(vec3 org, CPSEquipment* pEquip, int nNumber, vec3 color, vec3 fadeColor, float fSize, float fSpeed, float fLife)
{
	m_nNumber = nNumber;
	
	m_position = org;
	m_color = color;
	m_fadeColor = fadeColor;
	m_fSpeed = fSpeed;
	m_fSize = fSize;
	m_fLife = fLife;

	m_pParent = pEquip;
	
	m_pPoints = new PointInfo[m_nNumber];

	ResetPoints();
}

void CEquipParticle::Idle(float fElapsedTime)
{
	if(m_pPoints)
	{
		for(int i=0; i<m_nNumber; i++)
		{
			UpdatePoint(i, fElapsedTime);
		}
	}
}

void CEquipParticle::Render()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// make sprite
	mat4 matView = g_PSGame.m_Camera.getViewMat();
	// 	mat4 matViewInv;
	// 	invert(matViewInv, matView);
	// 	matViewInv.set_translation(m_posCurrent);
	glLoadMatrixf(matView.mat_array);
	// 	glMultMatrixf(matViewInv.mat_array);
	glMultMatrixf(m_pParent->m_matModel.mat_array);

	if(m_pPoints)
	{
		for(int i=0; i<m_nNumber; i++)
		{
			RenderPoint(i);
		}
	}
	glPopMatrix();
}

void CEquipParticle::ResetPoints()
{
	if(m_pPoints)
	{
		for(int i=0; i<m_nNumber; i++)
		{
			ResetPoint(i);
		}
	}
}

void CEquipParticle::ResetPoint(int index)
{
	if(m_pPoints)
	{
		m_pPoints[index].position = m_position;
		float fAngle = GetRandomFloat(0.f, ps_two_pi);
		m_pPoints[index].velocity = vec3(cosf(fAngle), sinf(fAngle), 0.f);
		m_pPoints[index].speed = m_fSpeed;
		m_pPoints[index].size = 0.f;
		m_pPoints[index].life = GetRandomFloat(m_fLife * 0.5f, m_fLife*2.f);
		m_pPoints[index].burntime = GetRandomFloat(0.f, m_pPoints[index].life);
		m_pPoints[index].age = 0.f;
		m_pPoints[index].color = m_color;
	}
}

void CEquipParticle::UpdatePoint(int index, float fElapsedTime)
{
	if(m_pPoints[index].burntime <= 0)
	{
		m_pPoints[index].age += fElapsedTime;
		m_pPoints[index].position +=  fElapsedTime * m_pPoints[index].speed * m_pPoints[index].velocity ;
	}
	else
	{
		m_pPoints[index].burntime -= fElapsedTime;
	}
}

void CEquipParticle::RenderPoint(int index)
{
	vec3 axis(-m_pPoints[index].velocity.y, m_pPoints[index].velocity.x, 0);

	vec3 vertex0 = m_pPoints[index].position - m_pPoints[index].size * m_pPoints[index].velocity  - m_pPoints[index].size * axis;
	vec3 vertex1 = m_pPoints[index].position - m_pPoints[index].size * m_pPoints[index].velocity  + m_pPoints[index].size * axis;
	vec3 vertex2 = m_pPoints[index].position + m_pPoints[index].size * m_pPoints[index].velocity  + m_pPoints[index].size * axis;
	vec3 vertex3 = m_pPoints[index].position + m_pPoints[index].size * m_pPoints[index].velocity  - m_pPoints[index].size * axis;

	glColor4f(m_pPoints[index].color.x, m_pPoints[index].color.y, m_pPoints[index].color.z, m_pPoints[index].alpha);

	glBegin(GL_QUADS);

	glTexCoord2f(0.f, 0.f);
	glVertex3f(vertex0.x, vertex0.y, vertex0.z);

	glTexCoord2f(0.f, 1.f);
	glVertex3f(vertex1.x, vertex1.y, vertex1.z);

	glTexCoord2f(1.f, 1.f);
	glVertex3f(vertex2.x, vertex2.y, vertex2.z);

	glTexCoord2f(1.f, 0.f);
	glVertex3f(vertex3.x, vertex3.y, vertex3.z);

	glEnd();
}

//////////////////////////////////////////////////////////////////////////
// Class CSpark
//////////////////////////////////////////////////////////////////////////

CSpark::CSpark()
{
	m_pRockets = NULL;
}

CSpark::~CSpark()
{
	SAFE_DELETE_ARRAY(m_pRockets);
}

void CSpark::Init(vec3 org, CPSEquipment* pEquip)
{
	m_position = org;
	m_pParent = pEquip;
	m_pRockets = new CRocket[NUM_ROCKET];

	ResetRockets();
}

void CSpark::ResetRockets()
{
	for(int i=0; i<NUM_ROCKET; i++)
	{
		float fAngle = GetRandomFloat(0.f, ps_two_pi);
		vec3 direction(cosf(fAngle), sinf(fAngle), 0.f);
		m_pRockets[i].Init(m_position, direction, m_pParent);
	}
}

void CSpark::Idle(float fElapsedTime)
{
	if(m_pRockets)
	{
		for(int i=0; i<NUM_ROCKET; i++)
		{
			if(m_pRockets[i].IsDead())
			{
				float fAngle = GetRandomFloat(0.f, ps_two_pi);
				vec3 direction(cosf(fAngle), sinf(fAngle), 0.f);
				m_pRockets[i].Init(m_position, direction, m_pParent);
			}

			m_pRockets[i].Idle(fElapsedTime);
		}

// 		if(IsDead())
// 		{
// 			ResetRockets();
// 		}
	}
}

void CSpark::Render()
{
	if(m_pRockets)
	{
		for(int i=0; i<NUM_ROCKET; i++)
		{
			m_pRockets[i].Render();
		}
	}
}

bool CSpark::IsDead()
{
	bool bResult = true;

	if(m_pRockets)
	{
		for(int i=0; i<NUM_ROCKET; i++)
		{
			if(m_pRockets)
			{
				bResult = bResult && m_pRockets[i].IsDead();
			}
		}
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
// Class CRocket
//////////////////////////////////////////////////////////////////////////
CRocket::CRocket()
{
	m_pParent = NULL;
	m_pPoints = NULL;
}


CRocket::~CRocket()
{
	SAFE_DELETE_ARRAY(m_pPoints);
}

void CRocket::Init(vec3 org, vec3 direction, CPSEquipment* pEquip)
{
	m_pParent = pEquip;
	m_position = org;
	m_direction = direction;
	
	m_fWeight = ROCKET_WEIGHT;
	m_fSpeedOrg = GetRandomFloat(ROCKET_SPEED/2.f, ROCKET_SPEED*2.f);
	m_fSpeed = m_fSpeedOrg;

	m_fSizeOrg = GetRandomFloat(ROCKET_SIZE/2.f, ROCKET_SIZE*2.f);
	m_fSize = m_fSizeOrg * 0.6f;

	m_fLife = GetRandomFloat(ROCKET_LIFE/2.f, ROCKET_LIFE * 2.f);
	m_fAge = 0.f;

	m_nNumber = FIRE_PARTICLE_NUM;
	
	SAFE_DELETE_ARRAY(m_pPoints);

	m_pPoints = new PointInfo[FIRE_PARTICLE_NUM];

	for(int i=0; i<FIRE_PARTICLE_NUM; i++)
	{
		ResetPoint(i, false);
	}
}

void CRocket::Render()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, m_pParent->s_nTexSpark);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// make sprite
	mat4 matView = g_PSGame.m_Camera.getViewMat();
	// 	mat4 matViewInv;
	// 	invert(matViewInv, matView);
	// 	matViewInv.set_translation(m_posCurrent);
	glLoadMatrixf(matView.mat_array);
	// 	glMultMatrixf(matViewInv.mat_array);
	glMultMatrixf(m_pParent->m_matModel.mat_array);

	if(m_pPoints)
	{
		for(int i=0; i<FIRE_PARTICLE_NUM; i++)
		{
			RenderPoint(i);
		}
	}
	glPopMatrix();
}

void CRocket::Idle(float fElapsedTime)
{
	m_fAge += fElapsedTime;

	// Update position
	m_position += fElapsedTime * m_fSpeed * m_direction;
	
	// Update direction
	vec3 velocity = m_fSpeed * m_direction;
	vec3 velOffset = m_fWeight * fElapsedTime * vec3(0.f, -1.f, 0.f);
	velocity += velOffset;
	velocity.normalize();
	m_direction = velocity;

	float fade = m_fAge/m_fLife;
	
	// Update number
	int nNumber = (int)((1.f- 2.f * fade) * (float)FIRE_PARTICLE_NUM);
	int nEmit = m_nNumber - nNumber;
	EmitFire(nEmit);
	m_nNumber = nNumber;

	// Update size
	if(fade < 0.1f)
		m_fSize = (0.6f + 4.f * fade) * m_fSizeOrg;
	else
		m_fSize = m_fSizeOrg;

	// Update Speed
	if(fade < 0.5f)
	{
		m_fSpeed = (1.f - 1.6f * fade) * m_fSpeedOrg;
	}
	else
	{
		m_fSpeed = (0.2f - 0.2f * (fade-0.5f)) * m_fSpeedOrg;
	}

	for(int i=0; i<FIRE_PARTICLE_NUM; i++)
	{
		UpdatePoint(i, fElapsedTime);
	}
}

void CRocket::EmitFire(int num)
{
	int total = num;
	for(int i=0; i<FIRE_PARTICLE_NUM; i++)
	{
		if(total <= 0)
			break;

		if(m_pPoints[i].texId == 0)
		{
			ResetPoint(i, true);
			total --;
		}
	}
}

void CRocket::ResetPoint(int index, bool bFire)
{
	if(bFire)
	{
		float fAngle = GetRandomFloat(ps_pi/3.f, ps_pi*4.f/3.f);
		mat3 matRot = mat3_id;
		matRot.set_rot(fAngle, vec3(0.f, 0.f, 1.f));
		m_pPoints[index].velocity = m_direction * matRot;
		m_pPoints[index].speed = FIRE_PARTICLE_SPEED;
		m_pPoints[index].size = FIRE_PARTICLE_SIZE;
		m_pPoints[index].color = FIRE_PARTICLE_COLOR;
		m_pPoints[index].alpha = 1.f;
		m_pPoints[index].weight = FIRE_PARTICLE_WEIGHT;
		m_pPoints[index].life = FIRE_PARTICLE_LIFE;
		m_pPoints[index].age = 0.f;

		m_pPoints[index].texId = 1; // this is fire
	}
	else
	{
		m_pPoints[index].position = m_position;
		m_pPoints[index].velocity = m_direction;
		m_pPoints[index].speed = ROCKET_SPEED;
		m_pPoints[index].size = FIRE_PARTICLE_SIZE;
		m_pPoints[index].color = FIRE_PARTICLE_COLOR;
		m_pPoints[index].alpha = 1.f;
		m_pPoints[index].weight = ROCKET_WEIGHT;
		m_pPoints[index].life = ROCKET_LIFE;
		m_pPoints[index].age = 0.f;

		m_pPoints[index].texId = 0; // this is rocket
	}
}

void CRocket::UpdatePoint(int index, float fElapsedTime)
{
	if(m_pPoints[index].texId == 0) // rocket
	{
		vec3 position = m_position;
 		vec3 offset;
 		float fAngle = GetRandomFloat(ps_pi*11.f/12.f, ps_pi*13.f/12.f);
 		mat3 matRot = mat3_id;
 		matRot.set_rot(fAngle, vec3(0.f, 0.f, 1.f));
 		offset = m_direction * matRot;
		float fOffset = GetRandomFloat(0.f, m_fSize);
		offset *= fOffset;
 		position += offset;

		m_pPoints[index].position = position;
		m_pPoints[index].velocity = m_direction;
	}
	else // fire
	{
		if(m_pPoints[index].age < m_pPoints[index].life)
		{
			m_pPoints[index].age += fElapsedTime;

			// Update position
			m_pPoints[index].position += fElapsedTime * m_pPoints[index].speed * m_pPoints[index].velocity;

			// Update direction
			vec3 velocity = m_pPoints[index].speed * m_pPoints[index].velocity;
			vec3 velOffset = m_pPoints[index].weight * fElapsedTime * vec3(0.f, -1.f, 0.f);
			velocity += velOffset;
			velocity.normalize();
			 m_pPoints[index].velocity = velocity;

			float fade = m_pPoints[index].age/m_pPoints[index].life;

			// Update size
			if(fade < 0.2f)
				m_pPoints[index].size = FIRE_PARTICLE_SIZE;
			else
				m_pPoints[index].size = (1.f - 0.9f*(fade-0.2f)) * FIRE_PARTICLE_SIZE;

			// Update Speed

			// Update Color
			if(fade < 0.6f)
			{
				vec3 color1 = FIRE_PARTICLE_COLOR;
				vec3 color2 = FIRE_PARTICLE_FADE_COLOR;
				lerp(m_pPoints[index].color, fade/0.6f, color1, color2);
			}
			else if(fade < 0.85f)
			{
				vec3 color1 = FIRE_PARTICLE_FADE_COLOR;
				vec3 color2 = FIRE_PARTICLE_LAST_COLOR;
				lerp(m_pPoints[index].color, (fade-0.6f)/(0.85f-0.6f), color1, color2);
			}
			else
			{
				m_pPoints[index].color = FIRE_PARTICLE_LAST_COLOR;
			}

			// Update alpha
			if(fade < 0.1f)
			{
				m_pPoints[index].alpha = 1.f;
			}
			else
			{
				m_pPoints[index].alpha = (1.f - fade)/(1.f - 0.1f);
			}
		}
	}
}

void CRocket::RenderPoint(int index)
{
	if(m_pPoints[index].age < m_pPoints[index].life)
	{
		vec3 pos = m_pPoints[index].position;
		float size = m_pPoints[index].size;
		glColor4f(m_pPoints[index].color.x, m_pPoints[index].color.y, m_pPoints[index].color.z, m_pPoints[index].alpha);

		glBegin(GL_QUADS);

		glTexCoord2f(0.f, 0.f);
		glVertex3f(pos.x-size/2.f, pos.y-size/2.f, pos.z);

		glTexCoord2f(0.f, 1.f);
		glVertex3f(pos.x-size/2.f, pos.y+size/2.f, pos.z);

		glTexCoord2f(1.f, 1.f);
		glVertex3f(pos.x+size/2.f, pos.y+size/2.f, pos.z);

		glTexCoord2f(1.f, 0.f);
		glVertex3f(pos.x+size/2.f, pos.y-size/2.f, pos.z);

		glEnd();
	}
}

bool CRocket::IsDead()
{
	bool bResult = true;

	for(int i=0; i<FIRE_PARTICLE_NUM; i++)
	{
		bResult = bResult && (m_pPoints[i].age > m_pPoints[i].life);
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
// Class CPlasma
//////////////////////////////////////////////////////////////////////////

CPlasma::CPlasma()
{
}

CPlasma::~CPlasma()
{
}

void CPlasma::Init(vec3 org, CPSEquipment* pEquip, int nNumber, vec3 color, vec3 fadeColor, float fSize, float fSpeed, float fLife)
{
	CEquipParticle::Init(org, pEquip, nNumber, color, fadeColor, fSize, fSpeed, fLife);
}

void CPlasma::ResetPoint(int index)
{
	CEquipParticle::ResetPoint(index);
	if(m_pPoints)
		m_pPoints[index].texId = rand()%CORONA_TEX_NUM;
}

void CPlasma::UpdatePoint(int index, float fElapsedTime)
{
	CEquipParticle::UpdatePoint(index, fElapsedTime);

	float fade = m_pPoints[index].age / m_pPoints[index].life;
	m_pPoints[index].speed = (1.f - 0.5f * fade) * m_fSpeed;
	m_pPoints[index].size = sqrtf(fade) * m_fSize;
	float fLerp = m_pPoints[index].age / (m_pPoints[index].life * 0.8f);
	if(fLerp >= 1.f)
	{
		m_pPoints[index].color = m_fadeColor;
	}
	else
	{
		lerp(m_pPoints[index].color, fLerp, m_color, m_fadeColor);
	}

	if(fade < 0.1f)
		m_pPoints[index].alpha = 0.f;
	else if(fade < 0.33f)
		m_pPoints[index].alpha = fade;
	else if(fade < 0.66f)
		m_pPoints[index].alpha = 1.f;
	else if(fade < 0.91f)
		m_pPoints[index].alpha = 1.f - fade;
	else
		m_pPoints[index].alpha = 0.f;

	if(m_pPoints[index].age > m_pPoints[index].life)
	{
		ResetPoint(index);
	}
}

void CPlasma::Render()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	CEquipParticle::Render();
}

void CPlasma::RenderPoint(int index)
{
	glBindTexture(GL_TEXTURE_2D, m_pParent->s_nTexCoronas[m_pPoints[index].texId]);
	CEquipParticle::RenderPoint(index);
}

//////////////////////////////////////////////////////////////////////////
// Class CFume
//////////////////////////////////////////////////////////////////////////

CFume::CFume()
{
}

CFume::~CFume()
{
}

void CFume::Init(vec3 org, CPSEquipment* pEquip, int nNumber, vec3 color, vec3 fadeColor, float fSize, float fSpeed, float fLife)
{
	CEquipParticle::Init(org, pEquip, nNumber, color, fadeColor, fSize, fSpeed, fLife);
}

void CFume::UpdatePoint(int index, float fElapsedTime)
{
	CEquipParticle::UpdatePoint(index, fElapsedTime);

	float fade = m_pPoints[index].age / m_pPoints[index].life;
	m_pPoints[index].speed = (1.f - 0.5f * fade) * m_fSpeed;
	if(fade < 0.45f)
		m_pPoints[index].size =  fade * m_fSize;
	else
		m_pPoints[index].size =  m_fSize;

	float fLerp = m_pPoints[index].age / (m_pPoints[index].life * 0.7f);
	if(fLerp >= 1.f)
	{
		m_pPoints[index].color = m_fadeColor;
	}
	else
	{
		lerp(m_pPoints[index].color, fLerp, m_color, m_fadeColor);
	}

	if(fade < 0.2f)
		m_pPoints[index].alpha = fade;
	else if(fade < 0.66f)
		m_pPoints[index].alpha = 1.f;
	else if(fade < 0.85f)
		m_pPoints[index].alpha = 1.f - fade;
	else
		m_pPoints[index].alpha = 0.f;

	if(m_pPoints[index].age > m_pPoints[index].life)
	{
		ResetPoint(index);
	}
}

void CFume::Render()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, m_pParent->s_nTexFume);

	CEquipParticle::Render();
}

//////////////////////////////////////////////////////////////////////////
// Class CCorona
//////////////////////////////////////////////////////////////////////////
CCorona::CCorona()
{
	m_pPlasma = NULL;
	m_pFume = NULL;
}

CCorona::~CCorona()
{
	SAFE_DELETE(m_pPlasma);
	SAFE_DELETE(m_pFume);
}
void CCorona::Init(vec3 org, CPSEquipment* pEquip)
{
	m_pPlasma = new CPlasma;
	m_pPlasma->Init(org, pEquip);

	m_pFume = new CFume;
	m_pFume->Init(org, pEquip);
}

void CCorona::Idle(float fElapsedTime)
{
	if(m_pPlasma)
		m_pPlasma->Idle(fElapsedTime);
	if(m_pFume)
		m_pFume->Idle(fElapsedTime);
}

void CCorona::Render()
{
	if(m_pPlasma)
		m_pPlasma->Render();
	if(m_pFume)
		m_pFume->Render();
}

//////////////////////////////////////////////////////////////////////////
// Class CSmoke
//////////////////////////////////////////////////////////////////////////

CSmoke::CSmoke()
{
	memset(m_smokePoints, 0x00, sizeof(m_smokePoints));
}

CSmoke::~CSmoke()
{

}

void CSmoke::Init(vec3 minPos, vec3 maxPos, float life, float size)
{
	m_minPos = minPos;
	m_maxPos = maxPos;
	m_fLife = life;
	m_fSize = size;

	ResetPoints();
}

void CSmoke::ResetPoints()
{
	for(int i=0; i<NUM_SMOKE_POINT; i++)
	{
		ResetPoint(i);
	}
}

void CSmoke::ResetPoint(int index)
{
	m_smokePoints[index].position.x = GetRandomFloat(m_minPos.x, m_maxPos.x);
	m_smokePoints[index].position.y = m_minPos.y;
	m_smokePoints[index].position.z = GetRandomFloat(m_minPos.z, m_maxPos.z);

	m_smokePoints[index].velocity.x = GetRandomFloat(-0.1f, 0.1f);
	m_smokePoints[index].velocity.y = 1.f;
	m_smokePoints[index].velocity.z = GetRandomFloat(-0.1f, 0.1f);
	float fSpeed = GetRandomFloat(SMOKE_SPEED/2.f, SMOKE_SPEED*2.f);
	m_smokePoints[index].velocity *= fSpeed;

	m_smokePoints[index].size = m_fSize;
	m_smokePoints[index].life = GetRandomFloat(m_fLife, m_fLife*2.f);
	m_smokePoints[index].age = 0.f;
}

void CSmoke::Idle(float fElapsedTime)
{
	vec3 windPos;
	windPos.x = GetRandomFloat(m_minPos.x, m_maxPos.x);
	windPos.y = GetRandomFloat(m_minPos.y, m_maxPos.y);
	windPos.z = GetRandomFloat(m_minPos.z, m_maxPos.z);

	vec3 windDir;
	windDir.x = GetRandomFloat(-1.f, 1.f);
	windDir.y = GetRandomFloat(-1.f, 1.f);
	windDir.z = GetRandomFloat(-1.f, 1.f);
	windDir.normalize();

	float windPower = GetRandomFloat(WIND_POWER/2.f, WIND_POWER*2.f);

	for(int i=0; i<NUM_SMOKE_POINT; i++)
	{
		m_smokePoints[i].age += fElapsedTime;
		m_smokePoints[i].position +=  fElapsedTime * m_smokePoints[i].velocity ;
		m_smokePoints[i].size = SMOKE_SIZE + 2.f * (m_smokePoints[i].position.y - m_minPos.y) / (m_maxPos.y - m_minPos.y) * SMOKE_SIZE;

		vec3 offsetWind = m_smokePoints[i].position - windPos;
		float fDist = offsetWind.norm();
		float fEffect = expf(-fDist) * windPower;
		m_smokePoints[i].velocity += fEffect * windDir;

#if USE_ALPHA
		if(m_smokePoints[i].age > m_smokePoints[i].life)
#else
		if(m_smokePoints[i].age > m_smokePoints[i].life
			|| m_smokePoints[i].position.y > m_maxPos.y 
			|| m_smokePoints[i].position.y < m_minPos.y)
#endif
		{
			ResetPoint(i);
		}
	}
}

void CSmoke::Render()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(g_PSGame.m_Camera.getProjectMat().mat_array);

	for(int i=0; i<NUM_SMOKE_POINT; i++)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		// make Sprite
		mat4 matView = g_PSGame.m_Camera.getViewMat();
		mat4 matViewInv;
		invert(matViewInv, matView);
		matViewInv.set_translation(m_smokePoints[i].position);
		glLoadMatrixf(matView.mat_array);
		glMultMatrixf(matViewInv.mat_array);

#if USE_ALPHA
		float fRemain = 1.f - (m_smokePoints[i].age / m_smokePoints[i].life);
		if(fRemain < 0.2f)
		glColor4f(1.f, 1.f, 1.f, fRemain*5.f);
#endif
		glBegin(GL_QUADS);

		glTexCoord2f(0.f, 0.f);
		//glVertex3f(0.f, -m_points[i].size/2.f, -m_points[i].size/2.f);
		glVertex3f(-m_smokePoints[i].size/2.f, -m_smokePoints[i].size/2.f, 0.f);

		glTexCoord2f(0.f, 1.f);
		//glVertex3f(0.f, m_points[i].size/2.f, -m_points[i].size/2.f);
		glVertex3f(-m_smokePoints[i].size/2.f, m_smokePoints[i].size/2.f, 0.f);

		glTexCoord2f(1.f, 1.f);
		//glVertex3f(0.f, m_points[i].size/2.f, m_points[i].size/2.f);
		glVertex3f(m_smokePoints[i].size/2.f, m_smokePoints[i].size/2.f, 0.f);

		glTexCoord2f(1.f, 0.f);
		//glVertex3f(0.f, -m_points[i].size/2.f, m_points[i].size/2.f);
		glVertex3f(m_smokePoints[i].size/2.f, -m_smokePoints[i].size/2.f, 0.f);

		glEnd();

		glPopMatrix();
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CPSEquipment::LoadTextures()
{
	GLbyte *pBytes;
	GLint nWidth, nHeight, nComponents;
	GLenum eFormat;
	CString appPath = GetAppDirectory() + "\\";
	CString path;

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Load Hand Textures
	glGenTextures(HAND_MAX, s_nTexHands);
	for(int i = HAND_DEFAULT; i < HAND_MAX; i++)
	{
		glBindTexture(GL_TEXTURE_2D, s_nTexHands[i]);
		switch( i )
		{
		case 0 :
			path = appPath + HAND_TEXTURE_PATH0;
			break;
		case 1 :
			path = appPath + HAND_TEXTURE_PATH1;
			break;
		}
		pBytes = LoadImage(path, &nWidth, &nHeight, &nComponents, &eFormat, true);
		if(pBytes)
		{
			gluBuild2DMipmaps(GL_TEXTURE_2D, nComponents, nWidth, nHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			free(pBytes);
		}

	}

	// Load Spark Texture
	glGenTextures(1, &s_nTexSpark);
	glBindTexture(GL_TEXTURE_2D, s_nTexSpark);
	path = appPath + SPARK_TEXTURE_PATH;
	pBytes = LoadImage(path, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBytes)
	{
		gluBuild2DMipmaps(GL_TEXTURE_2D, nComponents, nWidth, nHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		free(pBytes);
	}	

	// Load Corona Textures
	glGenTextures(CORONA_TEX_NUM, s_nTexCoronas);
	for(int i = 0; i < CORONA_TEX_NUM; i++)
	{
		glBindTexture(GL_TEXTURE_2D, s_nTexCoronas[i]);
		path = appPath + CORONA_TEXTURE_PATH;
		path.AppendFormat("%d.png", i);
		pBytes = LoadImage(path, &nWidth, &nHeight, &nComponents, &eFormat);
		if(pBytes)
		{
			gluBuild2DMipmaps(GL_TEXTURE_2D, nComponents, nWidth, nHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			free(pBytes);
		}
	}

	// Load Fume Textures
	glGenTextures(1, &s_nTexFume);
	glBindTexture(GL_TEXTURE_2D, s_nTexFume);
	path = appPath + FUME_TEXTURE_PATH;
	pBytes = LoadImage(path, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBytes)
	{		
		gluBuild2DMipmaps(GL_TEXTURE_2D, nComponents, nWidth, nHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		free(pBytes);
	}	

	// Load Smoke Texture
	glGenTextures(1, &s_nTexSmoke);
	glBindTexture(GL_TEXTURE_2D, s_nTexSmoke);
	path = appPath + SMOKE_TEXTURE_PATH;
	pBytes = LoadImage(path, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBytes)
	{
		gluBuild2DMipmaps(GL_TEXTURE_2D, nComponents, nWidth, nHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		free(pBytes);
	}	

	// Load Shine Texture
	glGenTextures(1, &s_nTexShine);
	glBindTexture(GL_TEXTURE_2D, s_nTexShine);
	path = appPath + SHINE_TEXTURE_PATH;
	pBytes = LoadImage(path, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBytes)
	{
		gluBuild2DMipmaps(GL_TEXTURE_2D, nComponents, nWidth, nHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		free(pBytes);
	}	
}