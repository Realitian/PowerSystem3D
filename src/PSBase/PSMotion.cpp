#ifdef SUPPORT_HTTP
#include <afxwin.h>
#include "Global.h"
#include "HttpClient.h"
#endif SUPPORT_HTTP
#include <stdio.h>
#include "PSMotion.h"
#include "PSMaterial.h"

PSJoint::PSJoint()
{
	memset( name, 0, NAME_LEN );
	rotAngle = 0;
	rot = 0;
}

PSJoint::~PSJoint()
{
	memset( name, 0, NAME_LEN );
	SAFE_DELETE_ARRAY( rotAngle );
	SAFE_DELETE_ARRAY(rot);
}

void PSJoint::ReadFromPSMFile( FILE* pFile, int nframeNum )
{
	fread( name, 1, NAME_LEN, pFile );

	rotAngle = new vec3[nframeNum];
	fread( rotAngle, 1, sizeof(vec3)*nframeNum, pFile );

	rot = new mat4[nframeNum];
	for ( int f = 0 ; f < nframeNum ; f++ )
	{
		mat4 rotx = mat4_id;
		mat4 roty = mat4_id;
		mat4 rotz = mat4_id;

		rotx.set_rot( -ps_to_rad*rotAngle[f].x, vec3_x );
		roty.set_rot( -ps_to_rad*rotAngle[f].y, vec3_y );
		rotz.set_rot( ps_to_rad*rotAngle[f].z, vec3_z );

		rot[f] = rotz * rotx * roty;
	}
}

void PSJoint::WriteToPSMFile( FILE* pFile, int nframeNum )
{
	fwrite( name, 1, NAME_LEN, pFile );
	fwrite( rotAngle, 1, sizeof(vec3)*nframeNum, pFile );
}

PSMotion::PSMotion()
{
	nFrameNum = 0;
	fFrameInterval = 0;
	nJointNum = 0;
	pJoints = 0;
}

PSMotion::~PSMotion()
{
	nFrameNum = 0;
	fFrameInterval = 0;
	nJointNum = 0;
	SAFE_DELETE_ARRAY( pJoints );
}

void PSMotion::ReadFromPSMFile( const char* filepath )
{
#ifdef SUPPORT_HTTP
	HttpDownLoad( filepath );
#endif SUPPORT_HTTP
	
	char uncompressedFile[MAX_PATH];
	strcpy( uncompressedFile, filepath );
	strcat( uncompressedFile, ".gz" );
	file_uncompress( filepath, uncompressedFile );

	FILE* pPSMFile = fopen( uncompressedFile, "rb" );

	if ( pPSMFile )
	{
		fread( &nFrameNum, 1, sizeof(int), pPSMFile );
		fread( &fFrameInterval, 1, sizeof(float), pPSMFile );
		fread( &nJointNum, 1, sizeof(int), pPSMFile );

		pJoints = new PSJoint[nJointNum];
		for ( int j = 0 ; j < nJointNum ; j++ )
			pJoints[j].ReadFromPSMFile( pPSMFile, nFrameNum );

		fclose( pPSMFile );
	}

	_unlink( uncompressedFile );
}

void PSMotion::WriteToPSMFile( char* filepath )
{
	FILE* pPSMFile = fopen( filepath, "w+b" );

	if ( pPSMFile )
	{
		fwrite( &nFrameNum, 1, sizeof(int), pPSMFile );
		fwrite( &fFrameInterval, 1, sizeof(float), pPSMFile );
		fwrite( &nJointNum, 1, sizeof(int), pPSMFile );
		
		for ( int j = 0 ; j < nJointNum ; j++ )
			pJoints[j].WriteToPSMFile( pPSMFile, nFrameNum );

		fclose( pPSMFile );
	}

	file_compress( filepath );
}