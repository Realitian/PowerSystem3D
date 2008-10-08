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
*
*	Read the motionData.h file for descriptions of each function if not given
*********************************************************************************************/

#ifdef _LINUX
   #include <stdlib.h>
   #include <GL/glut.h>
#else
   #include <mbstring.h>
#endif

#include "motionData.h"

#include <Windows.h>
#include <gl/GL.h>
//#include "fltkIncludes.h"

#include <string.h>
#include <stdio.h>

#include <math.h>

#ifndef PI
#define PI 3.1415926536
#endif

motionData::motionData()
{
	numOfRoots = 0;
	exists = 0;
	floorHeight = 0;
}

motionData::~motionData()
{
}

int motionData::getJointPosition( const char* jointName )
{
	return mappedJointNames[jointName];
}

float motionData::getFloorHeight()
{
	return floorHeight;
}

FILE *fptr ;
std::vector<char*> tempParentVector;


// The following function, through the use of a recursive helper function, reads the BioVision
// motion data format and initializes all data structures accordingly.
int motionData::readBVHfile( char* fn )
{

	if ((fptr  = fopen(fn, "rb")) == NULL){
		printf ("The file %s was not opened\n", fn) ;
		return 0;
	}else
		printf ("The file %s was opened\n", fn) ;

	//Reset the pointer to the beginning of the file...
	fseek (fptr, 0L, SEEK_SET) ;

	//While not at the end of file...
	while (!feof(fptr))
	{
		char buffer[2000] ;

		fscanf (fptr, "%s", buffer) ;

		//First look to see which header we're dealing with...
		if (strcmp (buffer, "HIERARCHY") == 0)	
		{
			readBVHjoint() ;
		}
		else if (strcmp (buffer, "MOTION") == 0)
		{
			float fBuffer;			
			fscanf (fptr, "%s", buffer) ;
			fscanf (fptr, "%f", &fBuffer) ;
			numberOfFrames = fBuffer;

			fscanf (fptr, "%s", buffer) ;
			fscanf (fptr, "%s", buffer) ;
			fscanf (fptr, "%f", &fBuffer) ;
			frameTime = fBuffer;

			//Now begins the actual data reading...

			fBuffer = 0.0f ;
			for(int m=0; m<numberOfFrames; m++)
			{
				for(size_t i=0; i<joints.size(); i++)
				{
					frameData fv;
					if( joints[i].numOfChannels() > 0 )
					{
						for(int k=0; k<joints[i].numOfChannels(); k++)
						{
							fscanf (fptr, "%f", &fBuffer) ;	//value 1
							fv.push_back( fBuffer );
						}
						joints[i].addFrame( fv );
					}
				}
			}
		}
	}

	size_t jointPointer = 0;
	for(size_t l=0; l<joints.size(); l++)
	{
		int n=0;
		char order[4];
		for(int i=0; i<joints[jointPointer].numOfChannels(); i++)
		{
#ifdef _LINUX
			if( strcasecmp(joints[jointPointer].getChannelName(i),"Xrotation")==0 || strcasecmp(joints[jointPointer].getChannelName(i),"rx")==0 ){
				order[n] = 'X';
				n++;
			}else if( strcasecmp(joints[jointPointer].getChannelName(i),"Yrotation")==0 || strcasecmp(joints[jointPointer].getChannelName(i),"ry")==0 ){
				order[n] = 'Y';
				n++;
			}else if( strcasecmp(joints[jointPointer].getChannelName(i),"Zrotation")==0 || strcasecmp(joints[jointPointer].getChannelName(i),"rz")==0 ){
				order[n] = 'Z';
				n++;
			}
#else
			if( _stricmp(joints[jointPointer].getChannelName(i),"Xrotation")==0 || _stricmp(joints[jointPointer].getChannelName(i),"rx")==0 ){
				order[n] = 'X';
				n++;
			}else if( _stricmp(joints[jointPointer].getChannelName(i),"Yrotation")==0 || _stricmp(joints[jointPointer].getChannelName(i),"ry")==0 ){
				order[n] = 'Y';
				n++;
			}else if( _stricmp(joints[jointPointer].getChannelName(i),"Zrotation")==0 || _stricmp(joints[jointPointer].getChannelName(i),"rz")==0 ){
				order[n] = 'Z';
				n++;
			}
#endif
		}
		order[3] = '\0';
		joints[jointPointer].setRotationOrder( order );
		jointPointer++;
	}

	// Coimpute Euler markers
	jointPointer = 0;
	glPushMatrix();
	for(size_t k=0; k<joints.size(); k++)
	{
		for(int fnum=1; fnum<=numberOfFrames; fnum++)
			computeEulerMarker( &joints[jointPointer], fnum, 1 );
		joints[jointPointer].copyEulerMarkersToOriginal();
		jointPointer++;
	}
	glPopMatrix();

	// Compute Quaternions
	jointPointer = 0;
	for(size_t m=0; m<joints.size(); m++)
	{
		if( joints[jointPointer].getType() == ROOT || joints[jointPointer].getType() == JOINT)
			joints[jointPointer].computeQuaternions();
		jointPointer++;
	}

	// Compute Quaternion markers
	glPushMatrix();
	jointPointer = 0;
	for(size_t n=0; n<joints.size(); n++)
	{
		for(int fnum=1; fnum<=numberOfFrames; fnum++)
			computeQuaternionMarker( &joints[jointPointer], fnum, 1 );
		jointPointer++;
	}
	glPopMatrix();

	fclose (fptr) ;

	// Compute floor height
	floorHeight = 100000000;
	jointPointer = 0;
	for(size_t p=0; p<joints.size(); p++)
	{
		float retVal = joints[jointPointer].smallestYvalueInMarkers();
		jointPointer++;	
		if( floorHeight > retVal )
			floorHeight = retVal;
	}			

	exists = 1;

	return 1;
}

void motionData::recomputeEulerMarkers()
{
	size_t jointPointer = 0;

	glPushMatrix();
	for(size_t k=0; k<joints.size(); k++)
	{
		joints[jointPointer].clearEulerMarkers();
		for(int fnum=1; fnum<=numberOfFrames; fnum++)
			computeEulerMarker( &joints[jointPointer], fnum, 1 );
		jointPointer++;
	}
	glPopMatrix();
}

void motionData::recomputeQuaternionMarkers()
{
	size_t jointPointer = 0;

	glPushMatrix();
	for(size_t n=0; n<joints.size(); n++)
	{
		joints[jointPointer].clearQuaternionMarkers();
		for(int fnum=1; fnum<=numberOfFrames; fnum++)
			computeQuaternionMarker( &joints[jointPointer], fnum, 1 );
		jointPointer++;
	}
	glPopMatrix();
}

// Old motion data this failed for some reason 
void motionData::readBVHjoint ()
{
	char  buffer[10000] ;
	float floatBuffer, floatBuffer2, floatBuffer3  ;
	int   intBuffer    ;

	//Thus begins the loop for nodes...
	fscanf (fptr, "%s", buffer) ;
	//printf ("The first printf : %s\n", buffer) ;

	//We're breaking recursion now...
	if (strcmp (buffer, "}") == 0)
	{	
		tempParentVector.pop_back();
		//printf ("RETURNING 1\n") ;
		return ;
	}

	joint j;

	//If we're at the end site...
#ifdef _LINUX
	if (strcasecmp (buffer, "End") == 0)	
#else
	if (_stricmp (buffer, "End") == 0)	
#endif
	{
		fscanf (fptr, "%s", buffer) ;
		//printf ("ONCE : %s\n", buffer) ;

		j.setType("EFFECTOR");
		j.setParent( tempParentVector.back() );

		// Create my name with the word Effector before the name of my parents name
		char* newBuff = new char[ 9 + (strlen(j.getParent()))];
		strcpy(newBuff, "Effector");
		strcat(newBuff, j.getParent() );
		j.setName(newBuff);
		tempParentVector.push_back( _strdup(newBuff) );
#ifdef _LINUX
		free( newBuff );
#else
		delete[] newBuff ;
#endif 

		// Call my parent and set his child to me
		int jointPosition = getJointPosition( j.getParent() );
		joints[ jointPosition ].addChild( j.getName() );

		fscanf (fptr, "%s", buffer) ;
		//printf ("TWICE : %s\n", buffer) ;
		fscanf (fptr, "%s", buffer) ;
		//printf ("THRICE : %s\n", buffer) ;
		fscanf (fptr, "%f", &floatBuffer) ;
		//printf ("4 : %f\n", floatBuffer) ;
		fscanf (fptr, "%f", &floatBuffer2) ;
		//printf ("4 : %f\n", floatBuffer2) ;
		fscanf (fptr, "%f", &floatBuffer3) ;
		//printf ("4 : %f\n", floatBuffer3) ;
		j.setOffset( floatBuffer, floatBuffer2, floatBuffer3 );

		if( j.getType() == ROOT )
			numOfRoots++;
		joints.push_back( j );
		mappedJointNames[ _strdup( j.getName() ) ] = joints.size() - 1;
		mappedEffectorNames[ _strdup(j.getName()) ] = joints.size() - 1;
	}
	else
	{
		if(strcmp("ROOT", buffer) != 0)
			j.setParent( tempParentVector.back() );

		j.setType( buffer );
	
		fscanf (fptr, "%s", buffer) ;
		j.setName( buffer );				//This one is the Joint Name
		//printf ("Joint name : %s\n", buffer) ;

		tempParentVector.push_back( _strdup(buffer) );

		if( j.getType() != ROOT )
		{
			// Call my parent and set his child to me
			int jointPosition = getJointPosition( j.getParent() );
			joints[ jointPosition ].addChild( j.getName() );
		}

		fscanf (fptr, "%s", buffer) ;		//This one is the bracket - Throw away...
		//printf ("Bracket : %s\n", buffer) ;

		fscanf (fptr, "%s", buffer) ;		//This one is OFFSET word

		fscanf (fptr, "%f", &floatBuffer) ;//This one is OFFSET x
		fscanf (fptr, "%f", &floatBuffer2) ;//This one is OFFSET y
		fscanf (fptr, "%f", &floatBuffer3) ;//This one is OFFSET z
		j.setOffset( floatBuffer, floatBuffer2, floatBuffer3 );

		fscanf (fptr, "%s", buffer) ;		//This one is CHANNEL word

		fscanf (fptr, "%d", &intBuffer) ; //This one is CHANNEL number

		for (int i = 0; i < intBuffer; i++)
		{
			fscanf (fptr, "%s", buffer) ;
			j.addChannelName( buffer );			//This is one of the CHANNELS

		}

		if( j.getType() == ROOT )
			numOfRoots++;
		joints.push_back( j );
		mappedJointNames[ _strdup( j.getName() ) ] = joints.size() - 1;
	}

	//j.dump() ;

	//Now we go back up to the top...
	readBVHjoint () ;

	fscanf (fptr, "%s", buffer) ;

	if (strcmp (buffer, "MOTION") != 0)
	{
		int len = -1 * (strlen(buffer)) ;
		fseek (fptr, len, SEEK_CUR)   ;
		readBVHjoint () ;
		//printf ("HERE IN OTHER REWIND %d %s\n", len, buffer) ;
	}
	else
	{
		int len = -1 * (strlen(buffer)) ;
		fseek (fptr, len, SEEK_CUR)   ;
		//printf ("HERE IN MOTION REWIND %d %s\n", len, buffer) ;
		return ;
	}
}

// The following function, through the use of a helper function, loads the Acclaim motion capture format
int motionData::readAcclaimFiles( char* fnASF, char* fnAMC ) 
{
	FILE *fptrASF ;
	FILE *fptrAMC ;

	//First argument, the ASF file, is opened
	if ((fptrASF  = fopen(fnASF, "r")) == NULL)
	{
		printf ("The file %s was not opened\n", fnASF) ;
		return 0;
	}
	else
		printf ("The file %s is being processed.\n", fnASF) ;

	//First argument, the AMC file, is opened
	if ((fptrAMC  = fopen(fnAMC, "r")) == NULL)
	{
		printf ("The file %s was not opened\n", fnAMC) ;
		return 0;
	}
	else
		printf ("The file %s is being processed.\n", fnAMC) ;

	//Reset the pointer to the beginning of the file...
	fseek (fptrASF, 0L, SEEK_SET) ;
	fseek (fptrAMC, 0L, SEEK_SET) ;

	//Buffers used for file reading...
	char buffer[10000]    ;

	//Definables used for skeleton construction...
	float globalScale = 0.0f ;
	bool  isDegrees   = true ;


	//Let us first dig through the ASF file and get all the goods...
	//While not at the end of file...
	while (!feof(fptrASF))
	{
		fscanf (fptrASF, "%[^:\0]", buffer) ;  //Get the whole chunka' data to work with...
		//printf ("NEW BUFFER\n") ;

		char * token ;
		token = strtok (buffer, " (),\t\n") ;

		// We have now discovered the version section of the asf file
		if (strcmp (token, "version") == 0)
		{
			token = strtok (NULL, " (),\t\n") ;
			if (strcmp (token, "1.1") == 0)
				printf ("Current file version (%s) is of questionable nature...", token) ;
		}
		// Here is the name section of the asf file
		else if (strcmp (token, "name") == 0)
		{
			token = strtok (NULL, " (),\t\n") ;
			printf ("Currently reading ASF skeleton : %s\n", token) ;
		}
		// Here is the units portion of the file
		else if (strcmp (token, "units") == 0)
		{
			token = strtok (NULL, " (),\t\n") ;

			while (token != NULL)
			{
				if (strcmp (token, "mass") == 0)
				{
					token = strtok (NULL, " (),\t\n") ;
					//printf ("The MASS is: %s\n", token) ;
				}
				else if (strcmp (token, "length") == 0)
				{
					token = strtok (NULL, " (),\t\n") ;
					globalScale = (float)atof(token)  ;
//					printf ("The LENGTH is: %f\n", globalScale) ;
				}
				else if (strcmp (token, "angle") == 0)
				{
					token = strtok (NULL, " (),\t\n") ;
					if (strcmp (token, "deg") == 0)
					{
						isDegrees = true ;
					}
					else
					{
						isDegrees = false ;
					}
//					printf ("The ANGLES are in degrees : %d\n", isDegrees) ;
				}
				
				token = strtok (NULL, " (),\t\n") ;
			}			
		}
		// It is safe to ignore everything in the documentation part.
		else if (strcmp (token, "documentation") == 0)
		{
			token = strtok (NULL, " (),\t\n") ;

			while (token != NULL)
			{
				token = strtok (NULL, " (),\t\n") ;
			}
		}
		// We have now entered the root portion of the asf file.
		else if (strcmp (token, "root") == 0)
		{
			int readFlag = 0    ;
			float tempOX, tempOY, tempOZ ; 
			joint j             ;

			//Init the necessary root var's
			numOfRoots++      ;
			j.setType("ROOT") ;													//set the type
			j.setName("root") ;													//set the name
			//No need to set parent for ROOT...

			token = strtok (NULL, " (),\t\n") ;

			while (token != NULL)
			{
				if (strcmp (token, "axis") == 0)
				{
					token = strtok (NULL, " (),\t\n") ;
//					printf ("The AXES ORDER is: %s\n", token) ;					
					j.setRotationOrder( token );
				}
				else if (strcmp (token, "order") == 0)
				{
					token = strtok (NULL, " (),\t\n") ;
					while (strlen (token) == 2)									
					{
//						printf ("Root channel names in order : %s\n", token) ;
						j.addChannelName(token) ;								//add the channel names
						token = strtok (NULL, " (),\t\n") ;
					}

					readFlag = 1 ;
				}
				else if (strcmp (token, "position") == 0)
				{
					token = strtok (NULL, " (),\t\n") ;
					for (int i = 0; i < 3; i++)
					{
						if (i == 0) tempOX = (float)atof(token) ;
						if (i == 1) tempOY = (float)atof(token) ;
						if (i == 2) tempOZ = (float)atof(token) ;						//HERE IS WHERE I TAKE
						token = strtok (NULL, " (),\t\n") ;						// ORDER INTO CONSID.
					}
					j.setOffset(tempOX, tempOY, tempOZ) ;						//CHANGE OFFSET FN VARS!

					readFlag = 1 ;
				}
				else if (strcmp (token, "orientation") == 0)
				{
					token = strtok (NULL, " (),\t\n") ;
					for (int i = 0; i < 3; i++)
					{
//						printf ("The ORIENTATION ORDER is: %s\n", token) ;
						token = strtok (NULL, " (),\t\n") ;	
					}
					readFlag = 1 ;
				}
				
				if (!readFlag)
					token = strtok (NULL, " (),\t\n") ;
				readFlag = 0 ;
			}

			joints.push_back(j) ;
			mappedJointNames[ _strdup(j.getName()) ] = joints.size() - 1 ;			//ADD THE JOINT HERE
		}
		// We have entered the bonedata section of the asf file
		else if (strcmp (token, "bonedata") == 0)
		{
			token = strtok (NULL, " (),\t\n") ;

			while (token != NULL)
			{
				int readFlag = 0 ;
				int dofNum   = 0 ;

				if (strcmp (token, "begin") == 0)
				{
					joint j ;

					float dirX, dirY, dirZ, len ;

					token = strtok (NULL, " (),\t\n") ;
					
					while (token != NULL)
					{
						if (strcmp (token, "id") == 0)
						{
							token = strtok (NULL, " (),\t\n") ;
//							printf ("ID is : %s\n", token) ;
						}
						else if (strcmp (token, "name") == 0)
						{
							token = strtok (NULL, " (),\t\n") ;					
//							printf ("NAME is : %s\n", token) ;
							j.setName(token) ;									//set the name...
							j.setType("JOINT") ;
						}
						else if (strcmp (token, "direction") == 0)
						{
							token = strtok (NULL, " (),\t\n") ;
							dirX = (float)atof(token) ;
//							printf ("dir X : %f\n", dirX) ;
							token = strtok (NULL, " (),\t\n") ;
							dirY = (float)atof(token) ;
//							printf ("dir Y : %f\n", dirY) ;
							token = strtok (NULL, " (),\t\n") ;
							dirZ = (float)atof(token) ;
//							printf ("dir Z : %f\n", dirZ) ;
						}
						else if (strcmp (token, "length") == 0)
						{
							token = strtok (NULL, " (),\t\n") ;
							len = (float)atof(token) ;
//							printf ("LENGTH is : %f\n", len)  ;					//Now that we've got
							dirX = dirX * len * globalScale ;					// the dirs and len's
							dirY = dirY * len * globalScale ;					// we will compose and
							dirZ = dirZ * len * globalScale ;					// add the offset vec.
							j.setOffset(dirX, dirY, dirZ) ;
						}
						else if (strcmp (token, "axis") == 0)
						{
							token = strtok (NULL, " (),\t\n") ;					
//							printf ("axs X : %s\n", token) ;					
							token = strtok (NULL, " (),\t\n") ;					
//							printf ("axs Y : %s\n", token) ;					
							token = strtok (NULL, " (),\t\n") ;
//							printf ("axs Z : %s\n", token) ;
							token = strtok (NULL, " (),\t\n") ;
//							printf ("axs order : %s\n", token);
							j.setRotationOrder( token );
						}
						else if (strcmp (token, "dof") == 0)
						{
							token = strtok (NULL, " (),\t\n") ;
							while (strlen (token) == 2)
							{
								j.addChannelName(token) ;
//								printf ("dofVal : %s\n", token) ;
								dofNum++ ;
								token = strtok (NULL, " (),\t\n") ;
							}
							readFlag = 1 ;

/*							token = strtok (NULL, " (),\t\n") ;
							j.addChannelName(token) ;
							printf ("axs X : %s\n", token) ;
							token = strtok (NULL, " (),\t\n") ;
							j.addChannelName(token) ;
							printf ("axs Y : %s\n", token) ;
							token = strtok (NULL, " (),\t\n") ;
							j.addChannelName(token) ;							//add the 3 channel
							printf ("axs Z : %s\n", token) ;					//  names...
*/
						}
						else if (strcmp (token, "limits") == 0)
						{
							for (int i = 0; i < dofNum; i++)
							{
								token = strtok (NULL, " (),\t\n") ;					//What on earth to
//								printf ("lim : %s\n", token) ;						//  do with limits?
								token = strtok (NULL, " (),\t\n") ;					//What on earth to
//								printf ("lim : %s\n", token) ;						//  do with limits?
							}
							readFlag = 1 ;
						}

						if (!readFlag)
							token = strtok (NULL, " (),\t\n") ;
						readFlag = 0 ;
						if (strcmp (token, "end") == 0)
							break ;
					}

					joints.push_back(j) ;
					mappedJointNames[ _strdup(j.getName()) ] = joints.size() - 1 ;			//ADD THE JOINT HERE
				}
				
				token = strtok (NULL, " (),\t\n") ;
			}
			delete[] token ;
		}

		// We have entered the hierarchy section of the asf file!
		else if (strcmp (token, "hierarchy") == 0)
		{
			char * hBuffer     ;
			char * hToken      ;
			char * hSubBuffer  ;
			char * hSubCopy    ;
			char * parent      ;
			int readFlag = 0   ;

			//I now have the whole chunk of data...
			//Time to copy it and dig with a new tokenizer...
			token = strtok (NULL, ":\0")   ;
			hBuffer = _strdup(token) ;

//			printf ("t0ken : %s\n", hBuffer) ;

//			printf ("xxx %s", token) ;

			hToken = strtok (hBuffer, "\n") ;

			while (hToken != NULL)
			{
				int parentFlag = 1 ;
				int readFlag   = 0 ;
				char * nodeName = new char[1000] ;

				hSubBuffer = _strdup (hToken)     ;
				hSubCopy   = _strdup (hSubBuffer) ;

				//printf ("%d\n",strlen(hSubBuffer)) ;
//				printf ("LINE: %s\n", hSubBuffer)  ;
				
				int len = strlen(hSubBuffer) ;

				for (int i = 0; i < len; i++)
				{
//					printf ("%d", i) ;

					if (hSubBuffer[i] == ' ' || hSubBuffer[i] == '\t'  
						|| hSubBuffer[i] == '(' || hSubBuffer[i] == ')' 
						|| hSubBuffer[i] == '\n' || hSubBuffer[i] == '\0')
					{
						if (readFlag != 0)
						{
							hSubCopy = hSubCopy + strlen (nodeName) ;
							readFlag = 0 ;
						}
						else
						{
							hSubCopy = hSubCopy + 1 ;
							readFlag = 0 ;
						}
					}
					else if (readFlag == 0)
					{
						sscanf (hSubCopy, "%s", nodeName) ;
						hSubCopy = hSubCopy + 1 ;
						if (parentFlag == 1 && strcmp(nodeName, "begin") != 0
							&& strcmp(nodeName, "end") != 0)
						{
							parent = _strdup(nodeName) ;
							parentFlag = 0 ;
//							printf ("The parent node is :%s\n", parent) ;
						}
						else if (strcmp(nodeName, "begin") != 0 && strcmp(nodeName, "end") != 0)
						{
							//SET UP CHILDREN OF PARENT HERE
							int jointPosition = getJointPosition( parent ) ;
							joints[ jointPosition ].addChild(nodeName)     ;

							jointPosition = getJointPosition( nodeName ) ;
							joints[ jointPosition ].setParent( parent )      ;

//							printf ("Child : %s\n", nodeName) ;
						}
						readFlag = 1 ;
					}
				}

				delete[] nodeName ;
				hToken = strtok (NULL, "\n")         ;
			}

//			printf ("DO I GET HERE?1\n") ;
			//delete[] hSubBuffer ;
			//delete[] hSubCopy ;
			//delete[] hBuffer ;
			//delete[] hToken ;
			//delete[] parent ;
#ifdef _LINUX
			free (hSubBuffer);
			free (hSubCopy) ;
			free (hBuffer) ;
			free (hToken) ;
			free (parent) ;
#else
			LocalFree (hSubBuffer) ;
			LocalFree (hSubCopy) ;
			LocalFree (hBuffer) ;
			LocalFree (hToken) ;
			LocalFree (parent) ;
#endif
		}

		else if (strcmp (token, "skin") == 0)
		{
			token = strtok (NULL, " (),\t\n") ;

			while (token != NULL)
			{
				token = strtok (NULL, " (),\t\n") ;
			}
		}


		while (token != NULL)
		{
			token = strtok (NULL, " \t\n") ;
		}

		fscanf (fptrASF, "%c", buffer)     ;  //Get rid of that \n at the end of the line...

		delete[] token ;
	}

	//Now it's time to dig through that .AMC file...
 
	while (!feof(fptrAMC))
	{
		//int looper = 0 ;
		//int jointPosition ;
		//frameData fd ;
		
		fscanf (fptrAMC, "%s", buffer) ;
//		printf ("%s\n", buffer) ;
		if (feof(fptrAMC))
		{}
		else if (buffer[0] == '#')
		{
			fscanf (fptrAMC, "%[^\n]", buffer) ;
			fscanf (fptrAMC, "%c", buffer) ;
//			printf ("Got COMMENT\n") ;
		}
		else if (buffer[0] == ':')
		{
//			printf ("Got Header\n") ;
			fscanf (fptrAMC, "%[^\n]", buffer) ;
			fscanf (fptrAMC, "%c", buffer) ;
		}
		else if (buffer[0] >= 48 && buffer[0] <= 57)
		{
			numberOfFrames = (float)atoi(buffer) ;
		}
		else
		{
//			printf ("Joint Name : %s\n", buffer) ;

			//Get the number of elements for this name...

			int jointPosition = getJointPosition(buffer) ;
			int looper = joints[ jointPosition ].numOfChannels() ;
		
			float rx, ry, rz, tx, ty, tz;
			
			if( joints[ jointPosition ].getType() == ROOT )
			{
//				frameData fd ;
				rx = ry = rz = tx = ty = tz = 0;

				for (int i = 0; i < looper; i++)
				{
					fscanf (fptrAMC, "%s", buffer)   ;
					char *chName = joints[ jointPosition ].getChannelName(i);
#ifdef _LINUX
					if( strcasecmp( chName, "rx" ) == 0 )
						rx = atof(buffer);
					else if( strcasecmp( chName, "ry" ) == 0 )
						ry = atof(buffer);
					else if( strcasecmp( chName, "rz" ) == 0 )
						rz = atof(buffer);
					else if( strcasecmp( chName, "rz" ) == 0 )
						rz = atof(buffer);
					else if( strcasecmp( chName, "tx" ) == 0 )
						tx = atof(buffer);
					else if( strcasecmp( chName, "ty" ) == 0 )
						ty = atof(buffer);
					else if( strcasecmp( chName, "tz" ) == 0 )
						tz = atof(buffer);
#else
					if( _stricmp( chName, "rx" ) == 0 )
						rx = (float)atof(buffer);
					else if( _stricmp( chName, "ry" ) == 0 )
						ry = (float)atof(buffer);
					else if( _stricmp( chName, "rz" ) == 0 )
						rz = (float)atof(buffer);
					else if( _stricmp( chName, "rz" ) == 0 )
						rz = (float)atof(buffer);
					else if( _stricmp( chName, "tx" ) == 0 )
						tx = (float)atof(buffer);
					else if( _stricmp( chName, "ty" ) == 0 )
						ty = (float)atof(buffer);
					else if( _stricmp( chName, "tz" ) == 0 )
						tz = (float)atof(buffer);
#endif
				} 
				joints[ jointPosition ].addFrame( tx, ty, tz, rx, ry, rz ) ;
			}else{
				rx = ry = rz = 0;

				for (int i = 0; i < looper; i++)
				{
					fscanf (fptrAMC, "%s", buffer)   ;
					char *chName = joints[ jointPosition ].getChannelName(i);
#ifdef _LINUX
					if( strcasecmp( chName, "rx" ) == 0 )
						rx = atof(buffer);
					else if( strcasecmp( chName, "ry" ) == 0 )
						ry = atof(buffer);
					else if( strcasecmp( chName, "rz" ) == 0 )
						rz = atof(buffer);
#else
					if( _stricmp( chName, "rx" ) == 0 )
						rx = (float)atof(buffer);
					else if( _stricmp( chName, "ry" ) == 0 )
						ry = (float)atof(buffer);
					else if( _stricmp( chName, "rz" ) == 0 )
						rz = (float)atof(buffer);
#endif
				} 
				joints[ jointPosition ].addFrame( rx, ry, rz ) ;
			}
		} 
	}

	size_t jointPointer = 0;

	// This adds and buffers the proper data in the joint objects.

	int rx, ry, rz;
	for(size_t n=0; n<joints.size(); n++)
	{
		if( joints[jointPointer].numOfChannels()<3 )
		{
			rx = ry = rz = 0;
			for(int i=0; i<joints[jointPointer].numOfChannels(); i++)
			{
				char *chName = joints[jointPointer].getChannelName(i);
#ifdef _LINUX
				if( strcasecmp( chName, "rx" ) == 0)
					rx = 1;
				else if( strcasecmp( chName, "ry" ) == 0 )
					ry = 1;
				else if( strcasecmp( chName, "rz" ) == 0 )
					rz = 1;
#else
				if( _stricmp( chName, "rx" ) == 0)
					rx = 1;
				else if( _stricmp( chName, "ry" ) == 0 )
					ry = 1;
				else if( _stricmp( chName, "rz" ) == 0 )
					rz = 1;
#endif
//				delete[] chName;
			}
			if( rx == 0 )
				joints[jointPointer].addChannelName( "rx" );
			if( ry == 0 )
				joints[jointPointer].addChannelName( "ry" );
			if( rz == 0 )
				joints[jointPointer].addChannelName( "rz" );
		}			
		jointPointer++;
	}

	jointPointer = 0;

	// Compute Euler Markers 
	glPushMatrix();
	for(size_t k=0; k<joints.size(); k++)
	{
		printf ("Computer effecotr for : %s\n", joints[jointPointer].getName()) ;
		for(int fnum=1; fnum<=numberOfFrames; fnum++)
		{
			computeEulerMarker( &joints[jointPointer], fnum, 1 );
			joints[jointPointer].copyEulerMarkersToOriginal();
		}
		jointPointer++;
	}
	glPopMatrix();

	// Compute Quaternions
	jointPointer = 0;
	for(size_t m=0; m<joints.size(); m++)
	{
		if( joints[jointPointer].getType() == ROOT || joints[jointPointer].getType() == JOINT )
			joints[jointPointer].computeQuaternions();
		jointPointer++;
	}

	// Compute Quaternion markers
	glPushMatrix();
	jointPointer = 0;
	for(size_t p=0; p<joints.size(); p++)
	{
		for(int fnum=1; fnum<=numberOfFrames; fnum++)
			computeQuaternionMarker( &joints[jointPointer], fnum, 1 );
		jointPointer++;
	}
	glPopMatrix();

	// Compute floor height
	floorHeight = 100000000;
	jointPointer = 0;
	for(size_t q=0; q<joints.size(); q++)
	{
		float retVal = joints[jointPointer].smallestYvalueInMarkers();
		jointPointer++;	
		if( floorHeight > retVal )
			floorHeight = retVal;
	}			

	exists = 1 ;

	fclose (fptrASF) ;
	fclose (fptrAMC) ;

	return 1;
}

// Though currently not working, the following function writes a renderman RIB file (see .h for parameter descriptions)
void motionData::writeRIBfile (const char * openFileName, int resX, int resY, int startFrame, int endFrame)
{
	//char* openFileName   ;

	FILE *ribStream ;

	//For testing purposes only...

	if ((ribStream  = fopen(openFileName, "w")) == NULL)
		printf ("The file %s was not opened\n", openFileName) ;
	else
		printf ("The file %s was opened\n", openFileName) ;


	//Used to buffer frame numbers...
	int bufferLength = 0        ;

	for (int j = 1 ; j < 1000000; j *= 10)
	{
		if (endFrame - startFrame >= (j))
			bufferLength++ ;
	}

	//Begin by writing the necessary comments on the top...
	fprintf (ribStream, "# Original file created by Steven Dutcher and Andrew Gardner\n") ;
	//Filename goes here...
	fprintf (ribStream, "# Output filename %s\n", openFileName) ;
	//Resolution goes here...
	fprintf (ribStream, "# Output resolution %d x %d\n", resX, resY) ;
	
	//Here comes the camera stuff...
	fprintf (ribStream, "version 3.03\n") ;
	fprintf (ribStream, "Option \"searchpath\" \"shader\" [\".:../shaders:&\"]\n") ;
	fprintf (ribStream, "\n") ;

	char zeroBuffer [100] ;
	char newBuff [100] ;

	printf ("Writing RIB file...\n") ;
		
	float tx, ty, tz, tx2, ty2, tz2;  // These are used for getting the translate for each joint

	for (int i = startFrame; i <= endFrame; i++)
	{
		fprintf (ribStream, "FrameBegin %d\n", (i-1)) ;

		//Assemble the proper line to feed to the ribStream - works for 0 buffering...
		zeroBuffer[0] = '\0' ;
		strcat (zeroBuffer, "  Display \"%s.%0") ;
		newBuff[0] = '\0' ;

#ifdef _LINUX
		sprintf (newBuff, "%s", 10) ;
#else
		_itoa( bufferLength, newBuff, 10) ;
#endif

		strcat (zeroBuffer, newBuff) ;
		strcat (zeroBuffer, "d.tif\" \"tiff\" \"rgba\"\n") ;

		//fprintf (ribStream, "  Display \"%s.%03d\" \"file\" \"rgba\"\n", openFileName, i) ;
		fprintf (ribStream, zeroBuffer, openFileName, i) ;
		fprintf (ribStream, "  Format %d %d 1\n", resX, resY) ;
		fprintf (ribStream, "  Projection \"perspective\" \"fov\" 40\n") ;		
		//fprintf (ribStream, "  Translate 0 0 %f\n", camTranslate ) ;
		//fprintf (ribStream, "  Rotate %f 1 0 0\n", camRotateX) ;
		//fprintf (ribStream, "  Rotate %f 0 1 0\n", camRotateY) ;
		fprintf (ribStream, "  ConcatTransform [ ") ;
		for (int xx = 0; xx < 16; xx++)
		{
			fprintf (ribStream, "%f ", rMatrix[xx]) ;
		}

		fprintf (ribStream, "]\n") ;

		fprintf (ribStream, "  WorldBegin\n") ;

		// Set up the lights.

		fprintf (ribStream, "    LightSource \"distantlight\" 1  \"intensity\" 0.75  \"from\"  [ 1 1 1 ]  \"to\"  [ 0 0 0 ]  \"lightcolor\"  [ 1 1 1 ]\n") ; 
		fprintf (ribStream, "    LightSource \"ambientlight\" 2  \"intensity\" 0.3\n") ;


		vectorOfJoints::iterator j2 = joints.begin();
		joint *j;
		for(int m=0; m < getNumberOfJoints(); m++)
		{
			if( j2->getType() != ROOT )
			{
				fprintf (ribStream, "    TransformBegin\n") ;
				//fprintf (ribStream, "    Identity\n") ;
				fprintf (ribStream, "      Color 1 0 0\n")          ; //Hardcode color for now...
				fprintf (ribStream, "      Surface \"matte\"\n") ;

				j2->getEulerMarkerPosition( i, tx2, ty2, tz2 );
				char *parentName = j2->getParent();
				j = getJoint( parentName );
  				j->getEulerMarkerPosition( i, tx, ty, tz );
//				delete[] parentName;

				fprintf (ribStream, "      Translate %f %f %f\n", tx, ty, tz);

				float rotY = (float)(atan((tz2-tz)/(tx2-tx))*180/PI);
				if( (tx2-tx)<0 )
					rotY = -180.0f - rotY;
				else if( (tx2-tx)>0 )
					rotY *= -1;
				else if( (tz2-tz) > 0 )
					rotY = -90;
				else
					rotY = 90;
				
				float distanceXZ = (float)sqrt ( (tx2-tx)*(tx2-tx) + (tz2-tz)*(tz2-tz) );
				float rotZ = (float)(atan( (ty2-ty) / distanceXZ )*180/PI);
				if( distanceXZ == 0 )
					rotZ = 90;

				fprintf (ribStream, "      Rotate %f 0 1 0\n", rotY)      ;
				fprintf (ribStream, "      Rotate %f 0 0 1\n", rotZ)      ;
				fprintf (ribStream, "      Scale 1 0.35 0.35\n")      ;
				
				float distance = (float)sqrt ( (tx2-tx)*(tx2-tx) + (ty2-ty)*(ty2-ty) + (tz2-tz)*(tz2-tz) );
				fprintf (ribStream, "      Translate %f 0 0\n", distance/2.0f );
				fprintf (ribStream, "      Sphere %f 0 1 360\n", distance/2.0f )   ; //Again, explicit for now...
				fprintf (ribStream, "    TransformEnd\n") ;
			}	
			j2++;
		}
		fprintf (ribStream, "  WorldEnd\n") ;
		fprintf (ribStream, "FrameEnd\n") ;
		fprintf (ribStream, "\n\n") ;
	}

	//delete[] zeroBuffer ;
	//delete[] newBuff    ;

	printf ("Done!\n") ;

	fclose (ribStream) ;
}


// This function writes a marker file according to a marker input file...
void motionData::writeMarkerFile(const char* openFileName)
{

	FILE *markerStream ;

	if ((markerStream  = fopen(openFileName, "w")) == NULL)
		printf ("The file %s was not opened\n", openFileName) ;
	else
		printf ("The file %s was opened\n", openFileName) ;


	printf("Writing Marker File...\n") ;

	//Start out with the two requisite tags: KSample and KTime
	fprintf (markerStream, "KSample,") ;
	fprintf (markerStream, "KTime,")   ;

	//Dig through the joint names and print their X, Y, and Z -names- only...
	float timeIncrementer = frameTime  ;
	float timeKeeper      = 0          ;

	vectorOfJoints::iterator jointPtr  ;
	jointPtr = joints.begin() ;	

	for (size_t i=0; i<joints.size(); i++)
	{
		fprintf(markerStream, "%s-X,",jointPtr->getName());
		fprintf(markerStream, "%s-Y,",jointPtr->getName());
		//Check for EndOfLine Printing...
		if (i + 1 == mappedJointNames.size())
			fprintf(markerStream, "%s-Z\n",jointPtr->getName());
		else
			fprintf(markerStream, "%s-Z,",jointPtr->getName());

		jointPtr++ ;
	}

	//Frame Numbering starts at ONE!
	for (int j = 1; j <= getNumberOfFrames(); j++)
	{
		//Print the frame number first...
		fprintf(markerStream, "%d,", j) ;
		fprintf(markerStream, "%f,", timeKeeper) ;
		timeKeeper += timeIncrementer   ;

		jointPtr = joints.begin() ;
		for (size_t k=0; k<joints.size(); k++)
		{
			//Gather pertient information...
			float mx, my, mz ;

			jointPtr->getOriginalMarkerPosition(j, mx, my, mz) ;

			//Check for EndOfLine Printing...
			if (k + 1 == mappedJointNames.size())
				fprintf (markerStream, "%f,%f,%f", mx, my, mz) ;
			else
				fprintf (markerStream, "%f,%f,%f,", mx, my, mz) ;

			jointPtr++ ;		
		}

		fprintf(markerStream, "\n") ;
	}

	printf ("Done!\n") ;

	fclose (markerStream) ;
}

// This function writes a MEL script, effectively exporting BVH and Acclaim files to Maya
void motionData::writeMelScript(const char * openFileName)
{
	FILE *melStream ;

	//For testing purposes only...
	//outputFileName = "finalRender" ;

	if ((melStream  = fopen(openFileName, "w")) == NULL)
		printf ("The file %s was not opened\n", openFileName) ;
	else
		printf ("The file %s was opened\n", openFileName) ;

	fprintf (melStream, "//Original file created by Steven Dutcher and Andrew Gardner\n") ;
	fprintf (melStream, "//This file will create a maya skeleton heirarchy and animation out of the data\n") ;
	fprintf (melStream, "//  last seen in the motion editor.\n") ;
	fprintf (melStream, "\n") ;

	fprintf (melStream, "//First we set the timerange to insure a proper fit.\n") ;
	fprintf (melStream, "playbackOptions -min 1 -max %d ;\n", (int)numberOfFrames)  ;  //This always exports all of the animation...
	fprintf (melStream, "\n") ;

	for(int i=0; i < getNumberOfRoots(); i++)
	{
		writeMelJoints( getRootName( i ), melStream, NULL );
	}

	printf ("Done writing MEL file...  Import away!\n") ;

	fclose (melStream) ;
}

// This is a helper function used by the write MEL function noted earlier.
// It recursively calls itself to account for every joint.
void motionData::writeMelJoints(char *name, FILE *melStream, char *oldName)
{
	joint *j = getJoint( name );

	printf ("%s\n", name) ;

	fprintf (melStream, "\n\n//Currently writing information for joint %s\n", name) ;
	fprintf (melStream, "\n") ;

	float ox, oy, oz, rx, ry, rz, tx, ty, tz ;
	j->getOffset(ox, oy, oz) ;
	fprintf (melStream, "joint -relative -name %s -position %f %f %f -rotationOrder yxz ;\n", name, ox, oy, oz) ;
	//fprintf (melStream, "sphere -name %s ;\n", name) ;

	for (int fnum=1; fnum <= numberOfFrames; fnum++)
	{
		fprintf (melStream, "currentTime %d ;\n", fnum) ;
		
		if (j->getType() == ROOT)
		{
			j->getTranslation(fnum, tx, ty, tz) ;
			fprintf (melStream, "setKeyframe -value %f -attribute \"translateX\" %s ;\n", tx, name) ;
			fprintf (melStream, "setKeyframe -value %f -attribute \"translateY\" %s ;\n", ty, name) ;
			fprintf (melStream, "setKeyframe -value %f -attribute \"translateZ\" %s ;\n", tz, name) ;
		}
		if (j->getType() != EFFECTOR)
		{
			j->getRotation(fnum, rx, ry, rz) ;
	
			fprintf (melStream, "setKeyframe -value %f -attribute \"rotateZ\" %s ;\n", rz, name) ;
			fprintf (melStream, "setKeyframe -value %f -attribute \"rotateX\" %s ;\n", rx, name) ;
			fprintf (melStream, "setKeyframe -value %f -attribute \"rotateY\" %s ;\n", ry, name) ;

			//Works with spheres...
			//j->getMarkerPosition(fnum, rx, ry, rz) ;
			//fprintf (melStream, "setKeyframe -value %f -attribute \"translateX\" %s ;\n", rx, name) ;
			//fprintf (melStream, "setKeyframe -value %f -attribute \"translateY\" %s ;\n", ry, name) ;
			//fprintf (melStream, "setKeyframe -value %f -attribute \"translateZ\" %s ;\n", rz, name) ;
		}
	}

	oldName = _strdup(name) ;

	for(int i=0; i < j->numOfChildren(); i++)
	{	
		if( j->getType() != EFFECTOR )
			writeMelJoints( j->getChild( i ), melStream, oldName );
	}

	fprintf (melStream, "pickWalk -d up;\n") ;
}

// This is a function that saves bvh files to the disk.
void motionData::writeBVHfile(char * openFileName)
{
	FILE *bvhStream ;

	//For testing purposes only...
	//outputFileName = "finalRender" ;

	if ((bvhStream  = fopen(openFileName, "w")) == NULL)
		printf ("The file %s was not opened\n", openFileName) ;
	else
		printf ("The file %s was opened\n", openFileName) ;

	fprintf (bvhStream, "HIERARCHY\n") ;

	for(int i=0; i < getNumberOfRoots(); i++)
	{
		writeBvhJoints( getRootName( i ), bvhStream, 0 );
	}

	fprintf (bvhStream, "MOTION\n") ;
	fprintf (bvhStream, "Frames: %d\n", (int)numberOfFrames) ;
	fprintf (bvhStream, "Frame Time: %f\n", frameTime)  ;

	for(int k=0; k < getNumberOfRoots(); k++)
	{
		writeBvhData( getRootName( k ), bvhStream);
	}

	printf ("Done writing BVH file!\n") ;

	fclose (bvhStream) ;
}

// This is a helper (recursive) function used by the write BVH file function.
void motionData::writeBvhJoints(char *name, FILE *bvhStream, int tabBuffer)
{
	float ox, oy, oz ;
	
	joint *j = getJoint( name );

	writeTabs (bvhStream, tabBuffer) ;
	if (j->getType() == ROOT)
	{
		fprintf (bvhStream, "ROOT ") ;
		fprintf (bvhStream, "%s\n", name) ;
	}
	else if (j->getType() == JOINT)
	{
		fprintf (bvhStream, "JOINT ") ;
		fprintf (bvhStream, "%s\n", name) ;
	}
	else if (j->getType() == EFFECTOR)
	{
		fprintf (bvhStream, "End Site\n") ;
	}

	writeTabs (bvhStream, tabBuffer) ;  fprintf (bvhStream, "{\n") ;
	j->getOffset(ox, oy, oz) ;
	writeTabs (bvhStream, tabBuffer) ;	fprintf (bvhStream, "\tOFFSET\t%f\t%f\t%f\n", ox, oy, oz) ;

	if (j->getType() != EFFECTOR)
	{
		writeTabs (bvhStream, tabBuffer) ;  fprintf (bvhStream, "\tCHANNELS %d ", j->numOfChannels()) ;		//NEED THIS FN...

		for (int k = 0; k < j->numOfChannels(); k++)
		{
			fprintf (bvhStream, "%s ", j->getChannelName(k)) ;
		}

		fprintf (bvhStream, "\n") ;
	}


	for(int i=0; i < j->numOfChildren(); i++)
	{	
		if( j->getType() != EFFECTOR )
			writeBvhJoints( j->getChild( i ), bvhStream, tabBuffer+1);
	}

	writeTabs (bvhStream, tabBuffer) ;	fprintf(bvhStream, "}\n") ;

}

// This is a second (recursive) function that writes the frame data to the bvh file.
void motionData::writeBvhData(char *name, FILE *bvhStream)
{
	float tx, ty, tz, rx, ry, rz ;
	
	for (int k=1; k <= numberOfFrames; k++)
	{
		joint *j = getJoint( name );
		
		if (j->getType() == ROOT)
		{
			//printf ("Root stuff\n") ;
			j->getTranslation(k, tx, ty, tz) ;
			j->getRotation(k, rx, ry, rz) ;

			fprintf (bvhStream, "%f %f %f ", tx, ty, tz) ;
			fprintf (bvhStream, "%f %f %f ", rz, rx, ry) ;
		}
		else if (j->getType() == JOINT)
		{
			//printf ("Joint stuff\n") ;
			j->getRotation(k, rx, ry, rz) ;

			fprintf (bvhStream, "%f %f %f ", rz, rx, ry) ;
		}

		for(int i=0; i < j->numOfChildren(); i++)
		{	
			if( j->getType() != EFFECTOR )
				writeBvhDataHelper( j->getChild( i ), bvhStream, k);
		}

		fprintf (bvhStream, "\n") ;
	}
}

// More help is needed to write BVH files, and this provides it.
void motionData::writeBvhDataHelper(char *name, FILE *bvhStream, int frameNum)
{
	float tx, ty, tz, rx, ry, rz ;
	
	joint *j = getJoint( name );
		
	if (j->getType() == ROOT)
	{
		//printf ("Root stuff\n") ;
		j->getTranslation(frameNum, tx, ty, tz) ;
		j->getRotation(frameNum, rx, ry, rz) ;

		fprintf (bvhStream, "%f %f %f ", tx, ty, tz) ;
		fprintf (bvhStream, "%f %f %f ", rz, rx, ry) ;
	}
	else if (j->getType() == JOINT)
	{
		//printf ("Joint stuff\n") ;
		j->getRotation(frameNum, rx, ry, rz) ;

		fprintf (bvhStream, "%f %f %f ", rz, rx, ry) ;
	}

	for(int i=0; i < j->numOfChildren(); i++)
	{	
		if( j->getType() != EFFECTOR )
			writeBvhDataHelper( j->getChild( i ), bvhStream, frameNum);
	}

}

// This is a function that takes any filestream and sticks the requested number of tabs
// in it current location.
void motionData::writeTabs(FILE *fileStream, int tabNum)
{
	for (int i = 0; i < tabNum; i++)
		fprintf (fileStream, "\t") ;
}

// This function gives you a joint from a name you should not delete
// the pointer you get to this joint
joint* motionData::getJoint( const char* jointName )
{
	int jointPosition = getJointPosition( jointName );

// 	vectorOfJoints::iterator _joint_pointer = joints.begin();
// 
// 	for(int i=0; i<jointPosition; i++)
// 		_joint_pointer++;
// 
// 	return _joint_pointer;
	if ( joints.empty() )
		return 0;
	return &joints[jointPosition];
}

int motionData::doesExist()
{
	return exists;
}

void motionData::clearMotionData()
{
	exists = 0;
	mappedJointNames.clear();
	joints.clear();
	numOfRoots = 0;
	numberOfFrames = 0;
	frameTime = 0;
}

int motionData::getNumberOfFrames()
{
	return (int)numberOfFrames;
}

int motionData::getNumberOfRoots()
{
	return numOfRoots;
}

// This function is used to get a root in case there are multiple roots
char* motionData::getRootName(int rootNumber)
{
	for(size_t i=0; i<joints.size(); i++)
	{
		if( joints[i].getType() == ROOT )
		{
			if( rootNumber > 0)
				rootNumber--;
			else if(rootNumber == 0)
				return joints[i].getName();
		}
	}
	return NULL;
}

/**********************************************************************************
*
*	These next two function computeQuaternionMarker and computeEulerMarker will
*	compute their respective markers for a certain frameNum by calculating the
*	marker position by using the glMatrix stack.  We did this for two reasons,
*	first by using the glMatrix stack it made it alot easier.  Second by using
*	the glMatrix stuff we make good use of the graphics hardware which gives
*	increased performance
*
**********************************************************************************/
void motionData::computeQuaternionMarker(joint *j, int frameNum, int initialCall)
{
	if(j->getType() == EFFECTOR){
		float ox, oy, oz;

		if (initialCall)
			glLoadIdentity();
		int parentPosition = getJointPosition( j->getParent() );
		size_t jointPointer = 0;
		for(int k=0; k<parentPosition; k++)
			jointPointer++;
		computeQuaternionMarker( &joints[jointPointer], frameNum );

		j->getOffset( ox, oy, oz );
		glTranslatef( ox, oy, oz );

		if (initialCall)
		{
			float M[16];
			glGetFloatv( GL_MODELVIEW_MATRIX, M );
			j->addQuaternionMarkerPosition( M[12], M[13], M[14] );
		}

	}else if(j->getType() == JOINT){
		float ox, oy, oz;

		if (initialCall)
			glLoadIdentity();
		int parentPosition = getJointPosition( j->getParent() );
		size_t jointPointer = 0;
		for(int k=0; k<parentPosition; k++)
			jointPointer++;
		computeQuaternionMarker( &joints[jointPointer], frameNum );

		j->getOffset( ox, oy, oz );
		glTranslatef( ox, oy, oz );

//		j->getRotation( frameNum, rx, ry, rz );
//		glRotatef( rz, 0, 0, 1 );
//		glRotatef( rx, 1, 0, 0 );
//		glRotatef( ry, 0, 1, 0 );
		float rotM[16];
		int retVal = j->getQuaternionMatrix( frameNum, rotM );
		if( retVal == 1)
			glMultMatrixf( rotM );

		if (initialCall)
		{
			float M[16];
			glGetFloatv( GL_MODELVIEW_MATRIX, M );
			j->addQuaternionMarkerPosition( M[12], M[13], M[14] );
		}

	}else if(j->getType() == ROOT){
		float tx, ty, tz, ox, oy, oz;

		if (initialCall)
			glLoadIdentity();
		j->getTranslation( frameNum, tx, ty, tz );
		glTranslatef( tx, ty, tz );

		j->getOffset( ox, oy, oz );
		glTranslatef( ox, oy, oz );

//		j->getRotation( frameNum, rx, ry, rz );
//		glRotatef( rz, 0, 0, 1 );
//		glRotatef( rx, 1, 0, 0 );
//		glRotatef( ry, 0, 1, 0 );
		float rotM[16];
		int retVal = j->getQuaternionMatrix( frameNum, rotM );
		if( retVal == 1 )
			glMultMatrixf( rotM );

		if (initialCall)
		{
			float M[16];
			glGetFloatv( GL_MODELVIEW_MATRIX, M );
			j->addQuaternionMarkerPosition( M[12], M[13], M[14] );
		}
	}
}
void motionData::computeEulerMarker(joint *j, int frameNum, int initialCall)
{
	if(j->getType() == EFFECTOR){
		float ox, oy, oz;

		if (initialCall)
			glLoadIdentity();
		int parentPosition = getJointPosition( j->getParent() );
		size_t jointPointer = 0;
		for(int k=0; k<parentPosition; k++)
			jointPointer++;
		computeEulerMarker( &joints[jointPointer], frameNum );

		j->getOffset( ox, oy, oz );
		glTranslatef( ox, oy, oz );

		if (initialCall)
		{
			float M[16];
			glGetFloatv( GL_MODELVIEW_MATRIX, M );
			j->addEulerMarkerPosition( M[12], M[13], M[14] );
		}

	}else if(j->getType() == JOINT){
		float rx, ry, rz, ox, oy, oz;

		if (initialCall)
			glLoadIdentity();
		int parentPosition = getJointPosition( j->getParent() );
		size_t jointPointer = 0;
		for(int k=0; k<parentPosition; k++)
			jointPointer++;
		computeEulerMarker( &joints[jointPointer], frameNum );

		j->getOffset( ox, oy, oz );
		glTranslatef( ox, oy, oz );

		// Check the return to make sure we got a valid number
		int retVal = j->getRotation( frameNum, rx, ry, rz );
		if( retVal == 1 )
		{
			char *rotOrder = j->getRotationOrder();
			if( strcmp(rotOrder, "ZXY") == 0 ){
				glRotatef( rz, 0, 0, 1 );
				glRotatef( rx, 1, 0, 0 );
				glRotatef( ry, 0, 1, 0 );
			}else if( strcmp(rotOrder, "XYZ") == 0 ){
				glRotatef( rx, 1, 0, 0 );
				glRotatef( ry, 0, 1, 0 );
				glRotatef( rz, 0, 0, 1 );
			}
		}

		if (initialCall)
		{
			float M[16];
			glGetFloatv( GL_MODELVIEW_MATRIX, M );
			j->addEulerMarkerPosition( M[12], M[13], M[14] );
		}

	}else if(j->getType() == ROOT){
		float rx, ry, rz, tx, ty, tz, ox, oy, oz;

		if (initialCall)
			glLoadIdentity();
		
		j->getTranslation( frameNum, tx, ty, tz );
		glTranslatef( tx, ty, tz );

		j->getOffset( ox, oy, oz );
		glTranslatef( ox, oy, oz );

		// Check the return to make sure we got a valid number
		int retVal = j->getRotation( frameNum, rx, ry, rz );
		if( retVal == 1 )
		{
			char *rotOrder = j->getRotationOrder();
			if( strcmp(rotOrder, "ZXY") == 0 ){
				glRotatef( rz, 0, 0, 1 );
				glRotatef( rx, 1, 0, 0 );
				glRotatef( ry, 0, 1, 0 );
			}else if( strcmp(rotOrder, "XYZ") == 0 ){
				glRotatef( rx, 1, 0, 0 );
				glRotatef( ry, 0, 1, 0 );
				glRotatef( rz, 0, 0, 1 );
			}
		}

		if (initialCall)
		{
			float M[16];
			glGetFloatv( GL_MODELVIEW_MATRIX, M );
			j->addEulerMarkerPosition( M[12], M[13], M[14] );
		}
	}
}

void motionData::resetJointPointer()
{
	jointPointer = 0;
}

joint* motionData::getCurrentJointPointer()
{
	return &joints[jointPointer];
}

void motionData::advanceJointPointer()
{
	jointPointer++;
}

int motionData::getNumberOfJoints()
{
	return joints.size();
}

void motionData::setCameraPosition(float* rT)
{
	for (int i = 0 ; i < 16; i++)
	{
		rMatrix[i] = rT[i] ;
	}
}

float motionData::getFrameTime()
{
	return frameTime;
}

/***************************************************************************************
* 
* These two functions writeMarkersToFile and writeMarkersToFileRecursion are used
* to compute a marker file from an input file
* Input file looks like:
*		name joint x y z
* The user should call the first function writeMarkersToFile with the input filename
* and the output filename and then this function will use the other function to write 
* out the markers
*
***************************************************************************************/
typedef std::vector<char*> markerNames;
typedef std::vector<float> intValues;

void motionData::writeMarkersToFile(const char* readFile, const char* writeFile)
{
	int i;
	markerNames mNames;
	intValues xValue, yValue, zValue;

	FILE * fptr = fopen( writeFile, "w" );
	fprintf(fptr, "KSample,KTime,");

	FILE * rFile = fopen( readFile, "rb" );
	char buffer[1000];
	
	while( !feof( rFile ) )
	{
		int n = fscanf( rFile, "%s", buffer );
		if( n <= 0)
			break;

		fprintf(fptr, "%s-X,%s-Y,%s-Z,",buffer,buffer,buffer);
		fscanf( rFile, "%s", buffer );
		mNames.push_back( _strdup( buffer ) );
	
		float x, y, z;
		fscanf( rFile, "%f", &x );
		xValue.push_back( x );
		fscanf( rFile, "%f", &y );
		yValue.push_back( y );
		fscanf( rFile, "%f", &z );
		zValue.push_back( z );
	}
	fprintf(fptr, "\n");

	glPushMatrix();
	for(i=1; i<=numberOfFrames; i++)
	{
		fprintf(fptr, "%d,%f,",i,i*frameTime);
		for(size_t k=0; k<mNames.size(); k++)
		{
			joint *j = getJoint( mNames[k] );
			writeMarkersToFileRecursion(j, i, fptr, xValue[k], yValue[k], zValue[k], 1 );
		}
		fprintf(fptr, "\n");
	}
	glPopMatrix();

	printf("Marker file %s successfully written\n", writeFile);
	fclose( rFile );
	fclose( fptr );
}

void motionData::writeMarkersToFileRecursion(joint *j, int frameNum, FILE* fptr, float offx, float offy, float offz, int initialCall)
{
	if(j->getType() == EFFECTOR){
		float ox, oy, oz;

		if (initialCall)
			glLoadIdentity();
		int parentPosition = getJointPosition( j->getParent() );
		size_t jointPointer = 0;
		for(int k=0; k<parentPosition; k++)
			jointPointer++;
		writeMarkersToFileRecursion( &joints[jointPointer], frameNum, fptr );

		j->getOffset( ox, oy, oz );
		glTranslatef( ox, oy, oz );
		
		if(initialCall)
			glTranslatef( offx, offy, offz );

		if (initialCall)
		{
			float M[16];
			glGetFloatv( GL_MODELVIEW_MATRIX, M );
			fprintf( fptr, "%f %f %f ", M[12], M[13], M[14] );
//			j->addEulerMarkerPosition( M[12], M[13], M[14] );
		}

	}else if(j->getType() == JOINT){
		float rx, ry, rz, ox, oy, oz;

		if (initialCall)
			glLoadIdentity();
		int parentPosition = getJointPosition( j->getParent() );
		size_t jointPointer = 0;
		for(int k=0; k<parentPosition; k++)
			jointPointer++;

		writeMarkersToFileRecursion( &joints[jointPointer], frameNum, fptr );

		j->getOffset( ox, oy, oz );
		glTranslatef( ox, oy, oz );

		// Check the return to make sure we got a valid number
		int retVal = j->getRotation( frameNum, rx, ry, rz );
		if( retVal == 1 )
		{
			char *rotOrder = j->getRotationOrder();

			if( strcmp(rotOrder, "ZXY") == 0 ){
				glRotatef( rz, 0, 0, 1 );
				glRotatef( rx, 1, 0, 0 );
				glRotatef( ry, 0, 1, 0 );
			}else if( strcmp(rotOrder, "XYZ") == 0 ){
				glRotatef( rx, 1, 0, 0 );
				glRotatef( ry, 0, 1, 0 );
				glRotatef( rz, 0, 0, 1 );
			}
		}

		if(initialCall)
			glTranslatef( offx, offy, offz );

		if (initialCall)
		{
			float M[16];
			glGetFloatv( GL_MODELVIEW_MATRIX, M );
			fprintf( fptr, "%f %f %f ", M[12], M[13], M[14] );
//			j->addEulerMarkerPosition( M[12], M[13], M[14] );
		}

	}else if(j->getType() == ROOT){
		float rx, ry, rz, tx, ty, tz, ox, oy, oz;

		if (initialCall)
			glLoadIdentity();
		
		j->getTranslation( frameNum, tx, ty, tz );
		glTranslatef( tx, ty, tz );

		j->getOffset( ox, oy, oz );
		glTranslatef( ox, oy, oz );
		
		if(initialCall)
			glTranslatef( offx, offy, offz );

		// Check the return to make sure we got a valid number
		int retVal = j->getRotation( frameNum, rx, ry, rz );
		if( retVal == 1 )
		{
			char *rotOrder = j->getRotationOrder();

			if( strcmp(rotOrder, "ZXY") == 0 ){
				glRotatef( rz, 0, 0, 1 );
				glRotatef( rx, 1, 0, 0 );
				glRotatef( ry, 0, 1, 0 );
			}else if( strcmp(rotOrder, "XYZ") == 0 ){
				glRotatef( rx, 1, 0, 0 );
				glRotatef( ry, 0, 1, 0 );
				glRotatef( rz, 0, 0, 1 );
			}
		}

		if (initialCall)
		{
			float M[16];
			glGetFloatv( GL_MODELVIEW_MATRIX, M );
			fprintf( fptr, "%f %f %f ", M[12], M[13], M[14] );
//			j->addEulerMarkerPosition( M[12], M[13], M[14] );
		}
	}
}

void motionData::dump()
{
	printf("==========================Dumping motion data=================================\n");
	for(size_t i=0; i<joints.size(); i++)
		joints[i].dump();
	printf("==========================Done dumping motion data============================\n");
}

void motionData::dumpMappedNames()
{
	printf("==========================Dumping mapped joint names===========================\n");
	mappedNames::iterator mapPointer = mappedJointNames.begin();
	printf("Map size = %d\n", mappedJointNames.size());
	for(size_t i=0; i<mappedJointNames.size(); i++)
	{
		printf("ID: %d\tName: %s\n", (*mapPointer).second, (*mapPointer).first);
		mapPointer++;
	}
	printf("==========================Done dumping mapped joint names======================\n");

	printf("==========================Dumping mapped effector names==========================\n");
	mappedNames::iterator mapEffectorPointer = mappedEffectorNames.begin();
	printf("Map size = %d\n", mappedEffectorNames.size());
	for(size_t k=0; k<mappedEffectorNames.size(); k++)
	{
		printf("ID: %d\tName: %s\n", (*mapEffectorPointer).second, (*mapEffectorPointer).first);
		mapEffectorPointer++;
	}
	printf("==========================Done dumping mapped effector names=====================\n");
}


#ifdef _MAIN

int main( int argc, char** argv )
{
	#ifdef _LINUX
		// The only reason this is here is for compiling this main function
		// I need to initialize GL so that I can do a glPushMatrix()
		glutCreateWindow("");
	#endif
	printf("Starting Motion Data Class Main Test Function\n");

	motionData md;
	char *f_name = new char[200];

	if( argc == 2 )
		md.readBVHfile( argv[1] );
	else
		while( 1 )
		{	
			printf( "Enter filenmae: " );
			scanf( "%s", f_name );
			printf("about to open file %s\n", f_name);
			md.readBVHfile( f_name );
//			md.dump();
			md.clearMotionData();
		}
			
	md.dump();

	md.writeRIBfile( "test.rib", 320, 240, 0, 50 );
	md.writeMarkerFile( "test.mak" );
}

#endif

