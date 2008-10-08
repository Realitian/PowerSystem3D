#include "stdafx.h"
#include "PSCamera.h"
#include "PSGame.h"

#define PROFILE

CPSCamera::CPSCamera()
{
	m_fAngleX = 0;
}

void CPSCamera::SetStand()
{
	m_eyePos = g_PSGame.m_Building.m_StartPos + HUMEN_EYE_DIST_MAX * vec3_z + METER * vec3_y;
	m_eyeDir = vec3_neg_z;
	g_PSGame.m_GameStage = STAND;
}

void CPSCamera::Idle( float fElapsedTime )
{
	if ( g_PSGame.m_EquipStage != EQUIP_OP_NONE )
	{
		static float fSparkTime = 0;
		static vec3 SparkTargetPos;
		
		static float fExplosionTime = 0;
		static vec3 ExplosionTargetPos;
		static vec3 ExplosionTargetDir;

		switch( g_PSGame.m_Scene.m_pCurEquip->m_state )
		{
		case EQUIP_STATE_SPARK:
			{
				if ( fSparkTime == 0 )
					SparkTargetPos = m_eyePos - 0.5f * METER * g_PSGame.m_Scene.m_pCurEquip->m_OpViewDir;

				m_eyePos = (1.0f - fSparkTime) * m_eyePos + fSparkTime * SparkTargetPos;
 				fSparkTime += fElapsedTime / SPARKLE_TIME;
			}
			break;
		case EQUIP_STATE_SMOKE:
			{
 				if ( fSparkTime > 0 )
 				{
 					m_eyePos = SparkTargetPos;
 					fSparkTime = 0;
 					vec3 humenPos = SparkTargetPos - 0.5f * METER * g_PSGame.m_Scene.m_pCurEquip->m_OpViewDir; 
					g_PSGame.m_Humen.m_Pos.x = humenPos.x;
					g_PSGame.m_Humen.m_Pos.z = humenPos.z;
 				}

				float turbx = 0.5f - ( (float)rand() / (float)RAND_MAX );
				float turby = 0.5f - ( (float)rand() / (float)RAND_MAX );
				vec3 OpViewRight;
				cross( OpViewRight, g_PSGame.m_Scene.m_pCurEquip->m_OpViewDir, vec3_y );
				OpViewRight.normalize();
				m_eyePos = m_eyePos + 0.1f * METER * ( turbx * OpViewRight + turby * vec3_y );

				fExplosionTime = 0;
			}
			break;
		case EQUIP_STATE_EXPLOSION:
			{
				if( fExplosionTime == 0)
				{
					ExplosionTargetPos = g_PSGame.m_Humen.m_Pos - 2.0f * METER * g_PSGame.m_Scene.m_pCurEquip->m_OpViewDir + 3.0f * METER * vec3_y;
					ExplosionTargetDir = 0.5f * (g_PSGame.m_Humen.m_Pos + g_PSGame.m_Scene.m_pCurEquip->m_PanelCenter ) - ExplosionTargetPos;
					ExplosionTargetDir.normalize();
				}
				
				m_eyePos = (1.0f - fExplosionTime) * m_eyePos + fExplosionTime * ExplosionTargetPos;
				m_eyeDir = (1.0f - fExplosionTime) * m_eyeDir + fExplosionTime * ExplosionTargetDir;
				fExplosionTime += fElapsedTime / EXPLOSION_TIME;
			}
			break;
		case EQUIP_STATE_DAMMAGED:
			break;
		default:
			{
				fSparkTime = 0;

				static float fTime = 0;

				vec3 TargetPos = g_PSGame.m_Scene.m_pCurEquip->m_PanelCenter - g_PSGame.m_Scene.m_pCurEquip->m_PanelSize * g_PSGame.m_Scene.m_pCurEquip->m_OpViewDir;
				m_eyePos = (1.0f - fTime) * m_eyePos + fTime * TargetPos;
				m_eyeDir = (1.0f - fTime) * m_eyeDir + fTime * g_PSGame.m_Scene.m_pCurEquip->m_OpViewDir;
				fTime += fElapsedTime;

				if ( fTime > 1 )
				{
					m_eyePos = TargetPos;
					m_eyeDir = g_PSGame.m_Scene.m_pCurEquip->m_OpViewDir;

					fTime = 0;
					g_PSGame.m_EquipStage = EQUIP_OP_OPERATING;
				}
			}
			break;
		}

		return;
	}

	switch ( g_PSGame.m_GameStage )
	{
	case SHOW_PREVIEW:
		m_eyePos.x = g_PSGame.m_Building.m_Center.x + g_PSGame.m_Building.m_Radius/4;
		m_eyePos.y = g_PSGame.m_Building.m_Center.y + g_PSGame.m_Building.m_Radius/4;
		m_eyePos.z = g_PSGame.m_Building.m_Center.z + g_PSGame.m_Building.m_Radius/2;

		m_eyeDir = g_PSGame.m_Building.m_Center - m_eyePos;
		m_eyeDir.normalize();
		break;
	case OVERVIEW_SCENE:
		{
			static float sForwardingRate = 1;
			static float fEyeAngleY = 0;

			if ( sForwardingRate == 1 && fEyeAngleY < ps_two_pi )
			{
				m_eyePos.x = g_PSGame.m_Building.m_Center.x + g_PSGame.m_Building.m_Radius * sinf( fEyeAngleY );
				m_eyePos.z = g_PSGame.m_Building.m_Center.z + g_PSGame.m_Building.m_Radius * cosf( fEyeAngleY );
				m_eyePos.y = g_PSGame.m_Building.m_StartPos.y + HUMEN_EYE_DIST_MAX + g_PSGame.m_Building.m_Radius * 0.1f * ( 1 + sinf( fEyeAngleY ) );
				m_eyeDir = g_PSGame.m_Building.m_StartPos - m_eyePos;
			}
			else
			{
				fEyeAngleY = 0;
				vec3 target = g_PSGame.m_Building.m_StartPos + HUMEN_EYE_DIST_MAX * vec3_z + METER * vec3_y;

				sForwardingRate -= fElapsedTime * 0.1f;

				if ( sForwardingRate <= 0.1f )
				{
					m_eyeDir = vec3_neg_z;
					g_PSGame.m_GameStage = STAND;

					sForwardingRate = 1;
					fEyeAngleY = 0;
					return;
				}

				if ( sForwardingRate < 0.7f )
					sForwardingRate -= fElapsedTime;

				float t = 1 - sForwardingRate;

				m_eyePos = sForwardingRate * m_eyePos + t * target;
				vec3 dir = g_PSGame.m_Building.m_StartPos - m_eyePos;
				dir.normalize();
				m_eyeDir = sForwardingRate * dir + t * vec3_neg_z;
			}

			fEyeAngleY += fElapsedTime;
		}
		break;
	case STAND:
		if ( g_PSGame.m_Building.m_DoorStatus == CPSBuilding::OPENED )
		{
			static float fDist = m_eyePos.z - g_PSGame.m_Humen.m_Pos.z;
			static float fDistLimit = 0;

			m_eyePos.z = g_PSGame.m_Humen.m_Pos.z + fDist + fDistLimit;
			fDistLimit -= fElapsedTime * fDist / 2.0f / 5.0f;
			if ( fDistLimit < -fDist/2 )
				fDistLimit = -fDist/2;
		}
		else
		{
			m_eyePos = g_PSGame.m_Building.m_StartPos + HUMEN_EYE_DIST_MAX * vec3_z + METER * vec3_y;
			m_eyeDir = vec3_neg_z;
		}
		break;
	case NAVIGATING:
	case PLAYING:
		{
			static float fDist = m_eyePos.z - g_PSGame.m_Humen.m_Pos.z;
			m_eyeDir = -vec3( sinf( g_PSGame.m_Humen.m_fAngleY ), sinf(m_fAngleX), cosf( g_PSGame.m_Humen.m_fAngleY ) );
			m_eyeDir.normalize();

			//1 ~ 0.1 => 1 ~ 0.3
			// x => y
			// y = powf(x, 0.3f) 

			m_eyePos = g_PSGame.m_Humen.m_Pos - powf(cosf(m_fAngleX), 0.3f)*fDist * m_eyeDir + METER * vec3_y;

			PSLineSeg lsg;
			lsg.start = g_PSGame.m_Humen.m_Pos + METER * vec3_y;
			lsg.dir = m_eyePos - lsg.start;

			float fEyeHumenDist = lsg.dist = lsg.dir.norm();
			lsg.dir.normalize();

#ifdef PROFILE
			LARGE_INTEGER Frequency;
			LARGE_INTEGER l1, l2, l3;

			QueryPerformanceFrequency(&Frequency);

			QueryPerformanceCounter(&l1);
			float fRDist1 = TestCollision( g_PSGame.m_Building.m_pGemNodeRoot, lsg );
			QueryPerformanceCounter(&l2);
			float fRDist = g_PSGame.m_Building.m_KdTree->QueryCastingNode( lsg.start.vec_array, lsg.dir.vec_array, FLT_MAX );
			QueryPerformanceCounter(&l3);

			float t0 = 1000.0f * ((float)(l2.QuadPart - l1.QuadPart)) / ((float)(Frequency.QuadPart));
			float t1 = 1000.0f * ((float)(l3.QuadPart - l2.QuadPart)) / ((float)(Frequency.QuadPart));

			static float mint0 = t0;
			static float maxt0 = t0;
			static float mint1 = t1;
			static float maxt1 = t1;

			static float meant0 = t0;
			static float meant1 = t1;

			if ( mint0 > t0 )
				mint0 = t0;
			if ( maxt0 < t0 )
				maxt0 = t0;

			if ( mint1 > t1 )
				mint1 = t1;
			if ( maxt1 < t1 )
				maxt1 = t1;

			meant0 += t0;
			meant0 *= 0.5f;
			
			meant1 += t1;
			meant1 *= 0.5f;

			char filename[50];
			sprintf_s( filename, "profile%d.log", g_PSGame.m_Building.m_KdTree->m_MaxDepth );
			FILE* pLog = fopen( filename, "wt" );
			fprintf( pLog, "bbox mode: minTime-%.3fms, maxTime-%.3fms, meanTime-%.3fms\n", mint0, maxt0, meant0 );
			fprintf( pLog, "kdTree mode: minTime-%.3fms, maxTime-%.3fms, meanTime-%.3fms\n", mint1, maxt1, meant1 );
			fclose ( pLog );
#else
		float fRDist = g_PSGame.m_Building.m_KdTree->QueryCastingNode( lsg.start.vec_array, lsg.dir.vec_array, FLT_MAX );
#endif //PROFILE

			float fRDistEquip = g_PSGame.m_Scene.TestCollisionBBNearest( lsg );
			if ( fRDist < fRDistEquip )
			{
				if ( fRDist < fEyeHumenDist )
				m_eyePos = lsg.start + (fRDist-HUMEN_EYE_DIST_MIN) * lsg.dir;
			}
			else
			{
				if ( fRDistEquip < fEyeHumenDist )
					m_eyePos = lsg.start + (fRDistEquip-HUMEN_OBJECT_DIST) * lsg.dir;
			}
		}
		break;
	}
}

mat4 CPSCamera::getMapViewMat()
{
	mat4 mapView( mat4_id );
	look_at( mapView, g_PSGame.m_Building.m_CenterInside+g_PSGame.m_Building.m_RadiusInside*vec3_y, g_PSGame.m_Building.m_CenterInside+METER*vec3_neg_y, vec3_neg_z );
	return mapView;
}

mat4 CPSCamera::getViewMat()
{
	mat4 buildingView( mat4_id );
	look_at( buildingView, m_eyePos, m_eyePos+METER*m_eyeDir, vec3_y );
	return buildingView;
}

void CPSCamera::RotateX( int y )
{
	if ( y < 0 )
	{
		POINT pt;
		GetCursorPos( &pt );
		SetCursorPos( pt.x, 0 );
		return;
	}

	if ( y > m_SCY*2 )
	{
		POINT pt;
		GetCursorPos( &pt );
		SetCursorPos( pt.x, m_SCY*2 );
		return;
	}

	m_fAngleX = CAMERA_ANGLEX_LIMIT*( y - m_SCY ) / (float)m_SCY;
}

mat4 CPSCamera::getMapProjectMat( )
{
	mat4 ProjMat( mat4_id );
	perspective( ProjMat, CAMERA_MAP_FOVY, (float)m_SCX/(float)m_SCY, CAMERA_NEAR_Z_OVERVIEW, g_PSGame.m_Building.m_Radius*40 );
	return ProjMat;
}

mat4 CPSCamera::getProjectMat( )
{
	mat4 ProjMat( mat4_id );
	if ( g_PSGame.m_GameStage == OVERVIEW_SCENE || g_PSGame.m_GameStage == SHOW_PREVIEW )
		perspective( ProjMat, CAMERA_FOVY, (float)m_SCX/(float)m_SCY, CAMERA_NEAR_Z_OVERVIEW, g_PSGame.m_Building.m_Radius*2 );
	else
		perspective( ProjMat, CAMERA_FOVY, (float)m_SCX/(float)m_SCY, CAMERA_NEAR_Z_WATCH, g_PSGame.m_Building.m_Radius*2 );
	return ProjMat;
}