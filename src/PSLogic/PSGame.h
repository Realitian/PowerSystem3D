#ifndef _PSGAME_H_
#define _PSGAME_H_

#include <vector>
#include "PSHumen.h"
#include "PSScene.h"
#include "PSStage.h"
#include "PSCamera.h"
#include "AVIVideo.h"
#include <Vfw.h>

using namespace std;

typedef void *LOADINGCALLBACK( int );
void LoadHumenData();
void LoadSceneData();
void LoadScenario();
void SaveScenario();

enum EQUIPACTION
{
	EQUIP_ACTION_NONE = 0,
	EQUIP_ACTION_STARTOP,
	EQUIP_ACTION_ENDOP
};

struct CPSHumenActionRecord
{
	float fTimeAt;
	int iMotionId;
	float fAngleX;
	float fAngleY;
	float fPosX;
	float fPosZ;
};

struct CPSEquipActionRecord
{
	float fTimeAt;
	short opType;
	float fMouseX;
	float fMouseY;
	int iCtrlID;
	float fCtrlValue;
};

class CPSHumenRecordManager
{
public:
	vector < CPSHumenActionRecord > m_vecHumenActions;
	vec3 m_InitHumenPos;

	void Init();
	void PushAction( CPSHumenActionRecord action );
	bool GetAction( CPSHumenActionRecord &action, float fCurrentTime );
	bool GetHumenPos( vec3 &pos, float fCurrentTime );
	void SaveToFile( FILE* pFile );
	void ReadFromFile( FILE* pFile );
};

class CPSEquipRecordManager
{
public:
	vector < CPSEquipActionRecord > m_vecEquipActions;
	size_t m_iCurActionId;
	void Init();
	void PushAction( CPSEquipActionRecord action );
	int GetAction( CPSEquipActionRecord &action1, CPSEquipActionRecord &action2, float fCurrentTime );
	void SaveToFile( FILE* pFile );
	void ReadFromFile( FILE* pFile );
};

class PSGame
{
public:
	//game logic
	bool m_bToPlay;
	bool m_bEnded;
	bool m_bExportAvi;
	PSStage m_GameStage;
	PSEquipOpStage m_EquipStage;
	CPSStage m_stageMgr;

	CPSCamera m_Camera;
	PSHumen m_Humen;
	CPSBuilding m_Building;
	PSScene m_Scene;	
	int	m_nHelpState; // enum PSHelp
	// Export
	int			m_nWidth;  // Viewport Width
	int			m_nHeight;  // Viewport Height
	CString	m_szAviPath;
	int m_nAviFps;
	AVICOMPRESSOPTIONS m_aviOpts;
	CAVIVideo *m_pVideoMgr;
	//
	CPSHumenRecordManager m_HumenRecordManager;
	CPSEquipRecordManager m_EquipRecordManager;
	//attribute.
	void GetName( char* strName );
	void GetHintTxt( char* strName );
	float m_fFade;

	bool m_bLoaded;
public:
	PSGame();
	~PSGame();

	//stage management members.
	void NewScenario();
	void SelectScene();
	void SelectHumen();
	void ShowPreview();
	void OpenScenario( const char* path );
	void SaveScenario( const char* path );
	void StartScenario( );
	void UnLoadAll( );
	void LoadScenario( LOADINGCALLBACK pLoadingProc );
	void ScenarioLoaded();
	void ReadyToPlay();
	void MainMenu();
	void Quit();
	void ExportToAvi();
	bool IsEmptyScenario();
	static void MakeEnvDir();

	void PlayScenario();

	void RecordState();
	void InitState();

	void Idle( float fElapsedTime );
	float m_fElapsedFrameTime;
	float m_fExportFrameTime;
	float m_fExportedTime;
	GLbyte *m_pFrameBuffer;
	bool m_bCalcExportFrameTime;

	//glProcs.
	void GLSetupRC( );
	void GLRenderProc();

	void RenderNormal();
	void GLSetupLight();
	void RenderMap( );
	void RenderFade();
	void GLResize(int cx, int cy);

	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	void CancelPlayScenario();
	void EnterIndoor();
	void GoExiting();
	void OnInitialize();

	void CalcCursorPoint(CPSEquipActionRecord* pFirst, CPSEquipActionRecord* pSecond, float fCurrentTime, CPoint* pPoint);
};

extern PSGame g_PSGame;

#endif //_PSGAME_H_