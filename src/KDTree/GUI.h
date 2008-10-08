#include "KDTree.h"
#include <Windows.h>

void DrawNode( HDC hdc, KdNode* p_node, int p_x, int p_y, KdNode* pCur );
void DrawTree( HDC hdc, KdNode* p_tree, int p_x, int p_y, int p_width, KdNode* pCur );

void PrintInfo( Geometry* pGeo );
void PrintInfo( KdNode* pNode );