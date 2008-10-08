
#ifndef _QUATERNION_H_
#define _QUATERNION_H_

/************************************************************************************
*	Files: quaternion.h and quaternion.cpp
*
*	Most of the code here was copied from 
*		http://www.neutralzone.org/home/facsys/math.html
*	The code there was written by:
*		BLACKAXE / kolor a.k.a. Laurent Schmalen
*	Thank you very much for the free source code
*		I acutally retyped most of it to get an understanding of what was going
*		on but the source is pretty similar
*	I would also like to say thanks to the website
*		http://www.flipcode.com/documents/matrfaq.html
*	which helped provide a lot of insight into what was going on
*	
*	This code is being used by Steve Dutcher and Andy Gardner in our 
*	motion capture program for CS 838
*
*************************************************************************************/


class Quaternion {

private:
	float W;
	float X;
	float Y;
	float Z;

public:

	Quaternion();
	// Create quaternion from the values w, x, y, z
	Quaternion( float w, float x, float y, float z );
	// Create quaternion from euler angles X, Y, and Z
	Quaternion( float radX, float radY, float radZ );
	// Copy constructor
	Quaternion( const Quaternion & );
	~Quaternion();

	// These are overload so that we can just multiply quaternions together
	friend Quaternion operator* (const Quaternion&, const Quaternion&);
	const Quaternion& operator*= (const Quaternion&);

	void print();

	// Normalize a quaternion
	void normalize();
	
	// Compute quaternion from axis angle representation
	//		Example (45, 1, 0, 0) ==> gives you a 45 degree rotation about X-axis
	void fromAxisAngle( float, float, float, float );
	
	// Compute quaternion from euler angles
	void fromEulerAngles( float, float, float );

	// This return the axis and angle to rotate around, I don't really use this however,
	// I rather just get the Rotation matrix and multiply that on my gl stack
	void getAngleAndAxis( float &angle, float &x, float &y, float &z );
	
	// You MUST give me the memory for this Matrix[16] 
	// This will return a matrix ready for use with openGL
	void getRotationMatrix( float* );

	// This is the function that will linearly interpolate between two quaternion rotation
	// You call this with a quaternion and give it two quaternions to interpolate betwee
	// and also t which is a float between 0 and 1 which tells you where you are between
	// a and b
	void slerp(const Quaternion &a,const Quaternion &b, const float t);
};
	
#endif