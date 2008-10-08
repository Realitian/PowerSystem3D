#include "stdafx.h"
#include "PSScene.h"
#include "PSGeometry.h"
#include "Global.h"
#include "PSGame.h"

PSScene::PSScene()
{
}

PSScene::~PSScene()
{
	UnLoadEquip();
}

void PSScene::FindEquip( char* strNodeName, PSGemNode* pGemNode, PSGemNode* pTransNode )
{
	if ( pTransNode == 0 )
		return;	
	
	if ( strstr( pTransNode->Name, strNodeName) )
	{
		CPSEquipment* pEquip = new CPSEquipment();
		pEquip->Init( pGemNode );
		pEquip->m_matModel = pTransNode->WTrans;

		if ( strstr( pTransNode->Name, BUILDING_IS_NAME ) || strstr( pTransNode->Name, BUILDING_IT_NAME ) )
		{
			pEquip->SetType(EQUIP_TYPE_I);
			mat4 trans( mat4_id );
			trans.set_translation( vec3(0, EQUIP_I_HEIGHT_DIFF, 0) );
			pEquip->m_matModel = pEquip->m_matModel * trans;
		}
		else if ( strstr( pTransNode->Name, BUILDING_US_NAME ) || strstr( pTransNode->Name, BUILDING_UT_NAME ) )
		{
			pEquip->SetType(EQUIP_TYPE_U);
		}

		pEquip->CalcBoundingBox();
		m_vecEquipments.push_back(pEquip);
	}

	FindEquip( strNodeName, pGemNode, pTransNode->pFirstChild );
	FindEquip( strNodeName, pGemNode, pTransNode->pSibling );
}

CPSEquipment* PSScene::TestCollisionBB( PSLineSeg &lsg )
{
	for ( size_t i = 0 ; i < m_vecEquipments.size() ; i++ )
	{
		if ( ::TestCollisionBB( m_vecEquipments[i]->m_bboxMinW, m_vecEquipments[i]->m_bboxMaxW, lsg ) )
		{
			return m_vecEquipments[i];
		}
	}

	return 0;
}

float PSScene::TestCollisionBBNearest( PSLineSeg &lsg )
{
	float fDist = FLT_MAX;
	for ( size_t i = 0 ; i < m_vecEquipments.size() ; i++ )
	{
		vec3 IntersetedPos;
		if ( ::TestCollisionBB( m_vecEquipments[i]->m_bboxMinW, m_vecEquipments[i]->m_bboxMaxW, lsg, IntersetedPos ) )
		{
			vec3 diff = lsg.start - IntersetedPos;
			if ( fDist > diff.norm() )
			{
				fDist = diff.norm();
			}
		}
	}

	return fDist;
}

void PSScene::UnLoadEquip()
{
	for ( size_t i = 0 ; i < m_vecEquipNodes.size() ; i++ )
		SAFE_DELETE( m_vecEquipNodes[i] );
	m_vecEquipNodes.clear();
	
	for ( size_t i = 0 ; i < m_vecEquipments.size() ; i++ )
		SAFE_DELETE( m_vecEquipments[i] );
	m_vecEquipments.clear();

	for ( size_t i = 0 ; i < g_EquipNodeImagePool.size() ; i++ )
	{
		g_EquipNodeImagePool[i]->DestroyGLTextures();
		SAFE_DELETE( g_EquipNodeImagePool[i] );
	}
	g_EquipNodeImagePool.clear();
}

void PSScene::LoadEquipment()
{
	UnLoadEquip();

	PSGemNode* pNode = NULL;
	CString path = GetAppDirectory() + "\\";
	ReadFromPSFile(&pNode, path + EQUIP_U_S_PATH, g_EquipNodeImagePool);
	pNode->Trans = mat4_id;
	UpdateWorldMatrix(pNode, NULL);
	UpdateBoundingBox(pNode);

	if(pNode != NULL)
	{
		m_vecEquipNodes.push_back(pNode);
		FindEquip( BUILDING_US_NAME, pNode, g_PSGame.m_Building.m_pGemNodeRoot );
	}

 	pNode = NULL;
 	ReadFromPSFile(&pNode, path + EQUIP_U_T_PATH, g_EquipNodeImagePool);
 	pNode->Trans = mat4_id;
	UpdateWorldMatrix(pNode, NULL);
	UpdateBoundingBox(pNode);

 	if(pNode != NULL)
 	{
 		m_vecEquipNodes.push_back(pNode);
 		FindEquip( BUILDING_UT_NAME, pNode, g_PSGame.m_Building.m_pGemNodeRoot );
 	}
 
 	pNode = NULL;
 	ReadFromPSFile(&pNode, path + EQUIP_I_S_PATH, g_EquipNodeImagePool);
 	pNode->Trans = mat4_id;
	UpdateWorldMatrix(pNode, NULL);
	UpdateBoundingBox(pNode);

 	if(pNode != NULL)
 	{
 		m_vecEquipNodes.push_back(pNode);
 		FindEquip( BUILDING_IS_NAME, pNode, g_PSGame.m_Building.m_pGemNodeRoot );
 	}
 
 	pNode = NULL;
 	ReadFromPSFile(&pNode, path + EQUIP_I_T_PATH, g_EquipNodeImagePool);
 	pNode->Trans = mat4_id;
	UpdateWorldMatrix(pNode, NULL);
	UpdateBoundingBox(pNode);

 	if(pNode != NULL)
 	{
 		m_vecEquipNodes.push_back(pNode);
 		FindEquip( BUILDING_IT_NAME, pNode, g_PSGame.m_Building.m_pGemNodeRoot );
 	}
}

void PSScene::StartOperateWithEquip()
{
	PSLineSeg lsg;
	lsg.start = g_PSGame.m_Humen.m_Pos + METER * vec3_y;
	lsg.dir = - g_PSGame.m_Camera.m_eyePos + lsg.start;
	lsg.dir.y = 0;
	lsg.dir.normalize();
	lsg.dist = HUMEN_OBJECT_DIST;
	m_pCurEquip = 0;
	if ( m_pCurEquip = TestCollisionBB( lsg ) )
	{
		if ( m_pCurEquip && m_pCurEquip->m_state == EQUIP_STATE_CLOSE_DOOR )
			return;

		g_PSGame.m_EquipStage = EQUIP_OP_START;
		m_pCurEquip->ToggleDoor();

		if ( g_PSGame.m_GameStage == NAVIGATING )
		{
			CPSEquipActionRecord action;
			action.opType = EQUIP_ACTION_STARTOP;
			action.fTimeAt = g_PSGame.m_fElapsedFrameTime;
			g_PSGame.m_EquipRecordManager.PushAction( action );
		}
	}
}

void PSScene::EndOperateWithEquip()
{
	if ( m_pCurEquip && m_pCurEquip->m_state >= EQUIP_STATE_SPARK && m_pCurEquip->m_state <= EQUIP_STATE_DAMMAGED )
		return;

	if ( m_pCurEquip && m_pCurEquip->m_state == EQUIP_STATE_OPEN_DOOR )
		return;

	g_PSGame.m_EquipStage = EQUIP_OP_NONE;
	
	if ( m_pCurEquip )
		m_pCurEquip->ToggleDoor();

	if ( g_PSGame.m_GameStage == NAVIGATING )
	{
		CPSEquipActionRecord action;
		action.opType = EQUIP_ACTION_ENDOP;
		action.fTimeAt = g_PSGame.m_fElapsedFrameTime;
		g_PSGame.m_EquipRecordManager.PushAction( action );
	}
}

void PSScene::Render( bool bMap )
{
	if ( !g_PSGame.m_bLoaded )
		return;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	vector<CPSEquipment*>::iterator it;
	for(it = m_vecEquipments.begin(); it != m_vecEquipments.end(); it++)
	{
		CPSEquipment* pEquip = (CPSEquipment*)(*it);
		pEquip->Render();
	}
	glPopAttrib();
}

void PSScene::ResetEquips()
{
	vector<CPSEquipment*>::iterator it;
	for(it = m_vecEquipments.begin(); it != m_vecEquipments.end(); it++)
	{
		CPSEquipment* pEquip = (CPSEquipment*)(*it);
		pEquip->Reset();
	}
}

void PSScene::Idle(float fElapsedTime)
{
	if( m_pCurEquip )
		m_pCurEquip->Idle(fElapsedTime);
}

void PSScene::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if( m_pCurEquip )
		m_pCurEquip->OnKeyDown(nChar, nRepCnt, nFlags);
}

void PSScene::OnLButtonDown(UINT nFlags, CPoint point)
{
	if( m_pCurEquip )
		m_pCurEquip->OnLButtonDown(nFlags, point);
}

void PSScene::OnMouseMove(UINT nFlags, CPoint point)
{
	if( m_pCurEquip )
		m_pCurEquip->OnMouseMove(nFlags, point);
}

void PSScene::OnLButtonUp(UINT nFlags, CPoint point)
{
	if( m_pCurEquip )
		m_pCurEquip->OnLButtonUp(nFlags, point);
}

void PSScene::SetCtrlValue(short actionID, int ctrlID, float ctrlValue)
{
	if(m_pCurEquip)
		m_pCurEquip->SetCtrlValue(actionID, (CONTROL_ID)ctrlID, ctrlValue);
}