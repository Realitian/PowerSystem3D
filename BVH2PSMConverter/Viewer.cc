#include <Windows.h>
#include "gl\glut.h"
#include "PSSkining.h"
#include "motionData.h"

#define CONV_BVH_TO_PSM

//#define BVH_FILENAME "bvh\\Walk.bvh"
//#define BVH_FILENAME "bvh\\Run.bvh"
//#define BVH_FILENAME "bvh\\Stand.bvh"
#define BVH_FILENAME "bvh\\Knockout.bvh"

//#define PSM_FILENAME "psm\\Walk.psm"
//#define PSM_FILENAME "psm\\Run.psm"
//#define PSM_FILENAME "psm\\Stand.psm"
#define PSM_FILENAME "psm\\Knockout.psm"

#define PSH_FILENAME "psh\\male.psh"

SkinNode* g_pSkinRoot = 0;
PSMotion g_PSMotion;
motionData g_Motion;

//
// Handle of the window we're rendering to
//
static GLint window;

//
// Movement variables
//
float fXDiff = 0;

float fTranDiff = 0.1;
float fXTran = 0;
float fYTran = 0;
float fZTran = -5;

int xLastIncr = 0;
int yLastIncr = 0;
float fXInertia = -0.5;
float fYInertia = 0;
float fXInertiaOld;
float fYInertiaOld;
float fScale = 1.0;
float ftime = 0;
int xLast = -1;
int yLast = -1;
char bmModifiers;
int Rotate = 1;
static int nFrameNum = 1;

//
// Rotation defines
//
#define INERTIA_THRESHOLD       1.0f
#define INERTIA_FACTOR          0.5f
#define SCALE_FACTOR            0.01f
#define SCALE_INCREMENT         0.5f
#define TIMER_FREQUENCY_MILLIS  20

GLfloat RotL = 1 * 3.14f / 180;
int LastTime = 0;

/***************************************************************************/
/* Parse GL_VERSION and return the major and minor numbers in the supplied
 * integers.
 * If it fails for any reason, major and minor will be set to 0.
 * Assumes a valid OpenGL context.
*/

void getGlVersion( int *major, int *minor )
{
    const char* verstr = (const char*)glGetString( GL_VERSION );
    if( (verstr == NULL) || (sscanf( verstr, "%d.%d", major, minor ) != 2) )
    {
        *major = *minor = 0;
        fprintf( stderr, "Invalid GL_VERSION format!!!\n" );
    }
}

#define ROTTO 0.3

int CurrentFrameLimit = 0;
int CurrentFrameDiff = 0;
void display(void)
{
    glLoadIdentity();
    glTranslatef(fXTran, fYTran, fZTran);

	glRotatef(fXDiff, 0,1,0);
 	glScalef( fScale, fScale, fScale);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	static float ambient[4] = { 1, 1, 1, 1};
	glLightModelfv ( GL_LIGHT_MODEL_AMBIENT, ambient );

 	static float eTime = 0 ;
	int nFrameNum = eTime / g_PSMotion.fFrameInterval + CurrentFrameDiff;

	nFrameNum %= CurrentFrameLimit;
 	
	eTime += 0.02;

	mat4 mat4_identity = mat4_id;
// #ifdef CONV_BVH_TO_PSM
// 	nFrameNum ++;
// #endif
	AdvanceMotionMatrix( true, 0, nFrameNum, g_pSkinRoot, 0, mat4_identity );

	UpdateBoneMatrix( g_pSkinRoot );
	DrawNode( 0, g_pSkinRoot );

    glFlush();
    glutSwapBuffers();
}


static
void play(void)
{
    glutPostRedisplay();
}


static
void key(unsigned char keyPressed, int x, int y)
{
    switch(keyPressed)
    {
		case 'a':
			fXTran -= fTranDiff;
			break;
		case 'd':
			fXTran += fTranDiff;
			break;
		case 's':
			fZTran -= fTranDiff;
			break;
		case 'w':
			fZTran += fTranDiff;
			break;
		case 'r':
			fYTran -= fTranDiff;
			break;
		case 'f':
			fYTran += fTranDiff;
			break;

		case '*':
			fTranDiff *= 2;
			break;
		case '/':
			fTranDiff /= 2.0f;
			break;
        case 'q':
        case 27:
            exit(0);
            break;
        case ' ':
			nFrameNum++;
			if ( nFrameNum >= g_Motion.getNumberOfFrames() )
				nFrameNum = 1;
            break;
		case '<':
			CurrentFrameDiff++;
			printf( "CurrentFrameDiff: %d\n", CurrentFrameDiff );
			break;
		case '>':
			CurrentFrameDiff --;
			printf( "CurrentFrameDiff: %d\n", CurrentFrameDiff );
			break;
        case '+':
            CurrentFrameLimit ++;
			printf( "CurrentFrameLimit: %d\n", CurrentFrameLimit );
            break;
        case '-':
            CurrentFrameLimit --;
			printf( "CurrentFrameLimit: %d\n", CurrentFrameLimit );
			break;
        default:
            fprintf(stderr, "\nKeyboard commands:\n\n"
            "b - Toggle among background clear colors\n"
            "q, <esc> - Quit\n"
            "t - Toggle among models to render\n"
            "? - Help\n"
            "<home>     - reset zoom and rotation\n"
            "<space> or <click>        - stop rotation\n"
            "<+>, <-> or <ctrl + drag> - zoom model\n"
            "<arrow keys> or <drag>    - rotate model\n\n");
            break;
    }
}


static
void timer(int value)
{
    /* Callback */
    glutTimerFunc(TIMER_FREQUENCY_MILLIS , timer, 0);
}


static
void mouse(int button, int state, int x, int y)
{
   bmModifiers = glutGetModifiers();

   if (button == GLUT_LEFT_BUTTON)
   {
      if (state == GLUT_UP)
      {
         xLast = -1;
         yLast = -1;

         if (xLastIncr > INERTIA_THRESHOLD)
            fXInertia = (xLastIncr - INERTIA_THRESHOLD)*INERTIA_FACTOR;

         if (-xLastIncr > INERTIA_THRESHOLD) 
            fXInertia = (xLastIncr + INERTIA_THRESHOLD)*INERTIA_FACTOR;

         if (yLastIncr > INERTIA_THRESHOLD) 
            fYInertia = (yLastIncr - INERTIA_THRESHOLD)*INERTIA_FACTOR;

         if (-yLastIncr > INERTIA_THRESHOLD) 
            fYInertia = (yLastIncr + INERTIA_THRESHOLD)*INERTIA_FACTOR;
      }
      else
      {
         fXInertia = 0;
         fYInertia = 0;
      }
      xLastIncr = 0;
      yLastIncr = 0;
   }
}


static
void motion(int x, int y)
{
   if ((xLast != -1) || (yLast != -1))
   {
      xLastIncr = x - xLast;
      yLastIncr = y - yLast;
      if (bmModifiers & GLUT_ACTIVE_CTRL)
      {
         if (xLast != -1)
         {
            fScale += (yLastIncr)*SCALE_FACTOR;
         }
      }
      else
      {
         if (xLast != -1)
         {
            fXDiff += xLastIncr;
         }
      }
   }

   xLast = x;
   yLast = y;
}


static
void reshape(int wid, int ht)
{
    float vp = 0.8f;
    float aspect = (float) wid / (float) ht;

    glViewport(0, 0, wid, ht);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, wid, ht);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustum(-vp, vp, -vp / aspect, vp / aspect, 1, 1000000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -5.0);
}

void special(int key, int x, int y)
{
    switch (key)
    {
         case GLUT_KEY_HOME:
//             fXDiff    = 0;
//             fYDiff    = 35;
//             fZDiff    = 0;
//             xLastIncr = 0;
//             yLastIncr = 0;
//             fXInertia = -0.5;
//             fYInertia = 0;
//             fScale    = 1.0;
		fXDiff = 0;
		nFrameNum = 1;
		break;
        case GLUT_KEY_LEFT:
           fXDiff--;
        break;
        case GLUT_KEY_RIGHT:
           fXDiff++;
        break;
    }
}


/******************************************************************************/
/*
/* Main
/*
/******************************************************************************/

int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    window = glutCreateWindow( "3Dlabs Brick Shader");

    glutIdleFunc(play);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutReshapeFunc(reshape);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);
    glutSpecialFunc(special);
    glutTimerFunc(TIMER_FREQUENCY_MILLIS , timer, 0);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    key('?', 0, 0);     // display help

	ReadFromPSHFile( &g_pSkinRoot, PSH_FILENAME );
	SetBoneMatrix( g_pSkinRoot, g_pSkinRoot );

 	g_Motion.clearMotionData();
#ifdef CONV_BVH_TO_PSM
 	g_Motion.readBVHfile( BVH_FILENAME );
	
	g_PSMotion.fFrameInterval = g_Motion.getFrameTime();
	g_PSMotion.nJointNum = g_Motion.getNumberOfJoints();
	g_PSMotion.pJoints = new PSJoint[g_PSMotion.nJointNum];
#ifdef MANUAL_RUN
	g_PSMotion.nFrameNum = 18/*22*/;
#else
	g_PSMotion.nFrameNum = g_Motion.getNumberOfFrames();
#endif	
	for (int j = 0 ; j < g_PSMotion.nJointNum ; j++)
	{		
		strcpy( g_PSMotion.pJoints[j].name , g_Motion.joints[j].getName() );
		g_PSMotion.pJoints[j].rotAngle = new vec3[g_PSMotion.nFrameNum];
		for ( int f = 0 ; f < g_PSMotion.nFrameNum ; f++ )
			g_Motion.joints[j].getRotation( f+1, g_PSMotion.pJoints[j].rotAngle[f].x, g_PSMotion.pJoints[j].rotAngle[f].y, g_PSMotion.pJoints[j].rotAngle[f].z );
	}
	g_PSMotion.WriteToPSMFile( PSM_FILENAME );
#endif //CONV_BVH_TO_PSM

	g_PSMotion.ReadFromPSMFile( PSM_FILENAME );
	CurrentFrameLimit = g_PSMotion.nFrameNum;
	
	MatchingJoints( 0, g_pSkinRoot, g_PSMotion.pJoints, g_PSMotion.nJointNum );
	glutMainLoop();
	DestroySkinNodeImages();
	g_Motion.clearMotionData();

	return 0;
}
