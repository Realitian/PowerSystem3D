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
*
*	Read joint.h for descriptions of what each function does
****************************************************************************************/

#ifdef _LINUX
	#include <stdlib.h>
	#include <strings.h>
#endif

#include <stdio.h>
#include <string.h>
#include "joint.h"

//#include "fltkIncludes.h"

#ifndef PI
#define PI (3.1415926535f)
#endif

enum QUATERNION_VECTOR { XYZ, JUSTX, JUSTY, JUSTZ };

// Main constructor initializes necessary strings to ""
joint::joint()
{
	name = _strdup( "" );
	parent = _strdup( "" );
	rotationOrder = _strdup( "" );
}

// Destructor clears all necessary memory.
joint::~joint()
{
	if( name )
		free( name );
	if( parent )
		free( parent );
	if( rotationOrder )
		free( rotationOrder );
	children.clear();
	offset.clear();
	channelNames.clear();
	frames.clear();
	originalMarkers.clear();
	eulerMarkers.clear();
	quaternionMarkers.clear();
	quatV.clear();
}

// Copy constructor does everything necessary.
joint::joint(const joint& j)
{
	type = j.type;
	name = _strdup( j.name );
	parent = _strdup( j.parent );
	rotationOrder = _strdup( j.rotationOrder );
	children = j.children;
	offset = j.offset;
	channelNames = j.channelNames;
	frames = j.frames;
	originalMarkers = j.originalMarkers;
	eulerMarkers = j.eulerMarkers;
	quaternionMarkers = j.quaternionMarkers;
	quatV = j.quatV;
}

// This returns the smallest Y value the markers have - this is used to calculate where
// the ground plane should be located in the scene.
float joint::smallestYvalueInMarkers()
{
	float temp = 1000000000;
	for(size_t i=0; i<eulerMarkers.size(); i++)
	{
		markerPosition mp = eulerMarkers[i];
		if( temp > mp[1] )
			temp = mp[1];
	}
	return temp;
}

// Sets the current joint's rotation order.
void joint::setRotationOrder(char* order)
{
	if( rotationOrder )
		free( rotationOrder );
	rotationOrder = _strdup( order );
}

// Gets the current joint's rotation order.
char* joint::getRotationOrder()
{
	return rotationOrder;
}

// Sets the joint type (see enum structure in joint.h)
void joint::setType(const char* jointType)
{
	if(strcmp(jointType, "JOINT") == 0)
		type = JOINT;
	else if(strcmp(jointType, "EFFECTOR") == 0)
		type = EFFECTOR;
	else if(strcmp(jointType, "ROOT") == 0)
		type = ROOT;
	else
		type = NOTYPE;
}

// Simply returns what type the current joint is.
int joint::getType()
{
	return (int)type;
}

// Gets the current joint's name.
char* joint::getName()
{
	return name;
}

// Sets the current joint's name.
void joint::setName(const char* myName)
{
	name = _strdup( myName );
}

// Gets the current joint's only parent as a string (name).
char* joint::getParent()
{
	return parent;
}

// Sets the sole parent of the joint as a string (name).
void joint::setParent(const char* parentName)
{
	parent = _strdup( parentName );
}

// Returns how many children the joint has.
int joint::numOfChildren()
{	
	return children.size();
}

// Gets the requested child's name as a string.
// Often called after the programmer finds out how many children there are.
char* joint::getChild(int childNum)
{
	if( childNum < numOfChildren() )
		return children[ childNum ];
	return NULL;
}

// Adds another child to the list of children (as a string).
void joint::addChild(const char* childName)
{
	children.push_back( _strdup( childName ) );
}

// Sets the current joint's offset from its parent.
void joint::setOffset(float v1, float v2, float v3)
{
	offset.push_back(v1);
	offset.push_back(v2);
	offset.push_back(v3);
}

// Names one of the animation channels - this is kept in proper order according to
// when things are read in.
void joint::addChannelName(char* ch_name)
{
	channelNames.push_back( _strdup(ch_name) );
}

// Gets the requested channel name as a string.
char* joint::getChannelName(int index)
{
	return channelNames[ index ];
}

// Returns the number of channels the current joint has.
int joint::numOfChannels()
{
	return channelNames.size();
}

// Adds a frameData object to the current list of animation frames.
void joint::addFrame(frameData& frame_data)
{
	frames.push_back( frame_data );
}

// Adds explicit frame data.  This one is used for the ROOT joint.
void joint::addFrame(float tx, float ty, float tz, float rx, float ry, float rz)
{
	frameData fd;
	fd.push_back( tx );
	fd.push_back( ty );
	fd.push_back( tz );
	fd.push_back( rz );
	fd.push_back( rx );
	fd.push_back( ry );
	frames.push_back( fd );
}

// Adds explicit frame data.  This one is used for joints other than the ROOT.
void joint::addFrame(float rx, float ry, float rz)
{
	frameData fd;
	fd.push_back( rz );
	fd.push_back( rx );
	fd.push_back( ry );
	frames.push_back( fd );
}

// Gets the current joint's rotation for the requested frame by reference.
int joint::getRotation(int frameNum, float &rx, float &ry, float &rz)
{
	if( frameNum-1 >= (int)frames.size() )
	{
		rx = ry = rz = 0;
		return 0;
	}
	frameData fd = frames[ frameNum-1 ];
	if(fd.size() == 6){
		rx = fd[4];
		ry = fd[5];
		rz = fd[3];
	}else if(fd.size() == 3){
		rx = fd[1];
		ry = fd[2];
		rz = fd[0];	
	}else{
		printf("encountered an error, trying to read bad ROTATION frame data at frame %d\n", frameNum);
		return -1;
	}
	return 1;
}

// Sets the current joint's rotation for the assigned frame.
void joint::setRotation(int frameNum, float rx, float ry, float rz)
{
	frameVectors::iterator framesPointer = frames.begin();
	for(int i=1; i<frameNum; i++)
		framesPointer++;

	frameData fd = frames[ frameNum-1 ];
	if(fd.size() == 6){
		fd[4] = rx;
		fd[5] = ry;
		fd[3] = rz;
	}else if(fd.size() == 3){
		fd[1] = rx;
		fd[2] = ry;
		fd[0] = rz;	
	}else{
		printf("encountered an error, trying to write bad ROTATION frame data at frame %d\n", frameNum);
	}
	
	framesPointer = frames.erase( framesPointer );
	frames.insert( framesPointer, fd );
}

// Gets the requested frame's translation values
void joint::getTranslation(int frameNum, float &tx, float &ty, float &tz)
{
	frameData fd = frames[ frameNum-1 ];
	if(fd.size() == 6){
		tx = fd[0];
		ty = fd[1];
		tz = fd[2];
	}else{
		printf("encountered an error, trying to read bad TRANSLATION frame data at frame %d\n", frameNum);
	}
}

// Gets the joint's offset from its parent.
// Data is returned by reference floats.
void joint::getOffset(float &ox, float &oy, float &oz)
{
	ox = offset[0];
	oy = offset[1];
	oz = offset[2];
}

// Gets the joint's offset from its parent.
// Data is returned by an offset vector.
offsetVector joint::getOffset()
{
	return offset;
}

// This is used for the origional set of data and is executed once at startup.
void joint::copyEulerMarkersToOriginal()
{
	originalMarkers = eulerMarkers;
}

// This is used to retrieve the origional data's marker positions.
void joint::getOriginalMarkerPosition( int frameNum, float &mx, float &my, float &mz )
{
	markerPosition markerPos = originalMarkers[frameNum - 1];
	mx = markerPos[0];
	my = markerPos[1];
	mz = markerPos[2];
}

// This is used to add euler angle marker positions to a joint.
void joint::addEulerMarkerPosition( float mx, float my, float mz )
{
	markerPosition markerPos;
	markerPos.push_back(mx);
	markerPos.push_back(my);
	markerPos.push_back(mz);
	eulerMarkers.push_back( markerPos );
}

// This is used to gather euler angle marker positions from a joint.
void joint::getEulerMarkerPosition( int frameNum, float &mx, float &my, float &mz )
{
	markerPosition markerPos = eulerMarkers[frameNum - 1];
	mx = markerPos[0];
	my = markerPos[1];
	mz = markerPos[2];
}

// This is used to add quaternion angle marker positions to a joint.
void joint::addQuaternionMarkerPosition( float mx, float my, float mz )
{
	markerPosition markerPos;
	markerPos.push_back(mx);
	markerPos.push_back(my);
	markerPos.push_back(mz);
	quaternionMarkers.push_back( markerPos );
}

// This is used to add quaternion angle marker positions to a joint.
void joint::getQuaternionMarkerPosition( int frameNum, float &mx, float &my, float &mz )
{
	markerPosition markerPos = quaternionMarkers[frameNum - 1];
	mx = markerPos[0];
	my = markerPos[1];
	mz = markerPos[2];
}

// This function computes the Quaternions for the entire set of Euler angle rotations.
void joint::computeQuaternions()
{
	Quaternion q; 
	float rx, ry, rz;

	for(int i=1; i<=(int)frames.size(); i++)
	{
		getRotation( i, rx, ry, rz );

		rx = rx * PI / 180.0f;
		ry = ry * PI / 180.0f;
		rz = rz * PI / 180.0f;

		q.fromEulerAngles( rx, ry, rz );
		quatV.push_back( q );
	}
}

// This function returns a 4x4 matrix of quaternion rotations.
int joint::getQuaternionMatrix( int frameNum, float *M )
{
	if( frameNum-1 >= (int)frames.size() )
		return 0;
	quatV[ frameNum-1 ].getRotationMatrix( M );
	return 1;
}

// This function returns the angle and axis of a quaternion representation by reference.
void joint::getAngleAndAxis( int frameNum, float &angle, float &x, float &y, float &z )
{
	quatV[ frameNum-1 ].getAngleAndAxis( angle, x, y, z );
}

// This interpolates the rotateX values of the Euler data.
void joint::interpolateRX( int start, int stop )
{
	interpolate( start, stop, 1 );
}

// This interpolates the rotateY values of the Euler data.
void joint::interpolateRY( int start, int stop )
{
	interpolate( start, stop, 2 );
}

// This interpolates the rotateZ values of the Euler data.
void joint::interpolateRZ( int start, int stop )
{
	interpolate( start, stop, 0 );
}

// This is the workhorse that actually re-assigns newly-interpolated values to the frames between
// the two requested interpolation points.
void joint::interpolate( int start, int stop, int channelPos )
{
	// user thinks of frame 1 as start but I use frame 0 so 
	// subtract 1 from start and stop
	start--;
	stop--;
	int fdXpos;
	if( numOfChannels() == 6)
		fdXpos = 3 + channelPos;
	else if( numOfChannels() == 3 )
		fdXpos = channelPos;
	else{
		printf("Bad number of channels for joint %s in trying to interpolate X-rotation\n",getName());
		return;
	}
	
	float startX, stopX;
	frameData fdStart, fdStop;
	
	// If user gave me start = -1 then use first frame, but it is actually just copying stop frame to all the frames in between
	if( start == -1 )
		fdStart = frames[ 0 ];
	else	
		fdStart = frames[ start ];

	startX = fdStart[ fdXpos ];
	fdStop = frames[ stop ];
	stopX = fdStop[ fdXpos ];

	frameVectors::iterator frameIterator = frames.begin();
	for(int ii=0; ii<=start; ii++){
		frameIterator++;
	}

	for(int i=start+1; i<stop; i++){
		frameData fd = frames[i];
		if( start == -1 )
			fd[ fdXpos ] = stopX;
		else
			fd[ fdXpos ] = (stopX-startX) * ((i - start)/float(stop-start)) + startX;
		frames.insert( frameIterator, fd );
		frameIterator++;
		frameIterator = frames.erase( frameIterator );
	}
}	
	

// This is the workhorse that actually re-assigns newly-interpolated values to the frames between
// the two requested quaternion interpolation points.
void joint::interpolateQuaternion( int start, int stop )
{
	// user thinks of frame 1 as start but I use frame 0 so 
	// subtract 1 from start and stop
	start--;
	stop--;
	
	Quaternion to, from, result;

	// If user gave me start==0 - 1 then use first frame, but it is actually just copying stop frame to all the frames in between
	if( start != -1 )
			from = quatV[ start ];
	to = quatV[ stop ];
	
	quaternionVector::iterator quaternionIterator = quatV.begin();
	for(int ii=0; ii<=start; ii++){
		quaternionIterator++;
	}

	for(int i=start+1; i<stop; i++){
		if( start == -1 )
			result = to;
		else
			result.slerp( from, to, ((i - start)/float(stop-start)) );
		quatV.insert( quaternionIterator, result );
		quaternionIterator++;
		quaternionIterator = quatV.erase( quaternionIterator );
	}
}

// Clears all of the markers for quaternion data.
void joint::clearQuaternionMarkers()
{
	quaternionMarkers.clear();
}

// Clears all of the markers for euler data.
void joint::clearEulerMarkers()
{
	eulerMarkers.clear();
}

// A true/false value as to whether or not the joint contains motion data.
int joint::hasFrameData()
{
	if( frames.size() > 0 )
		return 1;
	return 0;
}

//Just dump all the pertient information for the joint at hand.
void joint::dump()
{
	printf("**************** Dumping Joint *****************\n");
	printf("Joint name = %s\n", name);
	printf("Joint type = %d\n", type);
	printf("Joint parent = %s\n", parent);
	if( (int)children.size() > 0 )
	{
		printf("Joint children = ");
		for(int m=0; m<(int)children.size(); m++)
			printf("%s ", children[m]);
		printf("\n");
	}
	printf("Rotation order = %s\n",rotationOrder);
	printf("OFFSET ");
	if(offset.size() > 0)
		printf("%f %f %f", offset[0], offset[1], offset[2]);
	printf("\nJoint channel names  ");
	for(int i=0; i<(int)channelNames.size(); i++)
		printf("%s ", channelNames[i]);
	printf("\n");
	printf("FRAMES %d\n", frames.size());
	for(int j=0; j<(int)frames.size(); j++){
		frameData fdata = frames[j];
		for(int k=0; k<(int)fdata.size(); k++)
			printf("%f ",fdata[k]);
		printf("\n");
	}
	printf("ORIGINAL MARKER data %d\n", originalMarkers.size());
	for(int a=0; a<(int)originalMarkers.size(); a++){
		markerPosition mp = originalMarkers[a];
		for(int b=0; b<(int)mp.size(); b++)
			printf("%f ",mp[b]);
		printf("\n");

	}
	printf("EULER MARKER data %d\n", eulerMarkers.size());
	for(int aa=0; aa<(int)eulerMarkers.size(); aa++){
		markerPosition mp = eulerMarkers[aa];
		for(int b=0; b<(int)mp.size(); b++)
			printf("%f ",mp[b]);
		printf("\n");

	}
	printf("QUATERNION MARKER data %d\n", quaternionMarkers.size());
	for(int aaa=0; aaa<(int)quaternionMarkers.size(); aaa++){
		markerPosition mp = quaternionMarkers[aaa];
		for(int b=0; b<(int)mp.size(); b++)
			printf("%f ",mp[b]);
		printf("\n");

	}
	printf("QUATERNION data %d\n", quatV.size());
	for(int c=0; c<(int)quatV.size(); c++){
		Quaternion qt = quatV[c];
		qt.print();
	}
}

// This is just a testing function that I used for development
#ifdef _MAIN

typedef std::vector<joint> jointVector;
typedef std::vector<joint>::iterator jointPointer;

int main(int argc, char ** argv)
{
	vector<joint> js;

	printf("Starting Joint Class Main Test Function\n");

	printf("Creting joint1\n");
	joint j1;
	j1.setName( "joint1" );
	j1.setType( "ROOT" );
	j1.setOffset( 5.0f, 2.0f, 3.0f );

	printf("Adding channel names\n");
	j1.addChannelName( "TranslationZ" );
	j1.addChannelName( "TranslationX" );
	j1.addChannelName( "TranslationY" );

//	js.push_back( j1 );
//printf("pushing joint1\n");
//	js.push_back( j1 );
//printf("pushing joint1\n");
//	js.push_back( j1 );

	printf("Making frame data\n");
	frameData fd;
	frameData fd2;
	fd.push_back( 3.0f );
	fd.push_back( 4.0f );
	fd.push_back( 2.0f );
	fd2.push_back( 8.0f );
	fd2.push_back( 12.0f );
	fd2.push_back( 6.0f );

	printf("Adding 2 duplicate frames\n");
	j1.addFrame( fd );
	j1.addFrame( fd );
	j1.addFrame( fd2 );

	fd.clear();
	fd.push_back( 9.0f );
	fd.push_back( 7.8f );
	fd.push_back( 4.6f );
	j1.addFrame( fd );
	j1.addFrame( fd2 );

	j1.dump();
	// Ok lets test interpolation on j1 now
	j1.interpolateRX( 1, 5);
	j1.dump();
/*
	printf("Creting joint2\n");
	joint j2;
	j2.setName( "joint2" );
	j2.setType( "JOINT" );
	float ox, oy, oz;
	j1.getOffset( ox, oy, oz );
	j2.setOffset( ox, oy, oz );

	j2.addChannelName( "TranslationZ" );
	j2.addChannelName( "TranslationX" );
	j2.addChannelName( "TranslationY" );
	
	printf("Creting joint3\n");
	joint *j3 = new joint;

	printf("Setting joint 3's name\n");
	char *newBuff = new char [ 8 + strlen(j2.getName()) ];
	strcpy( newBuff, "Effector" );
	strcat( newBuff, j2.getName() );

	j3->setName( newBuff );
	free( newBuff );
	j3->setOffset( 5.0f, 2.0f, 3.0f );
	j3->setParent( "Myparent" );
	j3->setType( "EFFECTOR" );

	printf("Adding children\n");
	j2.addChild( j3->getName() );
	j1.addChild( j2.getName() );
	
	printf( "Should Output( Effeectorjoint2 ) = %s\n", j2.getName() );
	
	printf("pushing joint1 on stack\n");
	js.push_back( j1 );
	printf("pushing joint2 on stack\n");
	js.push_back( j2 );
	printf("pushing joint3 on stack\n");
	js.push_back( *j3 );

	printf("about to delet pointer to joint3\n");
	delete j3;

	jointPointer jp = js.begin();
	for(int i=0; i<js.size(); i++)
	{
		jp->dump();
		jp++;
	}
	
	printf("Clearing vector of joints\n");
	js.clear();
	printf("Done clearing jointVector\n");
*/
 	return 1;
}

#endif


