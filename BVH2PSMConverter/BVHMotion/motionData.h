#ifndef _MOTION_DATA_H
#define _MOTION_DATA_H

#pragma warning(disable: 4786)
#pragma warning(disable: 4788)

/********************************************************************************************
*	Files: motionData.h and motionData.cpp
*	Written by Steve Dutcher and Andy Gardner
*
*	Some key things to note:
*	There are two major structures in this class
*		1.) a vector of joints 
*		2.) a map of joint names
*	The map can be used to look up a the position of a joint in the joint vector
*   You do this by calling getJointPosition (this is a private function though)
*	
*	This class is the base of the animation window and graph editor.  The animation window
*	uses this class to for getting values of joints for drawing.  The graph editor then
*	uses this class to change values and interpolate values.
*
*	One thing to note, after the graph editor changes points it will call recompute on the
*   quaternions and euler data so that it can be recomputed.
*
*	This class also contains our data readers and writers:
*		1.)  Read BVH
*		2.)  Write BVH
*		3.)  Read Acclaim format
*		4.)	 Write Marker data
*		5.)	 Write MEL script
*		6.)	 Write RIB file
*********************************************************************************************/

#include <map>
#include <vector>
#include <stdio.h>

#include "joint.h"

// This is the function used for comparing names in the stl map
struct namesCompare
{
  bool operator()(const char* s1, const char* s2) const
  {
    return (strcmp(s1, s2) < 0);
  }
};

typedef std::vector<joint> vectorOfJoints;							// This structure will hold our motionData
typedef std::map<const char*, int, namesCompare> mappedNames;		// This structure gives us the position of the joint in the joints vector

class motionData{

private:
		
	mappedNames		mappedJointNames;		// This is a map of joint names
	mappedNames		mappedEffectorNames;	// This is a map of joint names that are effectors

	size_t jointPointer;	// A pointer into the joint class, the user of this class
											// can use this in conjunction with the 3 jointPointer functions

	float			frameTime;				// how long each frame lasts for
	float			numberOfFrames;			// number of frames in animation

	int				numOfRoots;				// This tells me how man roots there are, our version
											// supports motion data files with multiple roots

	int				exists;					// This value is used to see if motion data exists so
											// that animWindow can tell if there is anything it can draw

	// This function is a recursive function that is used by readBVHfile to read in the motion data
	void readBVHjoint();

	// Internal functions used to write the BVH and MEL data to a file.  Often recursive.
	void writeBvhJoints(char *name, FILE *bvhStream, int tabBuffer) ;	
	void writeBvhData  (char *name, FILE *bvhStream) ;	
	void writeBvhDataHelper(char *name, FILE *bvhStream, int frameNum) ;	
	void writeMelJoints(char *name, FILE *melStream, char *oldName);

	// Used to properly tab a BVH file.  Accepts a file to write to and the number of tabs
	// requested.
	void writeTabs(FILE *fileStream, int tabNum) ;

	// Gets the global position of a requested joint.
	int getJointPosition( const char* jointName );

	// How high the floor should be - gathered by finding the lowest Y-value in the motion.
	float floorHeight;

	float rMatrix[16] ;
public:
		
	motionData();
	~motionData();

	vectorOfJoints	joints;					// This is a vector of joints

	// Read and write BVH files.  Pass this function a filename to write.
	int readBVHfile( char* fn ) ;
	void writeBVHfile( char* fn) ;

	// Read ASF and AMC files -> pass an asf filename then an amc one.
	int readAcclaimFiles( char* fnASF, char* fnAMC ) ;
	
	// Compute the end effectors for all the joints.
	void computeEffectors();

	// Writes a RIB file using a private recursive function.  This takes arguments declared
	// by the window displayed in the main window.  
	// They are : The filename to write
	//            The filename to output as .tif images (from the rib file)
	//            The X resolution
	//            The Y resolution
	//            The starting frame of animation
	//            The ending frame of animation
	void writeRIBfile (const char*, int, int, int, int ) ;
	
	// This function takes a filename and writes marker data to that file.
	void writeMarkerFile (const char*) ;

	// Write out a mel script that can be used with maya for creating animations
	void writeMelScript (const char*) ;

	// Get a pointer to a joint given by jointName.  You should not delete the memory
	// for this joint I give you
	joint* getJoint( const char* jointName );

	// Is their motion data to be displayed
	int doesExist();

	// clear motion data so that a new file can be read in.  This will set motion data 
	// to does not exist
	void clearMotionData();

	// Returns the nubmer of frames
	int getNumberOfFrames();

	// Use these two functions together so that you get the number of roots and then loop on that count
	// from 0 to numOfRoots-1 and call getRootName with that number
	int getNumberOfRoots();
	char* getRootName( int );

	// Call this function to comute an euler or quaternion marker for each marker you should pass a 1
	// into the variable intial call so that the function knows what joint is being computed
	void computeEulerMarker(joint *j, int frameNum, int initialCall=0);
	void computeQuaternionMarker(joint *j, int frameNum, int initialCall=0);


	// These 4 functions can be used for going through the joints sequentially
	// Here is an example use:
	//		resetJointPointer()
	//		for(int i=0; i<getNumberOfJoints; i++){
	//			joint *j = getCurrentJointPointer();   // then do what you want with it
	//			advanceJointPointer();
	//		}
	void resetJointPointer();				// reset the joint pointer
	joint* getCurrentJointPointer();		// get the current pointer
	void advanceJointPointer();				// go to the next pointer
	int getNumberOfJoints();				// get the number of joints

	
	// Call these function to recompute the markers if the user has changed any of the angle via the graphEditor
	void recomputeEulerMarkers();
	void recomputeQuaternionMarkers();

	// set the camera position
	void setCameraPosition(float, float, float);

	// Returns the time that each frame should be shown
	float getFrameTime();

	// These are just two functions that I used for debuging by dumping out the joints and the mapped names
	void dump();
	void dumpMappedNames();

	// This is used for the RIB exporter and gathers a 4x4 transformation matrix for the camera.
	void setCameraPosition(float* rMatrix);

	// Write out the marker to a file, based on the input you get from the input file
	// Input file example:    joint name x y z
	void writeMarkersToFile(const char*, const char*);
	void writeMarkersToFileRecursion(joint* j,int frameNum,FILE* fptr,float offx=0,float offy=0,float offz=0, int initialCall=0);

	float getFloorHeight();
};

#endif
