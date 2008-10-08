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
*	NOTE: read quaternion.h for description of what each function does
*
*************************************************************************************/

#include <math.h>
#include <stdio.h>

#include "quaternion.h"

#ifndef PI
#define PI (3.1415926535)
#endif

#define FLT_EPSILON 0.005

Quaternion::Quaternion()
{
	W = 1;
	X = Y = Z = 0;
}

Quaternion::Quaternion( float w, float x, float y, float z )
{
	W = w;
	X = x;
	Y = y;
	Z = z;
}

Quaternion::Quaternion( float radX, float radY, float radZ)
{
	fromEulerAngles( radX, radY, radZ );
}

Quaternion::Quaternion( const Quaternion &q )
{
	W = q.W;
	X = q.X;
	Y = q.Y;
	Z = q.Z;
}

Quaternion::~Quaternion()
{
}


void Quaternion::slerp(const Quaternion &a,const Quaternion &b, const float t)
{
	float omega, cosom, sinom, sclp, sclq;

	// Calculate cosine
	cosom = a.X*b.X + a.Y*b.Y + a.Z*b.Z + a.W*b.W;

	if ((1.0f+cosom) > FLT_EPSILON){
		if ((1.0f-cosom) > FLT_EPSILON){
			// standard case SLERP
			omega = (float)acos(cosom);
			sinom = (float)sin(omega);
			sclp = (float)sin((1.0f-t)*omega) / sinom;
			sclq = (float)sin(t*omega) / sinom;
		}else{
			// a and b are very close so we can do a linear interpolation
			sclp = 1.0f - t;
			sclq = t;
		}
		X = sclp*a.X + sclq*b.X;
		Y = sclp*a.Y + sclq*b.Y;
		Z = sclp*a.Z + sclq*b.Z;
		W = sclp*a.W + sclq*b.W;
	}else{
		X =-b.Y;
		Y = b.X;
		Z =-b.W;
		W = b.Z;
		sclp = (float)sin((1.0f-t) * PI * 0.5);
		sclq = (float)sin(t * PI * 0.5);

		X = sclp*a.X + sclq*b.X;
		Y = sclp*a.Y + sclq*b.Y;
		Z = sclp*a.Z + sclq*b.Z;
		W = sclp*a.W + sclp*b.W;
	}
	//printf ("%f, %f, %f, %f\n", W, X, Y, Z) ;
}

void Quaternion::print()
{
	printf("Quaternion \tW=%f \tX=%f \tY=%f \tZ=%f\n",W,X,Y,Z);
}

Quaternion operator*(const Quaternion &a, const Quaternion &b)
{
	Quaternion q;

	q.W = a.W*b.W - (a.X*b.X + a.Y*b.Y + a.Z*b.Z);
	q.X = a.W*b.X + b.W*a.X + a.Y*b.Z - a.Z*b.Y;
	q.Y = a.W*b.Y + b.W*a.Y + a.Z*b.X - a.X*b.Z;
	q.Z = a.W*b.Z + b.W*a.Z + a.X*b.Y - a.Y*b.X;

	return q;
}

const Quaternion& Quaternion::operator*=(const Quaternion &q)
{
	float w = W*q.W - (X*q.X + Y*q.Y + Z*q.Z);

	float x = W*q.X + q.W*X + Y*q.Z - Z*q.Y;
	float y = W*q.Y + q.W*Y + Z*q.X - X*q.Z;
	float z = W*q.Z + q.W*Z + X*q.Y - Y*q.X;

	W = w;
	X = x;
	Y = y;
	Z = z;
	
	return *this;
}

void Quaternion::normalize()
{
	float distance = (float)sqrt(W*W + X*X + Y*Y + Z*Z);
	if( distance == 0 ){
		W = 1.0;
		X = Y = Z = 0.0;
	}else{
		W /= distance;
		X /= distance;
		Y /= distance;
		Z /= distance;
	}
}

void Quaternion::fromAxisAngle( float angle, float x, float y, float z )
{
	float sin_a = (float)sin( angle / 2 );
	float cos_a = (float)cos( angle / 2);

	W = cos_a;
	X = x * sin_a;
	Y = y * sin_a;
	Z = z * sin_a;

	normalize();
}

void Quaternion::fromEulerAngles( float radX, float radY, float radZ )
{

	Quaternion qx, qy, qz, qresult;
	
	qx.fromAxisAngle( radX, 1, 0, 0 );
	qy.fromAxisAngle( radY, 0, 1, 0 );
	qz.fromAxisAngle( radZ, 0, 0, 1 );

	qresult = qz * qx * qy;
	
	W = qresult.W;
	X = qresult.X;
	Y = qresult.Y;
	Z = qresult.Z;
}

void Quaternion::getAngleAndAxis( float &angle, float &x, float &y, float &z )
{
	angle = (float)(acos( W ) * 2 * 180 / PI);
	float sin_angle = (float)sqrt( 1.0 - W * W );

	if( fabs(sin_angle) < FLT_EPSILON )
		sin_angle = 1;

	x = X / sin_angle;
	y = Y / sin_angle;
	z = Z / sin_angle;
}

// Matrix you get looks like this
//      -----------
//     |0  4  8  12|
// M = |1  5  9  13|
//     |2  6  10 14|
//     |3  7  11 15|
//      ----------- 
void Quaternion::getRotationMatrix( float* M)
{	
	float xx, xy, xz, wx, yy, yz, wy, zz, wz;

	xx = X * X;
	xy = X * Y;
	xz = X * Z;
	wx = W * X;

	yy = Y * Y;
	yz = Y * Z;
	wy = W * Y;

	zz = Z * Z;
	wz = W * Z;

    M[0]  = 1 - 2 * ( yy + zz );
    M[1]  =     2 * ( xy + wz );
    M[2]  =     2 * ( xz - wy );

    M[4]  =     2 * ( xy - wz );
    M[5]  = 1 - 2 * ( xx + zz );
    M[6]  =     2 * ( yz + wx );

    M[8]  =     2 * ( xz + wy );
    M[9]  =     2 * ( yz - wx );
    M[10] = 1 - 2 * ( xx + yy );

	M[3] = M[7] = M[11] = M[12] = M[13] = M[14] = 0;
	M[15] = 1;
}


#ifdef _MAIN

#include <stdio.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdio.h>

// This is used as a tester app for Linux development
int main(int argc, char ** argv)
{
	#ifdef _LINUX
		// The only reason this is here is for compiling this main function
		// I need to initialize GL so that I can do a glPushMatrix()
		glutCreateWindow("");
	#endif

	float rx, ry, rz;
	while(1){
		printf("\n\n==============================================================\n");
		printf("Enter rotation X in degrees: ");
		scanf("%f", &rx);
		printf("Enter rotation Y in degrees: ");
		scanf("%f", &ry);
		printf("Enter rotation Z in degrees: ");
		scanf("%f", &rz);
		
		printf("\nComputing matrices for rotation (Z, X, Y) = (%f, %f, %f)\n", rz, rx, ry);

		Quaternion qt;
		qt.fromEulerAngles( rx*PI/180, ry*PI/180, rz*PI/180 );;
	
		float M[16];
		qt.getRotationMatrix( M );

		printf("\nMATRIX created from Quaternion\n");
		printf("%f\t%f\t%f\t%f\n", M[0], M[4], M[8], M[12]);
		printf("%f\t%f\t%f\t%f\n", M[1], M[5], M[9], M[13]);
		printf("%f\t%f\t%f\t%f\n", M[2], M[6], M[10], M[14]);
		printf("%f\t%f\t%f\t%f\n", M[3], M[7], M[11], M[14]);

		float GLMatrix[16];
		glPushMatrix();
		glLoadIdentity();
		glRotatef( rz, 0, 0, 1 );
		glRotatef( rx, 1, 0, 0 );
		glRotatef( ry, 0, 1, 0 );
		glGetFloatv( GL_MODELVIEW_MATRIX, GLMatrix );
		glPopMatrix();	

		printf("\nMATRIX created by doing glRotate and getting that matrix\n");
		printf("%f\t%f\t%f\t%f\n", GLMatrix[0], GLMatrix[4], GLMatrix[8], GLMatrix[12]);
		printf("%f\t%f\t%f\t%f\n", GLMatrix[1], GLMatrix[5], GLMatrix[9], GLMatrix[13]);
		printf("%f\t%f\t%f\t%f\n", GLMatrix[2], GLMatrix[6], GLMatrix[10], GLMatrix[14]);
		printf("%f\t%f\t%f\t%f\n", GLMatrix[3], GLMatrix[7], GLMatrix[11], GLMatrix[14]);
	}
}

#endif
