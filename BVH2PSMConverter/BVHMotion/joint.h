#ifndef _JOINT_H_
#define _JOINT_H_

#pragma warning(disable: 4786)
#pragma warning(disable: 4788)

/****************************************************************************************
*	Joint.h and Joint.cpp
*	Written by Steve Dutcher and Andy Gardner
*
*	Each joint holds information like:
*			name
*			parent's name
*			all its children's name
*			rotation order
*			lenght of joint (this is the offset)
*			channel names (rotateX, rotateY, rotateZ, translateX, ......)
*			frame data for those channels (these are the euler angles)
*			quaternion data for each channel
*			marker data at every frame for original, euler and quaternions
*
*	Any time you are required to give the values frameNum to a function in this class
*   you will think of 1 as the first frame and you can go to <= total number of frames
****************************************************************************************/

#include <map>
#include <vector>
#include <deque>

//#include "quat.h"
#include "quaternion.h"

// This holds the jointType constant, to use:
// 	joint1->getType() == ROOT
enum JOINT_TYPE { NOTYPE=-1, ROOT, JOINT, EFFECTOR};

// This holds the names of a joint's children
typedef std::vector<char*> childVector;

// This holds the joints OFFSET vector(3 pts)
typedef std::deque<float> offsetVector;

// This hold the joints CHANNELS usually 3 sometimes 6, 
// but can be any number
typedef std::vector<char*> channelVector;

// This hold a frame so probably 3 pts, name are give in CHANNELS
typedef std::vector<float> frameData;

// This holds the above structure(frameData), so basically 
// it has every frame number.  NOTE: Frame1 = frameVectors[0];
typedef std::vector<frameData>			frameVectors;

// This holds a marker positioin (3 float vector)
typedef std::vector<float> markerPosition;

// This holds a markerPosition for every frame.  
// NOTE: marker at frame 1 = markerVector[0];
typedef std::vector<markerPosition> markersVector;

// This hold a quaternioin
typedef std::vector<Quaternion> quaternionVector;

class joint{

private:
	int					type;				// Types are ROOT, JOINT, EFFECTOR, and NOTYPE
	char*				name;
	char*				parent;
	char*				rotationOrder;		// Rotation order is a string something like "XYZ"
	childVector			children;			// STL vector of char* to children names
	offsetVector		offset;				// STL vector of 3 floats representing the length of the joint
	channelVector		channelNames;		// STL vector holding the channel names for the joint
	frameVectors		frames;				// All the frame data

	markersVector		originalMarkers;	
	markersVector		eulerMarkers;
	markersVector		quaternionMarkers;

	quaternionVector	quatV;		
	
	void interpolate(int,int,int);			// This is a private function it gets used by interpolateRX, RY and RZ for eulers

public:
	joint();
	~joint();
	joint(const joint& j);

	// Sets and gets what type of joint the latest addition is.
	void		setType(const char*);
	int			getType();

	// Sets and gets what the current joint is named.
	void		setName(const char*);
	char*		getName();

	// Sets and gets the joint's only parent.
	void		setParent(const char*);	
	char*		getParent();	


	// Here is how you add a child onto a parents list:
	// 	j->addChild( "some_name" );
	// Here is how you get the children:
	//	for(int i=0; i< j->numOfChildren(); i++)
	//		j->getChild( i );
	// 
	int		numOfChildren();
	char*	getChild(int);
	void	addChild(const char*);

	// Sets and gets the motion data offset.
	// Make sure you pass in references for getOffset
	void	setOffset(float v1, float v2, float v3);
	void	getOffset(float &ox, float &oy, float &oz);
	offsetVector 	getOffset();


	// Add a channel name to the list and get a channel name from the list
	// The first name is 0 so if you want all the channels loop 
	// from i=0 to i<numOfChannels()
	void	addChannelName(char*);
	char*	getChannelName(int);
	int		numOfChannels();

	
	// After you compute marker positions for quaternions and eulers add their values
	// into the marker positions
	void	addEulerMarkerPosition(float mx, float my, float mz);
	void	addQuaternionMarkerPosition(float mx, float my, float mz);

	
	// If you want to rescompute quaternion markers call this function to clear the 
	// markers and then call add___MarkerPosition again for the quaternions and eulers
	void	clearQuaternionMarkers();
	void	clearEulerMarkers();

	
	// After you are finished computing your euler marker positions, call this function
	// to copy the euler markers so that they can be saved as the original
	void	copyEulerMarkersToOriginal();

	
	// These three functions are used to get marker positions for any joint at a certain
	// frame number.
	void	getOriginalMarkerPosition(int frameNum, float &mx, float &my, float &mz);
	void	getEulerMarkerPosition(int frameNum, float &mx, float &my, float &mz);
	void	getQuaternionMarkerPosition(int frameNum, float &mx, float &my, float &mz);

	
	// Call this function to compute the quaternions from the euler angles
	void 	computeQuaternions();
	
	// Then use this function getQuaternionMatrix to get the rotation matrix
	// You must give me the memory for this matrix of size 4x4
	int 	getQuaternionMatrix( int frameNum, float *M );
	void	getAngleAndAxis( int frameNum, float &angle, float &x, float &y, float &z );

	// Get and set rotation values for the joints
	int		getRotation(int frameNum, float &rx, float &ry, float &rz);
	void	setRotation(int frameNum, float rx, float ry, float rz);
	
	// Get teh translation values for the joints
	void	getTranslation(int frameNum, float &tx, float &ty, float &tz);


	// These two function will be used by the file parser.
	//		File parser asks - How many channels are there?
	//		It will then create a vector of type frameData and pass back into here in the setFrameData function for a frame
	void	addFrame(frameData&);
	void	addFrame(float tx, float ty, float tz, float fx, float fy, float fz);
	void	addFrame(float fx, float fy, float fz);


	// Interpolate euler angles
	void	interpolateRX(int, int);
	void	interpolateRY(int, int);
	void	interpolateRZ(int, int);
	

	// Interpolate quaternion angles
	void	interpolateQuaternion(int, int);
//	void	interpolateQuaternionRX(int, int);
//	void	interpolateQuaternionRY(int, int);
//	void	interpolateQuaternionRZ(int, int);

	// This function will print out all the motion data for this joint
	void	dump();

	// Does this joint have frameData (1 or 0)
	int		hasFrameData();

	// Set rotation order and get a character string saying what the rotation order is
	// String of size 3 normally Ex. "XYZ"
	void	setRotationOrder(char*);
	char*	getRotationOrder();

	float 	smallestYvalueInMarkers();
};

#endif
