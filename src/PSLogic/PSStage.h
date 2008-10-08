#ifndef _PSStage_H_
#define _PSStage_H_

#include "Global.h"
#include <Windows.h>
#include <gl/GLU.h>

class CPSStage
{
public:
	CPSStage();
	
private:
	GLuint		m_nTextures[TEXID_COUNT];	 
	static const char* szTextureFiles[];	
	int			m_nWidth;  // Viewport Width
	int			m_nHeight;  // Viewport Height
	int			m_nDrawState;		// To Show DrawState According To Moving Of Mouse
	int			m_nClickState;		// To Show ButtonDown
	int			m_nSelScene;		// To Show Selected Scene Index	
	int			m_nSelSex;			// To Show Selected Sex Index
	int			m_nSelUniform;		// To Show Selected Uniform Index

public:
	void		Resize(int cx, int cy);
	void		SetDrawState(int nState) { m_nDrawState = nState; };
	void		SetClickState(int nState) { m_nClickState = nState; };
	void		SetScene(int nScene) { m_nSelScene = nScene; };
	void		SetSex(int nSex) { m_nSelSex = nSex; };
	void		SetUniform(int nUniform) { m_nSelUniform = nUniform; };

	int			GetDrawState() { return m_nDrawState; };
	int			GetClickState() { return m_nClickState; };
	int			GetSelScene() { return m_nSelScene; };
	int			GetSelSex()	{ return m_nSelSex; };
	int			GetSelUniform() { return m_nSelUniform; };

	void		LoadTexture();
	void		Render();
	void		RenderMainMenu();					// Main Menu

	void		RenderBackground();

	void		RenderSceneSelect();				// Select Scene
 	void		RenderHumenSelect();				// Select Actor
	void		ShowPreview();						// Show Scenaro
	void		RenderStand();						// Show End Stage
	void		ShowHelp(int state);	

	void		RenderHighLight(int nIndex);
	GLint		GetSelectHits(int nX, int nY, GLuint* selectBuf);
	GLuint*		GetTextures() { return m_nTextures; };

//Event
public:
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
};

#endif