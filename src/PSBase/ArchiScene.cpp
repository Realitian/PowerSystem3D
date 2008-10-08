#include "ArchiScene.h"
#include <float.h>

Node::Node()
{
	memset( m_Trans, 0, sizeof(nv_scalar)*16 );
	type = OTHER;
	geometry = 0;
}

OutSide::~OutSide()
{
	for( size_t i = 0 ; i < m_pNodes.size() ; i++ )
		SAFE_DELETE(m_pNodes[i]);
}

Building::Building()
{
	m_glTex = 0;
	m_pOutline = 0;
}

Building::~Building()
{
	SAFE_DELETE_ARRAY( m_glTex );
	SAFE_DELETE( m_pOutline );

	for( size_t i = 0 ; i < m_pFloors.size() ; i++ )
		SAFE_DELETE(m_pFloors[i]);
}

Floor::Floor()
{
	m_pPre = 0;
	m_pNext = 0;
	m_MinZ = m_MaxZ = 0;
}

Floor::~Floor()
{
	for( size_t i = 0 ; i < m_pRooms.size() ; i++ )
		SAFE_DELETE(m_pRooms[i]);

	for( size_t i = 0 ; i < m_pNoSlabs.size() ; i++ )
		SAFE_DELETE(m_pNoSlabs[i]);

	for( size_t i = 0 ; i < m_pSlabs.size() ; i++ )
		SAFE_DELETE(m_pSlabs[i]);

	for( size_t i = 0 ; i < m_pSteps.size() ; i++ )
		SAFE_DELETE(m_pSteps[i]);

	for( size_t i = 0 ; i < m_pAllWalls.size() ; i++ )
		SAFE_DELETE(m_pAllWalls[i]);
}

Room::~Room()
{
	for( size_t i = 0 ; i < m_pInsids.size() ; i++ )
	{
		if ( m_pInsids[i]->type == Node::SLAB )
			m_pInsids[i] = 0;
		else
			SAFE_DELETE(m_pInsids[i]);
	}

	SAFE_DELETE_ARRAY( m_vVertices );
}

Wall::~Wall()
{
	for( size_t i = 0 ; i < m_pDoors.size() ; i++ )
		SAFE_DELETE(m_pDoors[i]);
}

DoorCell::DoorCell()
{
	opentype = OT_None;
	openingstate = DOOR_CLOSED;
}

Door::~Door()
{
	for (size_t i = 0 ; i < pCells.size() ; i++ )
		SAFE_DELETE(pCells[i]);

	for (size_t i = 0 ; i < pMixDoors.size() ; i++ )
		SAFE_DELETE(pMixDoors[i]);

	pCells.clear();
	pMixDoors.clear();
}

Archi::Archi()
{
	m_pCurrentRoom = 0;
	m_pCurrentFloor = 0;
	m_pPickedNode = 0;
	m_FloorZ = 0;
}

Archi::~Archi()
{
	for( size_t i = 0 ; i < m_pGeoList.size() ; i++ )
		SAFE_DELETE(m_pGeoList[i]);

	for( size_t i = 0 ; i < m_pMaterials.size() ; i++ )
		SAFE_DELETE(m_pMaterials[i]);

	for( size_t i = 0 ; i < m_pBuildings.size() ; i++ )
		SAFE_DELETE(m_pBuildings[i]);
}

Geometry::Geometry()
{
	m_nNumFace = 0;
	m_nNumVertices = 0;
	mat = 0;
	m_bIsCulling = true;
	m_glDlistId = 0;

	m_pVertices = 0;
	m_pNormals = 0;
	m_pTexCoords = 0;
	m_pIndices = 0;
};

Geometry::~Geometry()
{
	m_glDlistId = 0;

	SAFE_DELETE_ARRAY(m_pVertices);
	SAFE_DELETE_ARRAY(m_pNormals);
	SAFE_DELETE_ARRAY(m_pTexCoords);
	SAFE_DELETE_ARRAY(m_pIndices);
}

GeoGroup::GeoGroup()
{
	m_nNumGeo = 0;
	m_pGeos = 0;
}

GeoGroup::~GeoGroup()
{
	SAFE_DELETE_ARRAY(m_pGeos);
	m_nNumGeo = 0;
}

ViewPos::ViewPos()
{
	memset( pos, 0, sizeof(nv_scalar)*3 );
	memset( look, 0, sizeof(nv_scalar)*3 );
	memset( up, 0, sizeof(nv_scalar)*3 );
	memset( right, 0, sizeof(nv_scalar)*3 );
	fovy = 0;
}

CBnsImage::~CBnsImage()
{
	SAFE_DELETE_ARRAY(pPixels);
}

CMaterial::~CMaterial()
{
}

void Node::CalcAABB()
{

}

void Floor::CalcAABB()
{

}

void Room::CalcAABB()
{

}

void GeoGroup::CalcAABB()
{
#define DIM 3
	m_aabb.m_Min[0] = FLT_MAX;
	m_aabb.m_Min[1] = FLT_MAX;
	m_aabb.m_Min[2] = FLT_MAX;

	m_aabb.m_Max[0] = -FLT_MAX;
	m_aabb.m_Max[1] = -FLT_MAX;
	m_aabb.m_Max[2] = -FLT_MAX;

	for (int i = 0; i < m_nNumGeo; i++)
	{
		if (m_pGeos[i].m_pIndices)
		{
			for ( int f = 0 ; f < m_pGeos[i].m_nNumFace*3; f++)
			{
				int v = m_pGeos[i].m_pIndices[f];
				if ( m_pGeos[i].m_pVertices[DIM*v] > m_aabb.m_Max[0] )
					m_aabb.m_Max[0] = m_pGeos[i].m_pVertices[DIM*v];
				if ( m_pGeos[i].m_pVertices[DIM*v+1] > m_aabb.m_Max[1] )
					m_aabb.m_Max[1] = m_pGeos[i].m_pVertices[DIM*v+1];
				if ( m_pGeos[i].m_pVertices[DIM*v+2] > m_aabb.m_Max[2] )
					m_aabb.m_Max[2] = m_pGeos[i].m_pVertices[DIM*v+2];

				if ( m_pGeos[i].m_pVertices[DIM*v] < m_aabb.m_Min[0] )
					m_aabb.m_Min[0] = m_pGeos[i].m_pVertices[DIM*v];
				if ( m_pGeos[i].m_pVertices[DIM*v+1] < m_aabb.m_Min[1] )
					m_aabb.m_Min[1] = m_pGeos[i].m_pVertices[DIM*v+1];
				if ( m_pGeos[i].m_pVertices[DIM*v+2] < m_aabb.m_Min[2] )
					m_aabb.m_Min[2] = m_pGeos[i].m_pVertices[DIM*v+2];
			}
		}
		else
		{
			for( int v = 0 ; v < m_pGeos[i].m_nNumFace*4 ; v++ )
			{
				if ( m_pGeos[i].m_pVertices[DIM*v] > m_aabb.m_Max[0] )
					m_aabb.m_Max[0] = m_pGeos[i].m_pVertices[DIM*v];
				if ( m_pGeos[i].m_pVertices[DIM*v+1] > m_aabb.m_Max[1] )
					m_aabb.m_Max[1] = m_pGeos[i].m_pVertices[DIM*v+1];
				if ( m_pGeos[i].m_pVertices[DIM*v+2] > m_aabb.m_Max[2] )
					m_aabb.m_Max[2] = m_pGeos[i].m_pVertices[DIM*v+2];

				if ( m_pGeos[i].m_pVertices[DIM*v] < m_aabb.m_Min[0] )
					m_aabb.m_Min[0] = m_pGeos[i].m_pVertices[DIM*v];
				if ( m_pGeos[i].m_pVertices[DIM*v+1] < m_aabb.m_Min[1] )
					m_aabb.m_Min[1] = m_pGeos[i].m_pVertices[DIM*v+1];
				if ( m_pGeos[i].m_pVertices[DIM*v+2] < m_aabb.m_Min[2] )
					m_aabb.m_Min[2] = m_pGeos[i].m_pVertices[DIM*v+2];
			}
		}
	}
}

struct CoordEntry {
	CoordEntry* next;
	nv_scalar		coordinate;
};
void InsertCoordEntry(CoordEntry** inList,CoordEntry* inItem,long inLength);

bool PtInPolygon( const nv_scalar inPoints[], const long inPtCount, const nv_scalar inTestPt[], const int iDim)
{
	CoordEntry*	HArray = 0;
	CoordEntry*	theHArray = 0;
	short	k,hCount = 0;
	nv_scalar	thePt1[2],thePt2[2],nextPt[2],prevPt[2];

	if(inPtCount < 3) return false;

	thePt2[0] = inPoints[iDim*(inPtCount-1)];
	thePt2[1] = inPoints[iDim*(inPtCount-1)+1];

	prevPt[0] = inPoints[iDim*(inPtCount-2)];
	prevPt[1] = inPoints[iDim*(inPtCount-2)+1];

	for(long i = 0 ; i<inPtCount; i++) {
		thePt1[0] = inPoints[iDim*i];
		thePt1[1] = inPoints[iDim*i+1];
		if(i == inPtCount-1) 
		{
			nextPt[0] = inPoints[0];
			nextPt[1] = inPoints[1];
		}
		else 
		{
			nextPt[0] = inPoints[iDim*(i+1)];
			nextPt[1] = inPoints[iDim*(i+1)+1];
		}

		if((thePt1[0] == thePt2[0]) && (thePt1[1] == thePt2[1])) continue;		
		if((thePt1[1] == nextPt[1]) && (thePt1[1] == thePt2[1])) continue;

		if(thePt1[1]  == inTestPt[1]) {
			if((thePt2[1] > thePt1[1] && thePt1[1] > nextPt[1]) ||
				(thePt2[1] < thePt1[1] && thePt1[1] < nextPt[1])) {		
					theHArray = new  CoordEntry;			
					theHArray->coordinate = thePt1[0];
					InsertCoordEntry(&HArray,theHArray,hCount);
					hCount++;
			} else if(thePt1[1] == thePt2[1]) {
				theHArray = new  CoordEntry;			
				theHArray->coordinate = thePt2[0];
				InsertCoordEntry(&HArray,theHArray,hCount);
				hCount++;
				if((prevPt[1] > thePt2[1] && thePt1[1] < nextPt[1]) ||
					(prevPt[1] < thePt2[1] && thePt1[1] > nextPt[1])) {
						theHArray = new  CoordEntry;			
						theHArray->coordinate = thePt1[0];
						InsertCoordEntry(&HArray,theHArray,hCount);
						hCount++;
				}
			}
		} else if(((thePt2[1] < inTestPt[1]) && (thePt1[1] > inTestPt[1])) ||
			((thePt1[1] < inTestPt[1]) && (thePt2[1] > inTestPt[1]))) {
				theHArray = new  CoordEntry;			
				theHArray->coordinate = thePt1[0] + ((inTestPt[1] - thePt1[1])*(thePt2[0] - thePt1[0]))/((thePt2[1] - thePt1[1]));
				InsertCoordEntry(&HArray,theHArray,hCount);
				hCount++;
		}
		prevPt[0] = thePt2[0];
		prevPt[1] = thePt2[1];

		thePt2[0] = thePt1[0];
		thePt2[1] = thePt1[1];
	}

	// here check test point
	CoordEntry* thePtItem = HArray;
	double theTestPt_h = inTestPt[0];
	if(hCount != 0) {
		thePtItem = HArray;
		for( ;thePtItem->next != NULL; ) {
			if((theTestPt_h <= (thePtItem->next)->coordinate) && (theTestPt_h >= thePtItem->coordinate)) {
				CoordEntry* theNextItem;
				for(k = 0;k<hCount;k++) {
					theNextItem = HArray->next;
					if(HArray != NULL) delete HArray;
					HArray = theNextItem;
				}
				return(true); 
			}
			thePtItem = (thePtItem->next)->next;
			if(thePtItem == NULL) break;
		}
	}

	CoordEntry* theNextItem;
	for(k = 0;k<hCount;k++) {
		theNextItem = HArray->next;
		if(HArray != NULL) delete HArray;
		HArray = theNextItem;
	} 
	return(false);	
}

void InsertCoordEntry(CoordEntry** inList,CoordEntry* inItem,long inLength)
{
	if(inLength == 0) {
		inItem->next = NULL;
		*inList = inItem;
		return;
	}

	CoordEntry* theItem = *inList;
	if(theItem->coordinate > inItem->coordinate) {
		inItem->next = theItem;
		*inList = inItem;
		return;
	}
	for(long i = 0;i<inLength-1; i++) {
		if((theItem->coordinate <= inItem->coordinate) &&
			((theItem->next)->coordinate >= inItem->coordinate))
		{
			inItem->next = theItem->next;
			theItem->next = inItem;
			return;
		}
		theItem = theItem->next;
	}
	theItem->next = inItem;
	inItem->next = NULL;
}

//////////////////////////////////////////////////////////////////////////
///////////				FILE I/O Functions					   ///////////
//////////////////////////////////////////////////////////////////////////

void OutSide::ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	size_t nodes;
	fread( &nodes, sizeof(size_t), 1, pFile );
	for( size_t f = 0 ; f < nodes ; f++ )
	{
		Node* pNode = new Node;
		pNode->ReadFromFile( pFile, geopool );
		m_pNodes.push_back( pNode );
	}
}

void OutSide::WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	size_t nodes = m_pNodes.size();
	fwrite( &nodes, sizeof(size_t), 1, pFile );

	for( size_t n = 0 ; n < nodes ; n++ )
		m_pNodes[n]->WriteToFile( pFile, geopool );
}


void Building::ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	char s = 0;
	fread( &s, sizeof(char), 1, pFile );
	if ( s == 1 )
	{
		m_pOutline = new Node;
		m_pOutline->ReadFromFile( pFile, geopool );
	}

	size_t bb = 0;
	fread( &bb, sizeof(bb), 1, pFile );
	m_vecHallPoints.resize(bb);
	for (size_t i = 0 ; i<bb ; i++)
		fread( &m_vecHallPoints[i], sizeof(AABB), 1, pFile );

	size_t floors;
	fread( &floors, sizeof(size_t), 1, pFile );

	for( size_t f = 0 ; f < floors ; f++ )
	{
		Floor* pFloor = new Floor;
		pFloor->ReadFromFile( pFile, geopool );
		m_pFloors.push_back( pFloor );
	}
	
	vector< int > floorOrder;
	for ( size_t f = 0 ; f < floors ; f++ )
	{
		int order;
		fread( &order, sizeof(int), 1, pFile );
		floorOrder.push_back( order );
	}

	if( floors > 0 )
	{
		for ( size_t f = 0 ; f < floors-1 ; f++ )
		{
			m_pFloors[ floorOrder[f] ]->m_pNext = m_pFloors[ floorOrder[f+1] ];
			m_pFloors[ floorOrder[f+1] ]->m_pPre = m_pFloors[ floorOrder[f] ];
		}

		m_pFloors[ floorOrder[0] ]->m_pPre = 0;
		m_pFloors[ floorOrder[floors-1] ]->m_pNext = 0;
	}
}

void Building::Scale( float s )
{
	m_pOutline->Scale(s);

	for (size_t i=0 ; i<m_vecHallPoints.size() ; i++)
		m_vecHallPoints[i].Scale(s);

	for (size_t i=0 ; i<m_pFloors.size() ; i++)
		m_pFloors[i]->Scale(s);
}

void Building::WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	char s = 0;
	if ( m_pOutline )
		s = 1;
	fwrite( &s, sizeof(char), 1, pFile );
	if ( m_pOutline )
		m_pOutline->WriteToFile( pFile, geopool );

	size_t bb = m_vecHallPoints.size();
	fwrite( &bb, sizeof(bb), 1, pFile );
	for (size_t i = 0 ; i<bb ; i++)
		fwrite( &m_vecHallPoints[i], sizeof(AABB), 1, pFile );

	size_t floors = m_pFloors.size();
	fwrite( &floors, sizeof(size_t), 1, pFile );

	for( size_t f = 0 ; f < floors ; f++ )
		m_pFloors[f]->WriteToFile( pFile, geopool );

	vector< int > floorOrder;
	if ( floors > 0 )
	{
		Floor* pCur = m_pFloors[0];
		while( 1 )
		{
			if ( pCur->m_pPre )
				pCur = pCur->m_pPre;
			else
				break;
		}

		while( pCur )
		{
			for ( size_t f = 0 ; f < floors ; f++ )
			{
				if ( pCur == m_pFloors[f] )
				{
					floorOrder.push_back( f );
					break;
				}
			}

			pCur = pCur->m_pNext;
		}

		for ( size_t f = 0 ; f < floors ; f++ )
			fwrite( &floorOrder[f], sizeof(int), 1, pFile );
	}
}

void Floor::Scale( float s )
{
	Node::Scale(s);
	m_MinZ *= s;
	m_MaxZ *= s;

	for (size_t i = 0 ; i < m_pAllWalls.size() ; i++)
		m_pAllWalls[i]->Scale(s);

	for (size_t i = 0 ; i < m_pRooms.size() ; i++)
		m_pRooms[i]->Scale(s);

	for (size_t i = 0 ; i < m_pNoSlabs.size() ; i++)
		m_pNoSlabs[i]->Scale(s);

	for (size_t i = 0 ; i < m_pSteps.size() ; i++)
		m_pSteps[i]->Scale(s);
}

void Floor::ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	fread( &m_MinZ, sizeof(nv_scalar), 1, pFile );
	fread( &m_MaxZ, sizeof(nv_scalar), 1, pFile );

	size_t rooms;
	fread( &rooms, sizeof(size_t), 1, pFile );
	for( size_t r = 0 ; r < rooms ; r++ )
	{
		Room* pRoom = new Room;
		pRoom->ReadFromFile( pFile, geopool );
		m_pRooms.push_back( pRoom );

		for ( size_t o = 0 ; o < pRoom->m_pInsids.size() ; o++ )
		{
			if ( pRoom->m_pInsids[o]->type == Node::SLAB )
				m_pSlabs.push_back( pRoom->m_pInsids[o] );
		}
	}

	size_t walls;
	fread( &walls, sizeof(size_t), 1, pFile );
	for( size_t w = 0 ; w < walls ; w++ )
	{
		Wall* pWall = new Wall;
		pWall->ReadFromFile( pFile, geopool );
		pWall->ReadFromFile( pFile, m_pRooms );
		m_pAllWalls.push_back( pWall );

		if ( pWall->m_pRooms.size() < 2 )
			m_pOutSideWalls.push_back( pWall );
	}

	for( size_t r = 0 ; r < rooms ; r++ )
		m_pRooms[r]->ReadFromFile( pFile, m_pAllWalls );

	size_t noslabs;
	fread( &noslabs, sizeof(size_t), 1, pFile );
	for( size_t s = 0 ; s < noslabs ; s++ )
	{
		Node* pNoSlabs = new Node;
		pNoSlabs->ReadFromFile( pFile, geopool );
		m_pNoSlabs.push_back( pNoSlabs );
	}

	size_t steps;
	fread( &steps, sizeof(size_t), 1, pFile );
	for( size_t s = 0 ; s < steps ; s++ )
	{
		Node* pStep = new Node;
		pStep->ReadFromFile( pFile, geopool );
		m_pSteps.push_back( pStep );
	}

	for ( size_t r = 0 ; r < m_pRooms.size() ; r++ )
	{
		for ( size_t i = 0 ; i < m_pRooms[r]->m_pInsids.size() ; i++ )
		{
			if ( m_pRooms[r]->m_pInsids[i]->type == Node::STEPS )
			{
				m_pStepRooms.push_back( m_pRooms[r] );
				break;
			}
		}
	}
}

void Floor::WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	fwrite( &m_MinZ, sizeof(nv_scalar), 1, pFile );
	fwrite( &m_MaxZ, sizeof(nv_scalar), 1, pFile );

	size_t rooms = m_pRooms.size();
	fwrite( &rooms, sizeof(size_t), 1, pFile );
	for( size_t r = 0 ; r < rooms ; r++ )
		m_pRooms[r]->WriteToFile( pFile, geopool );
	
	size_t walls = m_pAllWalls.size();
	fwrite( &walls, sizeof(size_t), 1, pFile );
	for( size_t w = 0 ; w < walls ; w++ )
	{
		m_pAllWalls[w]->WriteToFile( pFile, geopool );
		m_pAllWalls[w]->WriteToFile( pFile, m_pRooms );
	}

	for( size_t r = 0 ; r < rooms ; r++ )
		m_pRooms[r]->WriteToFile( pFile, m_pAllWalls );

	size_t noslabs = m_pNoSlabs.size();
	fwrite( &noslabs, sizeof(size_t), 1, pFile );
	for( size_t s = 0 ; s < noslabs ; s++ )
		m_pNoSlabs[s]->WriteToFile( pFile, geopool );

	size_t steps = m_pSteps.size();
	fwrite( &steps, sizeof(size_t), 1, pFile );
	for( size_t s = 0 ; s < steps ; s++ )
		m_pSteps[s]->WriteToFile( pFile, geopool );
}

bool Archi::ReadFromFile( char* path )
{
	FILE* pFile = fopen( path, "rb" );
	if ( pFile )
	{
		//1.
		fread( m_AmbientRGB, sizeof(float)*4, 1, pFile );

		//2.
		fread( m_Range.m_Min, sizeof(float)*3, 1, pFile );
		fread( m_Range.m_Max, sizeof(float)*3, 1, pFile );

		//3.
		size_t lights = 0;
		fread( &lights, sizeof( size_t ), 1, pFile );
		for ( size_t li = 0 ; li < lights ; li++ )
		{
			Light light;
			light.ReadFromFile( pFile );
			m_Lights.push_back( light );
		}

		//4.
		size_t viewpoints = 0;
		fread( &viewpoints, sizeof( size_t ), 1, pFile );
		for ( size_t vi = 0 ; vi < viewpoints ; vi++ )
		{
			ViewPos viewpos;
			viewpos.ReadFromFile( pFile );
			m_ViewPoints.push_back( viewpos );
		}

		//5.
		size_t images = 0;
		fread( &images, sizeof( size_t ), 1, pFile );
		for ( size_t ii = 0 ; ii < images ; ii++ )
		{
			CBnsImage* pImage = new CBnsImage;
			pImage->ReadFromFile( pFile );
			m_pImages.push_back( pImage );
		}

		//6.
		size_t materials = 0;
		fread( &materials, sizeof( size_t ), 1, pFile );
		for ( size_t mi = 0 ; mi < materials ; mi++ )
		{
			CMaterial* pMaterial = new CMaterial;
			pMaterial->ReadFromFile( pFile, m_pImages );
			m_pMaterials.push_back( pMaterial );
		}

		//6.
		size_t geogroups = 0;
		fread( &geogroups, sizeof( size_t ), 1, pFile );
		for ( size_t gi = 0 ; gi < geogroups ; gi++ )
		{
			GeoGroup* pGeoGroup = new GeoGroup;
			pGeoGroup->ReadFromFile( pFile, m_pMaterials );
			m_pGeoList.push_back( pGeoGroup );
		}

		//7.
		m_Outside.ReadFromFile( pFile, m_pGeoList );

		//8.
		size_t buildings = 0;
		fread( &buildings, sizeof( size_t ), 1, pFile );
		for ( size_t bi = 0 ; bi < buildings ; bi++ )
		{
			Building* pBuilding = new Building;
			pBuilding->ReadFromFile( pFile, m_pGeoList );
			m_pBuildings.push_back( pBuilding );
		}

		fclose( pFile );
		return true;
	}

	return false;
}

bool Archi::WriteToFile( char* path )
{
	FILE* pFile = fopen( path, "wb" );
	if ( pFile )
	{
		//1.
		fwrite( m_AmbientRGB, sizeof(float)*4, 1, pFile );

		//2.
		fwrite( m_Range.m_Min, sizeof(float)*3, 1, pFile );
		fwrite( m_Range.m_Max, sizeof(float)*3, 1, pFile );

		//3.
		size_t lights = m_Lights.size();
		fwrite( &lights, sizeof( size_t ), 1, pFile );
		for ( size_t li = 0 ; li < lights ; li++ )
			m_Lights[li].WriteToFile( pFile );

		//4.
		size_t viewpoints = m_ViewPoints.size();
		fwrite( &viewpoints, sizeof( size_t ), 1, pFile );
		for ( size_t vi = 0 ; vi < viewpoints ; vi++ )
			m_ViewPoints[vi].WriteToFile( pFile );

		//5.
		size_t images = m_pImages.size();
		fwrite( &images, sizeof( size_t ), 1, pFile );
		for ( size_t ii = 0 ; ii < images ; ii++ )
			m_pImages[ii]->WriteToFile( pFile );

		//6.
		size_t materials = m_pMaterials.size();
		fwrite( &materials, sizeof( size_t ), 1, pFile );
		for ( size_t mi = 0 ; mi < materials ; mi++ )
			m_pMaterials[mi]->WriteToFile( pFile, m_pImages );

		//6.
		size_t geogroups = m_pGeoList.size();
		fwrite( &geogroups, sizeof( size_t ), 1, pFile );
		for ( size_t gi = 0 ; gi < geogroups ; gi++ )
			m_pGeoList[gi]->WriteToFile( pFile, m_pMaterials );

		//7.
		m_Outside.WriteToFile( pFile, m_pGeoList );

		//8.
		size_t buildings = m_pBuildings.size();
		fwrite( &buildings, sizeof( size_t ), 1, pFile );
		for ( size_t bi = 0 ; bi < buildings ; bi++ )
			m_pBuildings[bi]->WriteToFile( pFile, m_pGeoList );

		fclose( pFile );
		return true;
	}

	return false;
}

void Geometry::Scale( float s )
{
	if( m_pIndices )
		for ( int i = 0 ; i < 9*m_nNumVertices ; i++ )
			m_pVertices[i] *= s;
	else
		for ( int i = 0 ; i < 12*m_nNumFace ; i++ )
			m_pVertices[i] *= s;
}

void Geometry::ReadFromFile( FILE* pFile, vector< CMaterial* >& matpool )
{
	fread( &m_bIsCulling, sizeof(bool), 1, pFile );
	fread( &m_nNumFace, sizeof(int), 1, pFile );
	fread( &m_nNumVertices, sizeof(int), 1, pFile );
	if ( m_nNumVertices > 0 )
	{
		m_pIndices = new short[3*m_nNumFace];
		m_pVertices = new nv_scalar[9*m_nNumVertices];
		m_pNormals = new nv_scalar[9*m_nNumVertices];
		m_pTexCoords = new nv_scalar[6*m_nNumVertices];
		
		fread( m_pIndices, sizeof(short)*m_nNumFace*3, 1, pFile );
		fread( m_pVertices, sizeof(nv_scalar)*9*m_nNumVertices, 1, pFile );
		fread( m_pNormals, sizeof(nv_scalar)*9*m_nNumVertices, 1, pFile );
		fread( m_pTexCoords, sizeof(nv_scalar)*6*m_nNumVertices, 1, pFile );
	}
	else
	{
		m_pVertices = new nv_scalar[12*m_nNumFace];
		m_pNormals = new nv_scalar[12*m_nNumFace];
		m_pTexCoords = new nv_scalar[8*m_nNumFace];

		fread( m_pVertices, sizeof(nv_scalar)*12*m_nNumFace, 1, pFile );
		fread( m_pNormals, sizeof(nv_scalar)*12*m_nNumFace, 1, pFile );
		fread( m_pTexCoords, sizeof(nv_scalar)*8*m_nNumFace, 1, pFile );
	}

	int id = -1;
	fread( &id, sizeof(int), 1, pFile );
	mat = 0;
	if ( id >= 0 && id < matpool.size() )
		mat = matpool[id];

	fread( color, sizeof(unsigned char)*3, 1, pFile );
}

void Geometry::WriteToFile( FILE* pFile, vector< CMaterial* >& matpool )
{
	fwrite( &m_bIsCulling, sizeof(bool), 1, pFile );
	fwrite( &m_nNumFace, sizeof(int), 1, pFile );
	
	fwrite( &m_nNumVertices, sizeof(int), 1, pFile );
	if ( m_nNumVertices > 0 )
	{
		fwrite( m_pIndices, sizeof(short)*m_nNumFace*3, 1, pFile );
		fwrite( m_pVertices, sizeof(nv_scalar)*9*m_nNumVertices, 1, pFile );
		fwrite( m_pNormals, sizeof(nv_scalar)*9*m_nNumVertices, 1, pFile );
		fwrite( m_pTexCoords, sizeof(nv_scalar)*6*m_nNumVertices, 1, pFile );
	}
	else
	{
		fwrite( m_pVertices, sizeof(nv_scalar)*12*m_nNumFace, 1, pFile );
		fwrite( m_pNormals, sizeof(nv_scalar)*12*m_nNumFace, 1, pFile );
		fwrite( m_pTexCoords, sizeof(nv_scalar)*8*m_nNumFace, 1, pFile );
	}

	int id = -1;
	for ( size_t m = 0 ; m < matpool.size() ; m++ )
	{
		if ( matpool[m] == mat )
		{
			id = m;
			break;
		}
	}
	fwrite( &id, sizeof(int), 1, pFile );

	fwrite( color, sizeof(unsigned char)*3, 1, pFile );
}

void GeoGroup::ReadFromFile( FILE* pFile, vector< CMaterial* >& matpool )
{
	fread( &m_nNumGeo, sizeof(int), 1, pFile );
	m_pGeos = new Geometry[m_nNumGeo];

	for ( int gi = 0 ; gi < m_nNumGeo ; gi++ )
		m_pGeos[gi].ReadFromFile( pFile, matpool );
}

void GeoGroup::WriteToFile( FILE* pFile, vector< CMaterial* >& matpool )
{
	fwrite( &m_nNumGeo, sizeof(int), 1, pFile );

	for ( int gi = 0 ; gi < m_nNumGeo ; gi++ )
		m_pGeos[gi].WriteToFile( pFile, matpool );
}

bool GeoGroup::IsSame( GeoGroup *other )
{


	return false;
}

void ViewPos::ReadFromFile( FILE* pFile )
{
	fread( pos, sizeof(nv_scalar)*3, 1, pFile );
	fread( look, sizeof(nv_scalar)*3, 1, pFile );
	fread( up, sizeof(nv_scalar)*3, 1, pFile );
	fread( right, sizeof(nv_scalar)*3, 1, pFile );
	fread( &fovy, sizeof(nv_scalar), 1, pFile );
}

void ViewPos::WriteToFile( FILE* pFile )
{
	fwrite( pos, sizeof(nv_scalar)*3, 1, pFile );
	fwrite( look, sizeof(nv_scalar)*3, 1, pFile );
	fwrite( up, sizeof(nv_scalar)*3, 1, pFile );
	fwrite( right, sizeof(nv_scalar)*3, 1, pFile );
	fwrite( &fovy, sizeof(nv_scalar), 1, pFile );
}

void Light::ReadFromFile( FILE* pFile )
{
	fread( ambient, sizeof(nv_scalar)*4, 1, pFile );
	fread( diffuse, sizeof(nv_scalar)*4, 1, pFile );
	fread( specular, sizeof(nv_scalar)*4, 1, pFile );

	fread( &attenuation0, sizeof(nv_scalar), 1, pFile );
	fread( &attenuation1, sizeof(nv_scalar), 1, pFile );
	fread( &attenuation2, sizeof(nv_scalar), 1, pFile );
	fread( &cutoff, sizeof(int), 1, pFile );
	fread( &iner, sizeof(nv_scalar), 1, pFile );
	fread( &outer, sizeof(nv_scalar), 1, pFile );
	fread( &exponent, sizeof(int), 1, pFile );
	fread( direction, sizeof(nv_scalar)*4, 1, pFile );
	fread( position, sizeof(nv_scalar)*4, 1, pFile );

	fread( &type, sizeof(LIGHT_TYPE), 1, pFile );
}

void Light::WriteToFile( FILE* pFile )
{
	fwrite( ambient, sizeof(nv_scalar)*4, 1, pFile );
	fwrite( diffuse, sizeof(nv_scalar)*4, 1, pFile );
	fwrite( specular, sizeof(nv_scalar)*4, 1, pFile );

	fwrite( &attenuation0, sizeof(nv_scalar), 1, pFile );
	fwrite( &attenuation1, sizeof(nv_scalar), 1, pFile );
	fwrite( &attenuation2, sizeof(nv_scalar), 1, pFile );
	fwrite( &cutoff, sizeof(int), 1, pFile );
	fwrite( &iner, sizeof(nv_scalar), 1, pFile );
	fwrite( &outer, sizeof(nv_scalar), 1, pFile );
	fwrite( &exponent, sizeof(int), 1, pFile );
	fwrite( direction, sizeof(nv_scalar)*4, 1, pFile );
	fwrite( position, sizeof(nv_scalar)*4, 1, pFile );

	fwrite( &type, sizeof(LIGHT_TYPE), 1, pFile );
}

void CBnsImage::ReadFromFile( FILE* pFile )
{
	fread( &img_width, sizeof(int), 1, pFile );
	fread( &img_height, sizeof(int), 1, pFile );
	pPixels = 0;
	if ( img_width*img_height > 0 )
	{
		pPixels = new unsigned char[img_width*img_height*4];
		fread( pPixels, sizeof(unsigned char)*img_width*img_height*4, 1, pFile );
	}
}

void CBnsImage::WriteToFile( FILE* pFile )
{
	fwrite( &img_width, sizeof(int), 1, pFile );
	fwrite( &img_height, sizeof(int), 1, pFile );
	if ( img_width*img_height > 0 )
		fwrite( pPixels, sizeof(unsigned char)*img_width*img_height*4, 1, pFile );
}

void CMaterial::ReadFromFile( FILE* pFile, vector< CBnsImage* > &vecImages )
{
	fread( &ambient, sizeof(ambient), 1, pFile );
	fread( &diffuse, sizeof(diffuse), 1, pFile );
	fread( &emission, sizeof(emission), 1, pFile );
	fread( &specular, sizeof(specular), 1, pFile );
	fread( &shininess, sizeof(shininess), 1, pFile );

	int id = -1;
	fread( &id, sizeof(int), 1, pFile );
	
	if ( id >= 0)
		pImage = vecImages[id];
	else
		pImage = 0;
}

void CMaterial::WriteToFile( FILE* pFile, vector< CBnsImage* > &vecImages )
{
	fwrite( &ambient, sizeof(ambient), 1, pFile );
	fwrite( &diffuse, sizeof(diffuse), 1, pFile );
	fwrite( &emission, sizeof(emission), 1, pFile );
	fwrite( &specular, sizeof(specular), 1, pFile );
	fwrite( &shininess, sizeof(shininess), 1, pFile );
	
	int id = -1;
	for ( size_t i = 0 ; i < vecImages.size() ; i++ )
	{
		if ( pImage == vecImages[i] )
		{
			id = (int)i;
			break;
		}
	}

	fwrite( &id, sizeof(int), 1, pFile );
}

void Node::ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	fread( m_Trans, sizeof(nv_scalar)*16, 1, pFile );
	fread( &type, sizeof(NODE_TYPE), 1, pFile );
	fread( &m_aabb, sizeof(m_aabb), 1, pFile );

	int id = -1;
	fread( &id, sizeof(int), 1, pFile );
	geometry = 0;
	if ( id >= 0 && id < geopool.size() )
		geometry = geopool[id];
}

void Node::WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	fwrite( m_Trans, sizeof(nv_scalar)*16, 1, pFile );
	fwrite( &type, sizeof(NODE_TYPE), 1, pFile );
	fwrite( &m_aabb, sizeof(m_aabb), 1, pFile );

	int id = -1;
	for ( size_t g = 0 ; g < geopool.size() ; g++ )
	{
		if ( geopool[g] == geometry )
		{
			id = g;
			break;
		}
	}

	fwrite( &id, sizeof(int), 1, pFile );
}

void Door::ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	Node::ReadFromFile( pFile, geopool );
	fread( &m_bOpendDoor, sizeof(bool), 1, pFile );

	size_t cells;
	fread( &cells, sizeof(size_t), 1, pFile );
	for( size_t c = 0 ; c < cells ; c++ )
	{
		DoorCell* pCell = new DoorCell;
		pCell->ReadFromFile( pFile, geopool );
		pCells.push_back( pCell );
	}

	size_t mixes;
	fread( &mixes, sizeof(size_t), 1, pFile );
	for( size_t m = 0 ; m < mixes ; m++ )
	{
		Door* pMixedDoor = new Door;
		pMixedDoor->ReadFromFile( pFile, geopool );
		pMixDoors.push_back( pMixedDoor );
	}
}

void Door::WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	Node::WriteToFile( pFile, geopool );

	fwrite( &m_bOpendDoor, sizeof(bool), 1, pFile );

	size_t cells = pCells.size();
	fwrite( &cells, sizeof(size_t), 1, pFile );
	for( size_t c = 0 ; c < cells ; c++ )
		pCells[c]->WriteToFile( pFile, geopool );

	size_t mixes = pMixDoors.size();
	fwrite( &mixes, sizeof(size_t), 1, pFile );
	for( size_t m = 0 ; m < mixes ; m++ )
		pMixDoors[m]->WriteToFile( pFile, geopool );
}


void Room::ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	size_t walls;
	fread( &walls, sizeof(size_t), 1, pFile );
	m_vVertices = new nv_scalar[walls*3];
	fread( m_vVertices, sizeof(nv_scalar)*walls*3, 1, pFile );

	size_t nodes;
	fread( &nodes, sizeof(size_t), 1, pFile );
	for ( size_t n = 0 ; n < nodes ; n++ )
	{
		Node* pNode = new Node;
		pNode->ReadFromFile( pFile, geopool );
		m_pInsids.push_back( pNode );
	}
}

void Room::WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	size_t walls = m_pWalls.size();
	fwrite( &walls, sizeof(size_t), 1, pFile );
	fwrite( m_vVertices, sizeof(nv_scalar)*walls*3, 1, pFile );

	size_t nodes = m_pInsids.size();
	fwrite( &nodes, sizeof(size_t), 1, pFile );
	for ( size_t n = 0 ; n < nodes ; n++ )
		m_pInsids[n]->WriteToFile( pFile, geopool );
}

void Room::ReadFromFile( FILE* pFile, vector< Wall* > &wallpool )
{
	size_t walls;
	fread( &walls, sizeof(size_t), 1, pFile );
	for ( size_t w = 0 ; w < walls ; w++ )
	{
		size_t i;
		fread( &i, sizeof(size_t), 1, pFile );
		m_pWalls.push_back( wallpool[i] );
	}
}

void Room::WriteToFile( FILE* pFile, vector< Wall* > &wallpool )
{
	size_t walls = m_pWalls.size();
	fwrite( &walls, sizeof(size_t), 1, pFile );
	for ( size_t w = 0 ; w < walls ; w++ )
	{
		for ( size_t i = 0 ; i < wallpool.size() ; i++ )
		{
			if ( m_pWalls[w] == wallpool[i] )
			{
				fwrite( &i, sizeof(size_t), 1, pFile );
				break;
			}
		}
	}
}

void Wall::Scale( float s )
{
	Node::Scale(s);
	
	m_sp[0] *= s;
	m_sp[1] *= s;
	m_sp[2] *= s;

	m_ep[0] *= s;
	m_ep[1] *= s;
	m_ep[2] *= s;

	for (size_t i=0;i<m_pDoors.size();i++)
		m_pDoors[i]->Scale(s);
}

void Wall::ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	Node::ReadFromFile( pFile, geopool );

	fread( m_sp, sizeof(nv_scalar)*3, 1, pFile );
	fread( m_ep, sizeof(nv_scalar)*3, 1, pFile );

	size_t doors;
	fread( &doors, sizeof(size_t), 1, pFile );
	for ( size_t d = 0 ; d < doors ; d++ )
	{
		Door* pDoor = new Door;
		pDoor->ReadFromFile( pFile, geopool );
		m_pDoors.push_back( pDoor );
	}
}

void Wall::WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	Node::WriteToFile( pFile, geopool );
	
	fwrite( m_sp, sizeof(nv_scalar)*3, 1, pFile );
	fwrite( m_ep, sizeof(nv_scalar)*3, 1, pFile );

	size_t doors = m_pDoors.size();
	fwrite( &doors, sizeof(size_t), 1, pFile );
	for ( size_t d = 0 ; d < doors ; d++ )
		m_pDoors[d]->WriteToFile( pFile, geopool );
}

void Wall::ReadFromFile( FILE* pFile, vector< Room* >& rooms )
{
	size_t roomcount = 0;
	fread( &roomcount, sizeof(size_t), 1, pFile );
	for ( size_t r = 0 ; r < roomcount ; r++ )
	{
		size_t i;
		fread( &i, sizeof(size_t), 1, pFile );
		m_pRooms.push_back( rooms[i] );
	}
}

void Wall::WriteToFile( FILE* pFile, vector< Room* >& rooms )
{
	size_t roomcount = m_pRooms.size();
	fwrite( &roomcount, sizeof(size_t), 1, pFile );
	for ( size_t r = 0 ; r < roomcount ; r++ )
	{
		for ( size_t i = 0 ; i < rooms.size() ; i++ )
		{
			if ( m_pRooms[r] == rooms[i] )
			{
				fwrite( &i, sizeof(size_t), 1, pFile );
				break;
			}
		}
	}
}

void DoorCell::ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	Node::ReadFromFile( pFile, geopool );

	fread( &opentype, sizeof(OPEN_TYPE), 1, pFile );
	fread( &openingstate, sizeof(OPEN_STATE), 1, pFile );
	fread( &width, sizeof(int), 1, pFile );
	fread( &depth, sizeof(int), 1, pFile );
	fread( &height, sizeof(int), 1, pFile );
}

void DoorCell::WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool )
{
	Node::WriteToFile( pFile, geopool );

	fwrite( &opentype, sizeof(OPEN_TYPE), 1, pFile );
	fwrite( &openingstate, sizeof(OPEN_STATE), 1, pFile );
	fwrite( &width, sizeof(int), 1, pFile );
	fwrite( &depth, sizeof(int), 1, pFile );
	fwrite( &height, sizeof(int), 1, pFile );
}
