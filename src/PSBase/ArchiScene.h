#ifndef _ARCHISCENE_H__
#define _ARCHISCENE_H__

#include <vector>
using namespace std;

#ifndef SAFE_DELETE
#define SAFE_DELETE(X)  { if ((X)) { delete (X) ; (X) = 0 ; } }
#endif //SAFE_DELETE

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(X) { if( (X) ) { delete[] (X); (X) = 0; } }
#endif //SAFE_DELETE_ARRAY

class Building;
class OutSide;
class Floor;
class Room;
class Wall;
class Door;
class Sprite;
class Geometry;
class GeoGroup;
class CMaterial;
class Face;
class Node;

#ifndef nv_scalar
	typedef float nv_scalar;
#endif //nv_scalar

struct AABB
{
	nv_scalar m_Min[3];
	nv_scalar m_Max[3];

	void Scale( float s )
	{
		m_Min[0] *= s;
		m_Min[1] *= s;
		m_Min[2] *= s;

		m_Max[0] *= s;
		m_Max[1] *= s;
		m_Max[2] *= s;
	};
};

struct OBB
{
	nv_scalar m_Min[3];
	nv_scalar m_Max[3];
};

class Node
{
public:
	Node();
	AABB m_aabb;
	nv_scalar m_Trans[16];

	enum NODE_TYPE
	{
		STEPS,
		SLAB,
		GOOD,

		WALL,
		DOOR,
		SPRITE,
		ROOF,

		OTHER
	};
	NODE_TYPE type;

	GeoGroup* geometry;
	virtual void CalcAABB();
	virtual void Scale( float s )
	{
		m_aabb.Scale( s );
		m_Trans[12] *= s;
		m_Trans[13] *= s;
		m_Trans[14] *= s;
	}

public:
	virtual void ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool );
	virtual void WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool );
};

class OutSide
{
public:
	vector< Node* > m_pNodes;
	~OutSide();

public:
	void ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool );
	void WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool );
	void Scale( float s )
	{
		for (size_t i = 0 ; i < m_pNodes.size() ; i++)
			m_pNodes[i]->Scale( s );
	};
};

class Building
{
public:
	Building();
	~Building();

	Node* m_pOutline;
	vector< AABB > m_vecHallPoints;
	vector< Floor* > m_pFloors;

	unsigned int *m_glTex;

public:
	void ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool );
	void WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool );
	void Scale( float s );
};

class Geometry
{
public:
	Geometry();
	~Geometry();

	unsigned int m_glDlistId;
	bool m_bIsCulling;

	int m_nNumFace;

	nv_scalar *m_pVertices;
	nv_scalar *m_pNormals;
	nv_scalar *m_pTexCoords;

	short *m_pIndices;
	int m_nNumVertices;

	CMaterial* mat;
	unsigned char color[3];

public:
	void ReadFromFile( FILE* pFile, vector< CMaterial* >& matpool );
	void WriteToFile( FILE* pFile, vector< CMaterial* >& matpool );
	bool IsSame( Geometry *other );
	void Scale( float s );
};

class GeoGroup
{
public:
	GeoGroup();
	~GeoGroup();

	AABB m_aabb;
	void CalcAABB();

	int m_nNumGeo;
	Geometry* m_pGeos;

public:
	void ReadFromFile( FILE* pFile, vector< CMaterial* >& matpool );
	void WriteToFile( FILE* pFile, vector< CMaterial* >& matpool );
	bool IsSame( GeoGroup *other );
	void Scale( float s )
	{
		m_aabb.Scale( s );
		for (int i = 0 ; i < m_nNumGeo ; i++)
			m_pGeos[i].Scale( s );
	}
};

class Floor : public Node
{
public:
	Floor();
	~Floor();

	nv_scalar m_MinZ, m_MaxZ;
	vector< Wall* > m_pAllWalls;
	vector< Room* > m_pRooms;
	vector< Node* > m_pNoSlabs;
	vector< Node* > m_pSteps;

	vector< Room* > m_pStepRooms;

	Floor* m_pPre;
	Floor* m_pNext;

	vector< Wall* > m_pOutSideWalls;
	vector< Node* > m_pSlabs;

	virtual void CalcAABB();

public:
	void ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool );
	void WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool );
	virtual void Scale( float s );
};

class Room : public Node
{
public:
	~Room();

	nv_scalar* m_vVertices;
	vector< Wall* > m_pWalls;
	vector< Node* > m_pInsids;
	virtual void CalcAABB();

public:
	virtual void ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool );
	virtual void WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool );

	void ReadFromFile( FILE* pFile, vector< Wall* > &wallpool );
	void WriteToFile( FILE* pFile, vector< Wall* > &wallpool );
	virtual void Scale( float s )
	{
		Node::Scale(s);
		for (size_t i=0;i<m_pWalls.size();i++)
			m_vVertices[i] *= s;
	
		for (size_t i=0;i<m_pInsids.size();i++)
			m_pInsids[i]->Scale(s);
	}
};

class Wall : public Node
{
public:
	bool m_bRendered;
	~Wall();

	vector< Room* > m_pRooms;
	vector< Door* > m_pDoors;
	nv_scalar m_sp[3];
	nv_scalar m_ep[3];

public:
	void ReadFromFile( FILE* pFile, vector< Room* >& rooms );
	void WriteToFile( FILE* pFile, vector< Room* >& rooms );

	virtual void ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool );
	virtual void WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool );
	virtual void Scale( float s );
};

class DoorCell : public Node
{
public:
	DoorCell();
	enum OPEN_TYPE
	{
		OT_None,			
		OT_LeftFront,		
		OT_LeftBack,		
		OT_RightFront,		
		OT_RightBack,		
		OT_UpFront,			
		OT_UpBack,			
		OT_DownFront,		
		OT_DownBack,		
		OT_LeftSlide,		
		OT_RightSlide,		
		OT_UpSlide,			
		OT_DownSlide,		
		OT_HMidRotate,		
		OT_VMidRotate,		
		OT_Bifold_1,
		OT_Bifold_2,
		OT_Bifold_3,
		OT_Bifold_4,
		OT_BifoldBack_1,
		OT_BifoldBack_2,
		OT_BifoldBack_3,
		OT_BifoldBack_4,
		OT_Rotate,			
	};

	enum OPEN_STATE
	{
		DOOR_CLOSED=0,
		DOOR_OPENED,
		DOOR_OPENNIG,
		DOOR_CLOSING
	};

	OPEN_TYPE opentype;
	OPEN_STATE openingstate;
	int width, depth, height;

public:
	virtual void ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool );
	virtual void WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool );

	virtual void Scale( float s )
	{
		Node::Scale(s);
		
		width *= s;
		depth *= s;
		height *= s;
	}
};


class Door : public Node
{
public:
	Door() : m_bOpendDoor(false) { };
	~Door();
public:
	bool m_bOpendDoor;
	vector< DoorCell* > pCells;
	vector< Door* > pMixDoors;

public:
	virtual void ReadFromFile( FILE* pFile, vector< GeoGroup* > &geopool );
	virtual void WriteToFile( FILE* pFile, vector< GeoGroup* > &geopool );
	virtual void Scale( float s )
	{
		Node::Scale(s);
		
		for (size_t i=0 ; i<pMixDoors.size() ; i++)
			pMixDoors[i]->Scale(s);
		for (size_t i=0 ; i<pCells.size() ; i++)
			pCells[i]->Scale(s);
	}
};

class Sprite : public Node
{
public:
};

class CBnsImage
{
public:
	CBnsImage(){
		glTexId = 0;
	};
	~CBnsImage();
	
	int img_width;
	int img_height;
	unsigned char* pPixels;
	unsigned int glTexId;
public:
	void ReadFromFile( FILE* pFile );
	void WriteToFile( FILE* pFile );
};

class CMaterial
{
public:
	~CMaterial();

	float ambient[4];
	float diffuse[4];
	float emission[4];
	float specular[4];
	short shininess;

	CBnsImage* pImage;
public:
	void ReadFromFile( FILE* pFile, vector< CBnsImage* > &vecImages );
	void WriteToFile( FILE* pFile, vector< CBnsImage* > &vecImages );
};

class Light
{
public:
	nv_scalar ambient[4];
	nv_scalar diffuse[4];
	nv_scalar specular[4];

	nv_scalar attenuation0;
	nv_scalar attenuation1;
	nv_scalar attenuation2;
	int cutoff;
	nv_scalar iner;
	nv_scalar outer;

	int exponent;
	nv_scalar direction[4];
	nv_scalar position[4];

	enum LIGHT_TYPE{
		DIRECTIONAL = 0 ,
		POINT ,
		SPOT
	};

	LIGHT_TYPE type;

public:
	void ReadFromFile( FILE* pFile );
	void WriteToFile( FILE* pFile );
	void Scale( float s )
	{
		position[0] *= s;
		position[1] *= s;
		position[2] *= s;
	}
};

class ViewPos
{
public:
	ViewPos();

	nv_scalar pos[3];
	nv_scalar look[3];
	nv_scalar up[3];
	nv_scalar right[3];
	nv_scalar fovy;

public:
	void ReadFromFile( FILE* pFile );
	void WriteToFile( FILE* pFile );
	void Scale( float s )
	{
		pos[0] *= s;
		pos[1] *= s;
		pos[2] *= s;
	}
};

class Archi
{
public:
	Archi();
	~Archi();

	//property from file.
	float m_AmbientRGB[4];
	AABB m_Range;
	vector< Light > m_Lights;
	vector< ViewPos > m_ViewPoints;
	vector< GeoGroup* > m_pGeoList;
	vector< CMaterial* > m_pMaterials;
	vector< CBnsImage* > m_pImages;
	OutSide m_Outside;
	vector< Building* > m_pBuildings;

	//app property.
	Room* m_pCurrentRoom;
	Floor* m_pCurrentFloor;
	Building* m_pCurrentBuilding;
	Node* m_pPickedNode;
	nv_scalar m_FloorZ;

public:
	bool ReadFromFile( char* path );
	bool WriteToFile( char* path );
	void Scale( float s )
	{
		m_Range.Scale(s);

		for (size_t i=0; i<m_Lights.size(); i++)
			m_Lights[i].Scale(s);
		
		for (size_t i=0; i<m_ViewPoints.size(); i++)
			m_ViewPoints[i].Scale(s);

		for (size_t i=0; i<m_pGeoList.size(); i++)
			m_pGeoList[i]->Scale(s);

		m_Outside.Scale( s );

		for (size_t i=0; i<m_pBuildings.size(); i++)
			m_pBuildings[i]->Scale(s);

		m_FloorZ *= s;
	};
};

bool PtInPolygon( const nv_scalar inPoints[], const long inPtCount, const nv_scalar inTestPt[], const int iDim = 3);

#endif	//_ARCHISCENE_H__
