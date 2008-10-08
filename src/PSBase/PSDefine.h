#ifndef _PSDEFINE_H_
#define _PSDEFINE_H_

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#define SCALE_MODEL	1
//#define SCALE_MODEL	0.001f
#define PATH_LEN 64
#define NAME_LEN 32

#define MOTION_COUNT 4

#define MOTION_WALK_PATH		"psm\\Walk.psm"
#define MOTION_RUN_PATH			"psm\\Run.psm"
#define MOTION_STAND_PATH		"psm\\Stand.psm"
#define MOTION_KNOCKOUT_PATH	"psm\\Knockout.psm"

#define MOTION_WALK_ID		0
#define MOTION_RUN_ID		1
#define MOTION_STAND_ID		2
#define MOTION_KNOCKOUT_ID	3
#define WALK_STEP			1.2f*METER
#define RUN_STEP			4.0f*METER
#define HUMEN_INIT_Y_DIFF	- SCALE_MODEL * 400.0f
#define HUMEN_INIT_Z_DIFF	- SCALE_MODEL * 4000.0f

#define SKIN_COUNT 2

#define MALE_PSH_FILEPATH		"psh\\Male.psh"
#define FEMALE_PSH_FILEPATH		"psh\\Female.psh"

#define EQUIP_I_S_PATH			"pse\\I-s.pse"
#define EQUIP_U_S_PATH			"pse\\U-s.pse"
#define EQUIP_I_T_PATH			"pse\\I-t.pse"
#define EQUIP_U_T_PATH			"pse\\U-t.pse"
#define HAND_TEXTURE_PATH0		"Resource\\Image\\HandDefault.png"
#define HAND_TEXTURE_PATH1		"Resource\\Image\\HandHold.png"
#define SPARK_TEXTURE_PATH		"Resource\\Image\\spark.png"
#define CORONA_TEXTURE_PATH		"Resource\\Image\\corona"
#define FUME_TEXTURE_PATH		"Resource\\Image\\fume.png"
#define ELEC_LINE_TEXTURE_PATH	"Resource\\Image\\elecline.png"
#define SMOKE_TEXTURE_PATH		"Resource\\Image\\smoke.png"
#define SHINE_TEXTURE_PATH		"Resource\\Image\\shine.png"
#define EQUIP_I_HEIGHT_DIFF			-305.0f * SCALE_MODEL
#define EQUIP_INSIDEGROUP_NAME		"Group03"
#define EQUIP_PANEL_NAME			"panel"
#define EQUIP_BODY_NAME				"body"
#define BUILDING1_PATH				"psb\\01.psb"
#define BUILDING2_PATH				"psb\\02.psb"
#define BUILDING3_PATH				"psb\\03.psb"
#define BUILDING_LEFT_DOOR_NAME		"l_door"
#define BUILDING_RIGHT_DOOR_NAME	"r_door"
#define BUILDING_STANDPOINT_NAME	"StandPoint"
#define BUILDING_UT_NAME			"U-t"
#define BUILDING_US_NAME			"U-s"
#define BUILDING_IT_NAME			"I-t"
#define BUILDING_IS_NAME			"I-s"
#define BUILDING_ROOF_NAME			"roof"
#define BUILDING_OUTSIDE_NAME		"outside"
#define METER						SCALE_MODEL * 1000.0f
#define BUILDING_DOOR_OPENNING_TIME	3.0f
#define INIT_RADIUS					30 * METER
#define HUMEN_EYE_DIST_MAX			7.0f * METER
#define HUMEN_EYE_DIST_MIN			0.1f * METER
#define HUMEN_OBJECT_DIST			0.6f * METER
#define CAMERA_FOVY					45.0f
#define CAMERA_MAP_FOVY				45.0f
#define CAMERA_NEAR_Z_OVERVIEW		2.0f
#define CAMERA_NEAR_Z_WATCH			0.001f
#define CAMERA_ANGLEX_LIMIT			(ps_half_pi-0.01f)
#define OPEN_DOOR_DIST				2.0f * METER
#define KEY_WALK		'W'
#define KEY_RUN			'R'

#endif //_PSDEFINE_H_