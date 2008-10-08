#ifndef _PSKDTREE_H_
#define _PSKDTREE_H_

#include <assert.h>
#include <vector>
#include <float.h>

using namespace std;

#define LEAF 3
#define XVAL 0
#define YVAL 1
#define ZVAL 2
#define NOVAL 10

struct AxisBoundary
{
	float m_BoundMin[3];
	float m_BoundMax[3];
	float m_Median[3];
};

inline char splitValue( char &prevSplit0, char &prevSplit1, AxisBoundary bbox, float& m )
{
	float a = bbox.m_BoundMax[0] - bbox.m_BoundMin[0];
	float b = bbox.m_BoundMax[1] - bbox.m_BoundMin[1];
	float c = bbox.m_BoundMax[2] - bbox.m_BoundMin[2];

	if ( prevSplit0 == NOVAL )
	{
		if ( a >= b && a >= c )
		{
			m = bbox.m_Median[0];
			return XVAL;
		}

		if ( b >= c )
		{
			m = bbox.m_Median[1];
			return YVAL;
		}

		m = bbox.m_Median[2];
		return ZVAL;
	}
	else if ( prevSplit1 == NOVAL )
	{
		if ( prevSplit0 == XVAL )
		{
			if ( b >= c )
			{
				m = bbox.m_Median[1];
				return YVAL;
			}

			m = bbox.m_Median[2];
			return ZVAL;
		}
		if ( prevSplit0 == YVAL )
		{
			if ( a >= c )
			{
				m = bbox.m_Median[0];
				return XVAL;
			}

			m = bbox.m_Median[2];
			return ZVAL;
		}
		if ( prevSplit0 == ZVAL )
		{
			if ( b >= a )
			{
				m = bbox.m_Median[1];
				return YVAL;
			}

			m = bbox.m_Median[0];
			return XVAL;
		}
	}

	if ( ( prevSplit0 == XVAL && prevSplit1 == YVAL) || ( prevSplit1 == XVAL && prevSplit0 == YVAL) )
	{
		m = bbox.m_Median[2];
		return ZVAL;
	}

	if ( ( prevSplit0 == XVAL && prevSplit1 == ZVAL) || ( prevSplit1 == XVAL && prevSplit0 == ZVAL) )
	{
		m = bbox.m_Median[1];
		return YVAL;
	}

	m = bbox.m_Median[0];
	return XVAL;
};

struct Ray
{
	float m_Org[3];
	float m_Dir[3];
};

struct RaySeg
{
	float m_Org[3];
	float m_Dir[3];
	float m_tStart;
	float m_tEnd;
};

struct Geometry
{
	float *m_pVertices;
	int m_nVeritces;
	float r, g, b;

	vector< short > m_vecIndices;

	void CalcBBox( AxisBoundary &box );
	void CalcBBox( float min[], float max[] );
	void Split( char axis, float middle, Geometry **pLeft, Geometry** pRight );
	bool CastingRay( RaySeg rayseg, float coord[3], float &t );

	void ReadFromFile( const char* path );
	void WriteToFile( const char* path );
	
	Geometry() : m_pVertices(0), m_nVeritces(0), m_vecIndices(0) {
		r = (float)rand( ) / (float)RAND_MAX;
		g = (float)rand( ) / (float)RAND_MAX;
		b = (float)rand( ) / (float)RAND_MAX;
	};
};

class KdTree;
class KdNode
{
	friend KdTree;
public:
	char m_AxixId;
	float m_SplitValue;
	KdNode* m_pChildren;
public:
	KdNode() : m_AxixId( LEAF ), m_SplitValue(0), m_pChildren(0)
	{
	}
	~KdNode()
	{
		if( m_pChildren )
		{
			if ( m_AxixId != LEAF )
			{
				delete[] m_pChildren;
				m_pChildren = 0;
			}
			else
			{
				Geometry* pGeo = (Geometry*)m_pChildren;
				delete pGeo;
				m_pChildren = 0;
			}
		}
	}
	
	void Split( char prevSplit0, char prevSplit1, char prevSplit2, int depth, int depthLimit )
	{
		if ( depth >= depthLimit )
			return;

		Geometry* pGeometry = (Geometry*) m_pChildren;
		if ( prevSplit0 != NOVAL && prevSplit1 != NOVAL && prevSplit2 != NOVAL )
		{
			prevSplit0 = NOVAL;
			prevSplit1 = NOVAL;
			prevSplit2 = NOVAL;
		}

		Geometry *pLeftGeo, *pRightGeo;

		AxisBoundary bbox;
		pGeometry->CalcBBox( bbox );
		m_AxixId = splitValue( prevSplit0, prevSplit1, bbox, m_SplitValue );
		pGeometry->Split( m_AxixId, m_SplitValue, &pLeftGeo, &pRightGeo );

		if ( pLeftGeo && pRightGeo )
		{
			if ( pGeometry && depth != 0 )
				delete pGeometry;

			m_pChildren = new KdNode[2];

			m_pChildren[0].m_pChildren = (KdNode*)pLeftGeo;
			m_pChildren[1].m_pChildren = (KdNode*)pRightGeo;

			m_pChildren[0].Split( m_AxixId, prevSplit0, prevSplit1, depth+1, depthLimit );
			m_pChildren[1].Split( m_AxixId, prevSplit0, prevSplit1, depth+1, depthLimit );
		}
		else
		{
			m_AxixId = LEAF;
			
			if ( pLeftGeo )
				delete pLeftGeo;
			if ( pRightGeo )
				delete pRightGeo;
		}
	};

	KdNode* QueryCastingNode( RaySeg rayseg, float coord[], float & t )
	{
		if ( m_AxixId == LEAF )
		{
			if ( ((Geometry*)m_pChildren)->CastingRay( rayseg, coord, t ) )
				return this;
			else
				return 0;
		}

		KdNode* pNode = 0;
		
		if ( rayseg.m_Dir[ m_AxixId ] > 0 )
		{
			if ( rayseg.m_Org[ m_AxixId ] < m_SplitValue )
			{
				pNode = m_pChildren[0].QueryCastingNode( rayseg, coord, t );
				if ( pNode )
					return pNode;
				pNode = m_pChildren[1].QueryCastingNode( rayseg, coord, t );
				if ( pNode )
					return pNode;
			}
			else
			{
				pNode = m_pChildren[1].QueryCastingNode( rayseg, coord, t );
				if ( pNode )
					return pNode;
// 				pNode = m_pChildren[0].QueryCastingNode( rayseg, coord, t );//
// 				if ( pNode )
// 					return pNode;
			}
		}
		else
		{
			if ( rayseg.m_Org[ m_AxixId ] > m_SplitValue )
			{
				pNode = m_pChildren[1].QueryCastingNode( rayseg, coord, t );
				if ( pNode )
					return pNode;
				pNode = m_pChildren[0].QueryCastingNode( rayseg, coord, t );
				if ( pNode )
					return pNode;
			}
			else
			{
				pNode = m_pChildren[0].QueryCastingNode( rayseg, coord, t );
				if ( pNode )
					return pNode;
// 				pNode = m_pChildren[1].QueryCastingNode( rayseg, coord, t );//
// 				if ( pNode )
// 					return pNode;
			}
		}

		return 0;
	};
};

class KdTree
{
public:
	KdNode *m_Root;
	int m_MaxDepth;
	float m_BBoxMin[3];
	float m_BBoxMax[3];
public:
	KdTree(int depth) : m_Root(0), m_MaxDepth(depth)
	{
		assert(depth >= 0);
	}
	~KdTree()
	{
		if(m_Root)
		{
			delete m_Root;
			m_Root = 0;
		}
	}

	void BuildKdTree( Geometry *pGeometry )
	{
		m_Root = new KdNode;
		m_Root->m_AxixId = LEAF;
		m_Root->m_pChildren = (KdNode*)pGeometry;

		pGeometry->CalcBBox( m_BBoxMin, m_BBoxMax );
		
		m_Root->Split( NOVAL, NOVAL, NOVAL, 0, m_MaxDepth );
	};

	float QueryCastingNode( float rayorg[], float raydir[], float dist )
	{
		RaySeg rayseg;
		memcpy( rayseg.m_Dir, raydir, sizeof(float)*3 );
		memcpy( rayseg.m_Org, rayorg, sizeof(float)*3 );
		rayseg.m_tStart = 0;
		rayseg.m_tEnd = dist;

		float coord[3];
		float t;

		if ( m_Root->QueryCastingNode( rayseg, coord, t ) )
			return t;
		
		return FLT_MAX;
	}
};

struct PSGemNode;
void BuildGeoFromGemNode( PSGemNode* pRoot, Geometry *pGeo, bool bPropagetChild );

#endif //_PSKDTREE_H_
