#include "GUI.h"
#include <math.h>

#define CIRCLE_RADIUS 6
#define LAYER_HEIGHT CIRCLE_RADIUS*5
int	g_x;
int g_y;

void Line( HDC hdc, int x1, int y1, int x2, int y2 )
{
	MoveToEx( hdc, x1, y1, 0 );
	LineTo( hdc, x2, y2 );
}

void ArrowLine( HDC hdc, int x1, int y1, int x2, int y2,
			   int r1, int r2, bool arrow1, bool arrow2/*, SDL_Color p_color*/ )
{
	int tx1, ty1, tx2, ty2;

	// calculate distance from each point
	float dx = (float)(x2 - x1);
	float dy = (float)(y2 - y1);
	float d = (float)sqrt( dx * dx + dy * dy );

	// calculate the normal vector (dx, dy), and
	// the angle of that vector, a.
	float a;
	dx /= d;
	dy /= d;
	a = (float)acos( dx );

	// if dy is less than 0, then subtract the angle from 2PI
	if( dy < 0 )
		a = 6.2832f - a;

	// calculate new coordinates (tx1,ty1) and (tx2,ty2) that are
	// r1 and r2 units away from the original points, along the
	// same line.
	tx1 = x1 + (int)((float)cos(a) * (float)r1);
	ty1 = y1 + (int)((float)sin(a) * (float)r1);
	tx2 = x2 + (int)((float)cos(a+3.1416f) * (float)r2);
	ty2 = y2 + (int)((float)sin(a+3.1416f) * (float)r2);

	// draw the line with the new coordinates
	Line( hdc, tx1, ty1, tx2, ty2 );

	if( arrow1 == true )
	{
		// calculate two lines that are 0.3 degrees away
		// from the real line, 15 pixels long, and draw them.
		// these two lines are the arrowhead.
		tx2 = tx1 + (int)((float)cos(a+0.3f) * 15.0f);
		ty2 = ty1 + (int)((float)sin(a+0.3f) * 15.0f);
		Line( hdc, tx1, ty1, tx2, ty2 );
		tx2 = tx1 + (int)((float)cos(a-0.3f) * 15.0f);
		ty2 = ty1 + (int)((float)sin(a-0.3f) * 15.0f);
		Line( hdc, tx1, ty1, tx2, ty2 );
	}
	if( arrow2 == true )
	{
		// calculate two lines that are 0.3 degrees away
		// from the real line, 15 pixels long, and draw them.
		// these two lines are the arrowhead.
		a += 3.1416f;
		tx1 = x2 + (int)((float)cos(a) * (float)r2);
		ty1 = y2 + (int)((float)sin(a) * (float)r2);
		tx2 = tx1 + (int)((float)cos(a+0.3f) * 15.0f);
		ty2 = ty1 + (int)((float)sin(a+0.3f) * 15.0f);
		Line( hdc, tx1, ty1, tx2, ty2 );
		tx2 = tx1 + (int)((float)cos(a-0.3f) * 15.0f);
		ty2 = ty1 + (int)((float)sin(a-0.3f) * 15.0f);
		Line( hdc, tx1, ty1, tx2, ty2 );
	}
}

void DrawNode( HDC hdc, KdNode* p_node, int p_x, int p_y, KdNode* pCur )
{
	char text[50];
	memset( text, 0, 50 );

	switch ( p_node->m_AxixId )
	{
	case XVAL:
		sprintf ( text, "X" );
		//sprintf ( text, "X = %.3f", p_node->m_SplitValue );
		break;
	case YVAL:
		sprintf ( text, "Y" );
		//sprintf ( text, "Y = %.3f", p_node->m_SplitValue );
		break;
	case ZVAL:
		sprintf ( text, "Z" );
		//sprintf ( text, "Z = %.3f", p_node->m_SplitValue );
		break;
	case LEAF:
		int indices = ( (Geometry*) p_node->m_pChildren )->m_vecIndices.size();
		sprintf ( text, "%d", indices );
		break;
	}

	int left, top, right, bottom;

	left = p_x;
	right = p_x + 2*CIRCLE_RADIUS;
	top = p_y;
	bottom = p_y + 2*CIRCLE_RADIUS;

	HGDIOBJ original = NULL;

	//Save original object.
	original = SelectObject(hdc,GetStockObject(DC_PEN));

	//Change the DC pen color
	if( p_node->m_AxixId == LEAF )
		SetDCPenColor(hdc,RGB(0x00,0xff,0x00));
	else
		SetDCPenColor(hdc,RGB(0x00,0x00,0x00));
	if( pCur == p_node )
	{
		SetDCPenColor(hdc,RGB(0xff,0x00,0x00));
		g_x = p_x;
		g_y = p_y;
	}

	RECT rc;
	rc.left = left;
	rc.right = right;
	rc.top = top;
	rc.bottom = bottom;
	Ellipse( hdc, left, top, right, bottom );
	TextOutA( hdc, left, bottom, text, strlen(text) );
	SelectObject(hdc,original);
}

void DrawTree( HDC hdc, KdNode* p_tree, int p_x, int p_y, int p_width, KdNode* pCur )
{
	if( p_tree != 0 )
	{
		DrawNode( hdc, p_tree, p_x + (p_width / 2) - CIRCLE_RADIUS, p_y, pCur );

		int w = p_width / 2;
		int w2 = w / 2;

		if( p_tree->m_AxixId != LEAF )
		{
			DrawTree( hdc, p_tree->m_pChildren, p_x, p_y + LAYER_HEIGHT, w, pCur );
			ArrowLine( hdc, p_x + (p_width / 2), p_y + CIRCLE_RADIUS, 
				p_x + w2, p_y + LAYER_HEIGHT+CIRCLE_RADIUS, CIRCLE_RADIUS, CIRCLE_RADIUS, false, true );

			DrawTree( hdc, p_tree->m_pChildren+1, p_x + w, p_y + LAYER_HEIGHT, w, pCur );
			ArrowLine( hdc, p_x + (p_width / 2), p_y + CIRCLE_RADIUS, 
				p_x + w2 + w, p_y + LAYER_HEIGHT+CIRCLE_RADIUS, CIRCLE_RADIUS, CIRCLE_RADIUS, false, true );
		}
	}
}

void PrintInfo( Geometry* pGeo )
{
	FILE *pLog = fopen( "log.txt", "at" );
	fprintf( pLog, "indices : %d\n", pGeo->m_vecIndices.size() );
	fclose( pLog );
}

void PrintInfo( KdNode* pNode )
{
	if ( pNode->m_AxixId == LEAF )
		PrintInfo( (Geometry*)pNode->m_pChildren );
	else
	{
		FILE *pLog = fopen( "log.txt", "at" );
		fprintf( pLog, "AxisId : %i, SplitValue : %.3f\n", pNode->m_AxixId, pNode->m_SplitValue );
		fclose( pLog );

		PrintInfo( pNode->m_pChildren );
		PrintInfo( pNode->m_pChildren+1 );
	}
}