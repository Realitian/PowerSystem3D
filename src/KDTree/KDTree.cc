#include "KDTree.h"
#include <float.h>

void Geometry::CalcBBox( AxisBoundary &box )
{
	box.m_BoundMin[0] = FLT_MAX;
	box.m_BoundMin[1] = FLT_MAX;
	box.m_BoundMin[2] = FLT_MAX;

	box.m_BoundMax[0] = -FLT_MAX;
	box.m_BoundMax[1] = -FLT_MAX;
	box.m_BoundMax[2] = -FLT_MAX;

	box.m_Median[0] = box.m_Median[1] = box.m_Median[2] = 0;

	for ( size_t t = 0 ; t < m_vecIndices.size()/3 ; t++ )
	{
		float cx = ( m_pVertices[ 3*m_vecIndices[3*t+0]+0 ] + m_pVertices[ 3*m_vecIndices[3*t+1]+0 ] + m_pVertices[ 3*m_vecIndices[3*t+2]+0 ] ) / 3.0f;
		float cy = ( m_pVertices[ 3*m_vecIndices[3*t+0]+1 ] + m_pVertices[ 3*m_vecIndices[3*t+1]+1 ] + m_pVertices[ 3*m_vecIndices[3*t+2]+1 ] ) / 3.0f;
		float cz = ( m_pVertices[ 3*m_vecIndices[3*t+0]+2 ] + m_pVertices[ 3*m_vecIndices[3*t+1]+2 ] + m_pVertices[ 3*m_vecIndices[3*t+2]+2 ] ) / 3.0f;

		if ( cx > box.m_BoundMax[0] )
			box.m_BoundMax[0] = cx;
		if ( cy > box.m_BoundMax[1] )
			box.m_BoundMax[1] = cy;
		if ( cz > box.m_BoundMax[2] )
			box.m_BoundMax[2] = cz;

		if ( cx < box.m_BoundMin[0] )
			box.m_BoundMin[0] = cx;
		if ( cy < box.m_BoundMin[1] )
			box.m_BoundMin[1] = cy;
		if ( cz < box.m_BoundMin[2] )
			box.m_BoundMin[2] = cz;

		box.m_Median[0] += cx;
		box.m_Median[1] += cy;
		box.m_Median[2] += cz;
	}

	box.m_Median[0] /= (float)m_vecIndices.size()/3;
	box.m_Median[1] /= (float)m_vecIndices.size()/3;
	box.m_Median[2] /= (float)m_vecIndices.size()/3;
}

void Geometry::CalcBBox( float min[], float max[] )
{
	min[0] = FLT_MAX;
	min[1] = FLT_MAX;
	min[2] = FLT_MAX;

	max[0] = -FLT_MAX;
	max[1] = -FLT_MAX;
	max[2] = -FLT_MAX;

	for ( size_t i = 0 ; i < m_vecIndices.size() ; i++ )
	{
		if ( m_pVertices[ 3*m_vecIndices[i]+0 ] > max[0] )
			max[0] = m_pVertices[ 3*m_vecIndices[i]+0 ];
		if ( m_pVertices[ 3*m_vecIndices[i]+1 ] > max[1] )
			max[1] = m_pVertices[ 3*m_vecIndices[i]+1 ];
		if ( m_pVertices[ 3*m_vecIndices[i]+2 ] > max[2] )
			max[2] = m_pVertices[ 3*m_vecIndices[i]+2 ];

		if ( m_pVertices[ 3*m_vecIndices[i]+0 ] < min[0] )
			min[0] = m_pVertices[ 3*m_vecIndices[i]+0 ];
		if ( m_pVertices[ 3*m_vecIndices[i]+1 ] < min[1] )
			min[1] = m_pVertices[ 3*m_vecIndices[i]+1 ];
		if ( m_pVertices[ 3*m_vecIndices[i]+2 ] < min[2] )
			min[2] = m_pVertices[ 3*m_vecIndices[i]+2 ];
	}
}

void Geometry::Split( char axis, float middle, Geometry **ppLeft, Geometry** ppRight )
{
	if ( m_vecIndices.size() <= 3 )
	{
		*ppRight = *ppLeft = 0;
		return;
	}

	*ppLeft = new Geometry;
	*ppRight = new Geometry;
	(*ppLeft)->m_pVertices = (*ppRight)->m_pVertices = m_pVertices;

	for ( size_t f = 0 ; f < m_vecIndices.size()/3 ; f++ )
	{
		short ti0 = m_vecIndices[3*f+0];
		short ti1 = m_vecIndices[3*f+1];
		short ti2 = m_vecIndices[3*f+2];

		float v0 = m_pVertices[ 3*ti0+axis ];
		float v1 = m_pVertices[ 3*ti1+axis ];
		float v2 = m_pVertices[ 3*ti2+axis ];

		if ( v0 > middle && v1 > middle && v2 > middle )
		{
			(*ppRight)->m_vecIndices.push_back( m_vecIndices[3*f+0] );
			(*ppRight)->m_vecIndices.push_back( m_vecIndices[3*f+1] );
			(*ppRight)->m_vecIndices.push_back( m_vecIndices[3*f+2] );
		}
		else if ( v0 < middle && v1 < middle && v2 < middle )
		{
			(*ppLeft)->m_vecIndices.push_back( m_vecIndices[3*f+0] );
			(*ppLeft)->m_vecIndices.push_back( m_vecIndices[3*f+1] );
			(*ppLeft)->m_vecIndices.push_back( m_vecIndices[3*f+2] );
		}
		else
		{
			(*ppRight)->m_vecIndices.push_back( m_vecIndices[3*f+0] );
			(*ppRight)->m_vecIndices.push_back( m_vecIndices[3*f+1] );
			(*ppRight)->m_vecIndices.push_back( m_vecIndices[3*f+2] );
		
			(*ppLeft)->m_vecIndices.push_back( m_vecIndices[3*f+0] );
			(*ppLeft)->m_vecIndices.push_back( m_vecIndices[3*f+1] );
			(*ppLeft)->m_vecIndices.push_back( m_vecIndices[3*f+2] );
		}
	}

#ifdef LOGGING
	if ( ( (*ppRight)->m_vecIndices.empty() && (*ppLeft)->m_vecIndices.size() > 3 ) ||
		( (*ppRight)->m_vecIndices.size() > 3 && (*ppLeft)->m_vecIndices.empty() ) )
	{
		FILE *pLog = fopen( "log.txt", "at" );
		fprintf( pLog, "axis : %d, split Value : %.3f\n", axis, middle );
		
		for ( size_t f = 0 ; f < m_vecIndices.size()/3 ; f++ )
		{
			short ti0 = m_vecIndices[3*f+0];
			short ti1 = m_vecIndices[3*f+1];
			short ti2 = m_vecIndices[3*f+2];

			float v0 = m_pVertices[ 3*ti0+axis ];
			float v1 = m_pVertices[ 3*ti1+axis ];
			float v2 = m_pVertices[ 3*ti2+axis ];

			float triCenter = ( v0 + v1 + v2 ) / 3.0f;
			fprintf( pLog, "%.3f, %.3f, %.3f : %.3f\n", v0, v1, v2, triCenter );
		}
		
		fclose( pLog );
	}
#endif

	if ( (*ppRight)->m_vecIndices.empty() )
	{
		delete (*ppRight);
		(*ppRight) = 0;
	}
	
	if ( (*ppLeft)->m_vecIndices.empty() )
	{
		delete (*ppLeft);
		(*ppLeft) = 0;
	}
}

void Geometry::ReadFromFile( const char* path )
{
	FILE *pFile = fopen( path, "rb" );
	if ( pFile )
	{
		fread( &m_nVeritces, sizeof(int), 1, pFile );
		m_pVertices = new float[m_nVeritces*3];
		fread( m_pVertices, sizeof(float)*m_nVeritces*3, 1, pFile );

		size_t indices = 0;
		fread( &indices, sizeof(size_t), 1, pFile );
		m_vecIndices.resize( indices );

		for ( size_t i = 0 ; i < m_vecIndices.size() ; i++ )
			fread( &m_vecIndices[i], sizeof(short), 1, pFile );

		fclose( pFile );
	}
}

void Geometry::WriteToFile( const char* path )
{
	FILE *pFile = fopen( path, "wb" );
	if ( pFile )
	{
		fwrite( &m_nVeritces, sizeof(int), 1, pFile );
		fwrite( m_pVertices, sizeof(float)*m_nVeritces*3, 1, pFile );

		size_t indices = m_vecIndices.size();
		fwrite( &indices, sizeof(size_t), 1, pFile );
		
		for ( size_t i = 0 ; i < m_vecIndices.size() ; i++ )
			fwrite( &m_vecIndices[i], sizeof(short), 1, pFile );

		fclose( pFile );
	}
}

bool IntersectTriangle( const float orig[], const float dir[], 
					   float v0[], float v1[], float v2[], 
					   float* t, 
					   float* u, 
					   float* v );

bool Geometry::CastingRay( RaySeg rayseg, float coord[3], float &t )
{
	float shortedT = FLT_MAX;
	for ( size_t f = 0 ; f < m_vecIndices.size() / 3 ; f++ )
	{
		float v0[3];
		float v1[3];
		float v2[3];

		short ti0 = m_vecIndices[3*f+0];
		short ti1 = m_vecIndices[3*f+1];
		short ti2 = m_vecIndices[3*f+2];

		memcpy( v0, m_pVertices+3*ti0, sizeof(float)*3 );
		memcpy( v1, m_pVertices+3*ti1, sizeof(float)*3 );
		memcpy( v2, m_pVertices+3*ti2, sizeof(float)*3 );

		float u, v;
		if ( IntersectTriangle( rayseg.m_Org, rayseg.m_Dir, v0, v1, v2, &t, &u, &v ) )
		{
			if ( t < shortedT )
			{
				shortedT = t;

				coord[0] = v0[0] + u * (v1[0] - v0[0]) + v * (v2[0] - v0[0]);
				coord[1] = v0[1] + u * (v1[1] - v0[1]) + v * (v2[1] - v0[1]);
				coord[2] = v0[2] + u * (v1[2] - v0[2]) + v * (v2[2] - v0[2]);
			}
		}
	}

	if ( shortedT < rayseg.m_tEnd )
	{
		t = shortedT;
		return true;
	}

	return false;
}