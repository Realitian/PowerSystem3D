#ifndef _PSMATERIAL_H_
#define _PSMATERIAL_H_

#include "PSDefine.h"
#include <vector>
#include <Windows.h>
#include <gl/GLU.h>

using namespace std;

struct P3DImage
{
	P3DImage();
	~P3DImage();
	
	char img_path[PATH_LEN];
	int img_width;
	int img_height;
	unsigned char* pPixels;

	unsigned int glTexId;
	void WriteTGA( const char *szFileName );
	void GenGLTextures();
	void DestroyGLTextures();
};

struct P3DMaterial
{
	P3DMaterial();

	float ambient[4];
	float diffuse[4];
	float emission[4];
	float specular[4];
	float shininess;
};

GLbyte *LoadImage(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat, bool bFlipVertical = false );
void file_compress( const char* file );
void file_uncompress( const char* infile, const char* outfile );

#endif //_PSMATERIAL_H_