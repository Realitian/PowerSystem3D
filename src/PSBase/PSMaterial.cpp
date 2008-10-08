#ifdef SUPPORT_HTTP
#include <afxwin.h>
#include "Global.h"
#include "HttpClient.h"
#endif SUPPORT_HTTP
#include "PSMaterial.h"
#include <Windows.h>
#include <gl/GLU.h>
#include <GdiPlus.h>
#include "zlib.h"

#pragma pack(1)
typedef struct
{
	GLbyte	identsize;              // Size of ID field that follows header (0)
	GLbyte	colorMapType;           // 0 = None, 1 = paletted
	GLbyte	imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
	unsigned short	colorMapStart;          // First colour map entry
	unsigned short	colorMapLength;         // Number of colors
	unsigned char 	colorMapBits;   // bits per palette entry
	unsigned short	xstart;                 // image x origin
	unsigned short	ystart;                 // image y origin
	unsigned short	width;                  // width in pixels
	unsigned short	height;                 // height in pixels
	GLbyte	bits;                   // bits per pixel (8 16, 24, 32)
	GLbyte	descriptor;             // image descriptor
} TGAHEADER;
#pragma pack(8)

void P3DImage::WriteTGA( const char *szFileName )
{
	FILE *pFile;
	TGAHEADER tgaHeader;

	// Initialize the Targa header
	tgaHeader.identsize = 0;
	tgaHeader.colorMapType = 0;
	tgaHeader.imageType = 2;
	tgaHeader.colorMapStart = 0;
	tgaHeader.colorMapLength = 0;
	tgaHeader.colorMapBits = 0;
	tgaHeader.xstart = 0;
	tgaHeader.ystart = 0;
	tgaHeader.width = img_width;
	tgaHeader.height = img_height;
	tgaHeader.bits = 24;
	tgaHeader.descriptor = 0;

	// Attempt to open the file
	pFile = fopen(szFileName, "wb");

	fwrite(&tgaHeader, sizeof(TGAHEADER), 1, pFile);
	fwrite(pPixels, img_width*img_height*3, 1, pFile);
	fclose(pFile);
}

P3DImage::P3DImage()
{
	img_path[0] = '\0';
	img_width = 0;
	img_height = 0;
	pPixels = 0;
	glTexId = 0;
}

P3DImage::~P3DImage()
{
	SAFE_DELETE_ARRAY( pPixels );
}

void P3DImage::GenGLTextures()
{
	if ( glTexId == 0 )
	{
		glGenTextures( 1, &glTexId );
		glBindTexture( GL_TEXTURE_2D, glTexId );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR);
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_REPEAT );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_REPEAT );

		gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB8, img_width, img_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, pPixels );
	}
}

void P3DImage::DestroyGLTextures()
{
	if ( glTexId > 0 )
		glDeleteTextures( 1, &glTexId );
}

P3DMaterial::P3DMaterial()
{
	memset( ambient, 0, sizeof(float)*4 );
	memset( diffuse, 0, sizeof(float)*4 );
	memset( emission, 0, sizeof(float)*4 );
	memset( specular, 0, sizeof(float)*4 );
	shininess = 0;
}

GLbyte *LoadImage(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat, bool bFlipVertical )
{
#ifdef SUPPORT_HTTP
	HttpDownLoad( szFileName );
#endif SUPPORT_HTTP

	using namespace Gdiplus;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	WCHAR wfileName[MAX_PATH];
	MultiByteToWideChar( CP_ACP, 0, szFileName, -1, wfileName, MAX_PATH );
	Bitmap *pBitmap = Bitmap::FromFile( wfileName );
	Status status = pBitmap->GetLastStatus();

	GLbyte* pBits = 0;

	if ( status == Ok )
	{
		if ( bFlipVertical )
			pBitmap->RotateFlip( Rotate180FlipX );
		Rect rc(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight() );
		BitmapData bitmapData;
		pBitmap->LockBits( &rc, ImageLockModeRead, pBitmap->GetPixelFormat(), &bitmapData );

		int channels = 3;

		switch ( bitmapData.PixelFormat )
		{
		case PixelFormat24bppRGB:
			channels = 3;
			break;
		case PixelFormat32bppARGB:
			channels = 4;
			break;
		}

		*iWidth = bitmapData.Width;
		*iHeight = bitmapData.Height;
		switch(channels)
		{
		case 3:     // Most likely case
			*eFormat = GL_BGR_EXT;
			*iComponents = GL_RGB8;
			break;
		case 4:
			*eFormat = GL_BGRA_EXT;
			*iComponents = GL_RGBA8;
			break;
		};

		pBits = (GLbyte*)malloc( bitmapData.Width * bitmapData.Height * channels );
		memcpy( pBits, bitmapData.Scan0, bitmapData.Width * bitmapData.Height * channels );
		pBitmap->UnlockBits( &bitmapData );
		delete pBitmap;

	}
	GdiplusShutdown(gdiplusToken);
	return pBits;
}

#define BUFLEN      16384
void gz_compress(FILE   *in, gzFile out)
{
	char buf[BUFLEN];
	int len;

	for (;;) {
		len = (int)fread(buf, 1, sizeof(buf), in);
		if (ferror(in)) {
			perror("fread");
			exit(1);
		}
		if (len == 0) break;

		if (gzwrite(out, buf, (unsigned)len) != len) 
			return;
	}

	fclose(in);
	if (gzclose(out) != Z_OK)
		return;
}

/* ===========================================================================
* Uncompress input to output then close both files.
*/
void gz_uncompress(gzFile in, FILE   *out)
{
	char buf[BUFLEN];
	int len;

	for (;;) {
		len = gzread(in, buf, sizeof(buf));
		if (len < 0) 
			return;

		if (len == 0) break;

		if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
			return;
		}
	}
	if (fclose(out)) 
		return;

	if (gzclose(in) != Z_OK) 
		return;
}

void file_compress( const char* file )
{
	char outfile[256];
	FILE  *in;

	strcpy(outfile, file);
	strcat( outfile, ".gz" );

	in = fopen(file, "rb");
	if (in )
	{
		gzFile out;
		out = gzopen( outfile, "wb" );
		if ( out )
			gz_compress(in, out);
		
		_unlink( file );
		rename( outfile, file );
	}
}

void file_uncompress( const char* infile, const char* outfile )
{
	gzFile in;
	in = gzopen( infile, "rb" );
	if (in )
	{
		FILE  *out = fopen(outfile, "wb");
		if ( out )
			gz_uncompress(in, out);
	}
}