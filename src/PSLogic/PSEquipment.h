#ifndef _PSEQUIPMENT_H_
#define _PSEQUIPMENT_H_

#include "gl/GL.h"
#include "PSDefine.h"
#include "PS3DMath.h"
#include "PSGeometry.h"
#include <vector>
#include <map>

using namespace std;

#define LAMP_NUM 16
#define BUTTON_NUM 6
#define SWITCH_NUM 3

typedef enum{
	DOOR_LEFT,
	DOOR_RIGHT,
	DOOR_NUM
}DOOR_NUMBER;

typedef enum{
	CTRL_NONE,
	CTRL_BUTTON_1,
	CTRL_BUTTON_2,
	CTRL_BUTTON_3,
	CTRL_BUTTON_4,
	CTRL_BUTTON_5,
	CTRL_BUTTON_6,
	CTRL_BUTTON_OK,
	CTRL_BUTTON_RETRY,
	CTRL_SWITCH_1,
	CTRL_SWITCH_2,
	CTRL_SWITCH_3,
	CTRL_MAX
}CONTROL_ID;

typedef enum
{
	EQUIP_STATE_INIT,
	EQUIP_STATE_OPEN_DOOR,
	EQUIP_STATE_CONTROL,
	EQUIP_STATE_RESULT,
	EQUIP_STATE_WAITING,
	EQUIP_STATE_SPARK,
	EQUIP_STATE_SMOKE,
	EQUIP_STATE_EXPLOSION,
	EQUIP_STATE_DAMMAGED,
	EQUIP_STATE_CLOSE_DOOR,
	EQUIP_STATE_MAX
}EQUIP_STATE;

typedef enum
{
	EQUIP_RESULT_NONE,
	EQUIP_RESULT_SUCCESS,
	EQUIP_RESULT_WARNING,
	EQUIP_RESULT_FAIL,
	EQUIP_RESULT_MAX
}EQUIP_RESULT;

struct PSElement
{
	PSGemNode* m_pNode; // control node;
	char m_pName[NAME_LEN];
	GLfloat m_fValue;
};

struct PSLamp : public PSElement
{
	vec3 m_vColor;
};

struct PSCtrl : public PSElement
{
	mat4 m_matTrans;
};

struct PSSelectCtrl : public PSCtrl
{
	GLuint m_nID;
};

typedef map<CONTROL_ID, GLfloat> StateMap;
typedef vector<StateMap> StateTable;

#define LIGHTING_TIME 3.f
#define TURN_INTERVAL 0.05f
#define BLINKING_INTERVAL 0.2f

#define INIT_LAMP_COLOR vec3(0.f, 1.f, 0.f) // green
#define SUCCESS_LAMP_COLOR vec3(0.f, 1.f, 1.f) // cyan
#define WARING_LAMP_COLOR vec3(1.f, 1.f, 0.f) // yellow
#define FAIL_LAMP_COLOR vec3(1.f, 0.f, 0.f) // red
#define SCORCH_COLOR vec3(0.3f, 0.3f, 0.3f) // dark grey

#define WAITING_TIME 1.f
#define SPARKLE_TIME 8.f
#define SMOKE_TIME 4.f
#define EXPLOSION_TIME 4.f

#define SPARK_NUM 1
#define SMOKE_NUM 5

#define CORONA_NUM 1

#define ELECTRIC_LINE_NUM 6
#define EXPLOSION_NUM 12

typedef enum{
	HAND_DEFAULT,
	HAND_HOLD,
	HAND_MAX
}HAND_TYPE;

#define HAND_SIZE 40.f
#define SCREEN_SIZE 1024.f

#define SHINE_SCALE 2.4f

class CSpark;
class CSmoke;
class CCorona;
class CElectricLine;

#define CONTROL_ELAPSED_TIME 0.5f

typedef enum
{
	EQUIP_TYPE_I,
	EQUIP_TYPE_U,
	EQUIP_TYPE_MAX
}EQUIP_TYPE;

#define CORONA_TEX_NUM 4

class CPSEquipment
{
public:
	CPSEquipment();
	~CPSEquipment();

public:
	void Init(PSGemNode* pRoot);
	void SetType(EQUIP_TYPE type);
	void CalcBoundingBox();

	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	
	void AddAction(short actionType, CPoint point, CONTROL_ID ctrlID, float ctrlValue);
	void SetCtrlValue(short actionType, CONTROL_ID ctrlID, float ctrlValue);

	void ToggleDoor();
	void CloseDoor();
	void OnOK();
	void OnRetry();

	void Reset();
	void Render();

	void Idle(float fElapsedTime);

protected:

	void InitStateMap();
	void InitCtrl();
	void InitCtrlGeo();
	void InitCtrlValue();
	void SetCtrlGeo();
	
	bool IsDoor(PSElement* pElem);
	bool IsLamp(PSElement* pElem);
	bool IsButton(PSElement* pElem);
	bool IsSwitch(PSElement* pElem);
	PSElement* GetControl(PSGemNode* pNode);

	void RenderGeo(P3DGeometry* pGeo, bool bOpaque, P3DMaterial* pMtrl = NULL);
	void RenderNode(PSGemNode* pNode, bool bOpaque);
	void RenderEquip();

	void RenderShine(int index);
	void RenderShines();
	void RenderHand();
	void RenderSmoke();
	void RenderSpark();
	void RenderCorona();
	void RenderExplosion();

	CONTROL_ID GetSelectHits(LONG x, LONG y);

	bool IsSuccess();
	bool IsWarning();
	bool IsFail();

	void AddSpark();
	void DeleteSpark();

	void AddCorona();
	void DeleteCorona();

	void AddSmoke();
	void DeleteSmoke();
	
	void UpdateSpark(float fElapsedTime);
	void UpdateCorona(float fElapsedTime);
	void UpdateSmoke(float fElapsedTime);
	void UpdateExplosion(float fElapsedTime);

public:
	EQUIP_TYPE m_type;

	vec3 m_OpViewDir;
	mat4 m_matModel;

	PSGemNode* m_pRoot;
	vec3 m_bboxMinW;
	vec3 m_bboxMaxW;

	PSGemNode* m_pBody;
	vec3 m_bboxBodyMinW;
	vec3 m_bboxBodyMaxW;
	vec3 m_BodyCenter;

	PSGemNode* m_pCtrlPanel;

	PSGemNode* m_pPanel;
	vec3 m_bboxPanelMinW;
	vec3 m_bboxPanelMaxW;

	vec3 m_PanelCenter;
	float m_PanelSize;

	EQUIP_STATE m_state;
	EQUIP_RESULT m_result;

	GLfloat m_fFrameTime;
	GLfloat m_fTime;
	GLfloat m_fLampTime;
	GLuint m_curLamp; // Used only for EQUIP_STATE_NORMAL

	CONTROL_ID m_curCtrl;

	PSSelectCtrl m_buttons[BUTTON_NUM];
	PSSelectCtrl m_buttonOk;
	PSSelectCtrl m_buttonRetry;
	PSSelectCtrl m_switches[SWITCH_NUM];
	CPoint m_switchPos;
	PSLamp	m_lamps[LAMP_NUM];
	PSCtrl m_doors[DOOR_NUM];

	StateTable m_successTable;
	StateTable m_warningTable;
	StateTable m_failTable;

	vec3 m_posHand;
	GLuint m_curHand;
	vector<CSpark*> m_vecSparks;
	vector<CCorona*> m_vecCoronas;
	vector<CSmoke*> m_vecSmokes;

	static void LoadTextures();

	static GLuint s_nTexHands[HAND_MAX];
	static GLuint s_nTexSpark;
	static GLuint s_nTexCoronas[CORONA_TEX_NUM];
	static GLuint s_nTexFume;
	static GLuint s_nTexSmoke;
	static GLuint s_nTexShine;
};

float GetRandomFloat(float fMin, float fMax);
void GetRandomVector(vec3 &out, const vec3 minPos, const vec3 maxPos);

typedef struct  
{
	vec3 position;
	vec3 velocity;
	float speed;
	vec3 color;
	float alpha;
	float life;
	float age;
	float burntime;
	float size;
	float weight;

	int texId;
}PointInfo;

//////////////////////////////////////////////////////////////////////////
// Class CRocket
//////////////////////////////////////////////////////////////////////////
#define FIRE_PARTICLE_NUM 15
#define FIRE_PARTICLE_SPEED 0.3f
#define FIRE_PARTICLE_SIZE 0.06f
#define FIRE_PARTICLE_LIFE 1.6f
#define FIRE_PARTICLE_COLOR vec3(1.f, 0.6431f, 0.3098f)
#define FIRE_PARTICLE_FADE_COLOR vec3(1.f, 0.4824f, 0.f)
#define FIRE_PARTICLE_LAST_COLOR vec3(0.0667f, 0.0667f, 0.2863f)
#define FIRE_PARTICLE_WEIGHT 0.36f

#define ROCKET_WEIGHT 3.82f
#define ROCKET_SPEED 1.5f
#define ROCKET_LIFE 2.f
#define ROCKET_SIZE 0.12f

class CRocket
{
public:
	CRocket();
	~CRocket();
	void Init(vec3 org, vec3 direction, CPSEquipment* pEquip);
	void Idle(float fElapsedTime);
	void EmitFire(int num);
	virtual void Render();

	virtual void ResetPoint(int index, bool bFire);
	virtual void UpdatePoint(int index, float fElapsedTime);
	virtual void RenderPoint(int index);

	bool IsDead();
public:
	vec3 m_position;
	vec3 m_direction;
	float m_fWeight;
	float m_fSpeedOrg;
	float m_fSpeed;
	float m_fSizeOrg;
	float m_fSize;
	float m_fLife;
	float m_fAge;

	int m_nNumber;
	PointInfo* m_pPoints;
	CPSEquipment* m_pParent;
};

//////////////////////////////////////////////////////////////////////////
// Class CSpark
//////////////////////////////////////////////////////////////////////////
#define NUM_ROCKET 600

class CSpark
{
public:
	CSpark();
	~CSpark();
	void Init(vec3 org, CPSEquipment* pEquip);
	void Idle(float fElapsedTime);
	void Render();
	void ResetRockets();
	bool IsDead();
public:
	vec3 m_position;

	CRocket* m_pRockets;
	CPSEquipment* m_pParent;
};

//////////////////////////////////////////////////////////////////////////
// Class CEquipParticle
//////////////////////////////////////////////////////////////////////////
class CEquipParticle
{
public:
	CEquipParticle();
	virtual ~CEquipParticle();
	virtual void Init(vec3 org, CPSEquipment* pEquip, int nNumber, vec3 color, vec3 fadeColor, float fSize, float fSpeed, float fLife);
	virtual void ResetPoints();
	virtual void Idle(float fElapsedTime);
	virtual void Render();

	virtual void ResetPoint(int index);
	virtual void UpdatePoint(int index, float fElapsedTime);
	virtual void RenderPoint(int index);
public:
	int m_nNumber;
	vec3 m_position;
	vec3 m_color;
	vec3 m_fadeColor;
	float m_fLife;
	float m_fSpeed;
	float m_fSize;

	PointInfo* m_pPoints;
	CPSEquipment* m_pParent;
};


/////////////////////////////////////////////////////////////////////////
// Class CPlasma
//////////////////////////////////////////////////////////////////////////
#define PLASMA_PARTICLE_NUM 90
#define PLASMA_PARTICLE_SPEED 0.9f
#define PLASMA_PARTICLE_SIZE 0.7f
#define PLASMA_PARTICLE_LIFE 0.24f
#define PLASMA_PARTICLE_COLOR vec3(0.5255f, 0.9333f, 1.f)
#define PLASMA_PARTICLE_FADE_COLOR vec3(0.7176f, 0.3961f, 1.f)

class CPlasma : public CEquipParticle
{
public:
	CPlasma();
	~CPlasma();
	virtual void Init(vec3 org, CPSEquipment* pEquip, int nNumber=PLASMA_PARTICLE_NUM, vec3 color=PLASMA_PARTICLE_COLOR, vec3 fadeColor=PLASMA_PARTICLE_FADE_COLOR, float fSize = PLASMA_PARTICLE_SIZE, float fSpeed = PLASMA_PARTICLE_SPEED, float fLife = PLASMA_PARTICLE_LIFE);
	virtual void Render();
	virtual void ResetPoint(int index);
	virtual void UpdatePoint(int index, float fElapsedTime);
	virtual void RenderPoint(int index);
};

//////////////////////////////////////////////////////////////////////////
// Class CFume
//////////////////////////////////////////////////////////////////////////
#define FUME_PARTICLE_NUM 90
#define FUME_PARTICLE_SPEED 0.5f
#define FUME_PARTICLE_SIZE 0.3f
#define FUME_PARTICLE_LIFE 1.f
#define FUME_PARTICLE_COLOR vec3(0.749f, 0.7922f, 1.f)
#define FUME_PARTICLE_FADE_COLOR vec3(1.f, 0.6706f, 0.9765f)

class CFume : public CEquipParticle
{
public:
	CFume();
	~CFume();
	virtual void Init(vec3 org, CPSEquipment* pEquip, int nNumber=FUME_PARTICLE_NUM, vec3 color=FUME_PARTICLE_COLOR, vec3 fadeColor=FUME_PARTICLE_FADE_COLOR, float fSize = FUME_PARTICLE_SIZE, float fSpeed = FUME_PARTICLE_SPEED, float fLife = FUME_PARTICLE_LIFE);
	virtual void Render();
	virtual void UpdatePoint(int index, float fElapsedTime);
};

//////////////////////////////////////////////////////////////////////////
// Class CCorona
//////////////////////////////////////////////////////////////////////////
class CCorona
{
public:
	CCorona();
	~CCorona();
	void Init(vec3 org, CPSEquipment* pEquip);
	void Idle(float fElapsedTime);
	void Render();
public:
	CPlasma* m_pPlasma;
	CFume* m_pFume;
};


//////////////////////////////////////////////////////////////////////////
// Class CSmoke
//////////////////////////////////////////////////////////////////////////
#define NUM_SMOKE_POINT 10
#define SMOKE_SPEED 0.2f
#define WIND_POWER 0.02f
#define SMOKE_SIZE 0.5f

typedef struct  
{
	vec3 position;
	vec3 velocity;
	float size;
	float life;
	float age;
}SmokePointInfo;

class CSmoke
{
public:
	CSmoke();
	~CSmoke();
	void Init(vec3 minPos, vec3 maxPos, float life, float size);
	void Idle(float fElapsedTime);
	void Render();
	void ResetPoints();
	void ResetPoint(int index);
public:
	vec3 m_minPos;
	vec3 m_maxPos;
	float m_fLife;
	float m_fSize;

	SmokePointInfo m_smokePoints[NUM_SMOKE_POINT];
};

#endif //_PSEQUIPMENT_H_
