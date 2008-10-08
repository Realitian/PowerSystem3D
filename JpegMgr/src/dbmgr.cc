#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" unsigned char* decompress_Jpeg( FILE* pJepgFile, int *img_width, int *img_height );
void CreateJpegDB( const char* dirpath, const char* prefix, int startid, int endid )
{
	char dbpath[260];
	strcpy( dbpath, dirpath );
	strcat( dbpath, prefix );
	strcat( dbpath, "db.db" );

	char refpath[260];
	strcpy( refpath, dirpath );
	strcat( refpath, prefix );
	strcat( refpath, "ref.ref" );

	FILE* pRef = fopen( refpath, "wb" );
	FILE* pDB = fopen( dbpath, "wb" );
	if ( pDB && pRef )
	{
		fwrite( &startid, sizeof(int), 1, pRef );
		fwrite( &endid, sizeof(int), 1, pRef );

		for ( int i = startid; i <= endid; i++ )
		{
			char jpegpath[260];
			sprintf ( jpegpath, "%s%s%i.jpg", dirpath, prefix, i );
			FILE *pJpeg = fopen( jpegpath, "rb" );

			if ( pJpeg )
			{
				char buf;
				fwrite( &i, sizeof(int), 1, pRef );
				long dbPos = ftell( pDB );
				fwrite( &dbPos, sizeof(long), 1, pRef );

				while( fread( &buf, 1, 1, pJpeg ) )
				{
					fwrite( &buf, 1, 1, pDB );
				}

				fclose ( pJpeg );
			}
		}

		fclose ( pDB );
		fclose ( pRef );
	}
}

void ReadJpegRef( const char* dirpath, const char* prefix, int &startid, int &endid )
{
	char refpath[260];
	strcpy( refpath, dirpath );
	strcat( refpath, prefix );
	strcat( refpath, "ref.ref" );

	FILE* pRef = fopen( refpath, "rb" );
	if ( pRef )
	{
		fread( &startid, sizeof(int), 1, pRef );
		fread( &endid, sizeof(int), 1, pRef );
		fclose ( pRef );
	}
}

unsigned char* ReadJpegDB( const char* dirpath, const char* prefix, int id, int& img_width, int &img_height )
{
	unsigned char* pBmp = 0;
	
	char dbpath[260];
	strcpy( dbpath, dirpath );
	strcat( dbpath, prefix );
	strcat( dbpath, "db.db" );

	char refpath[260];
	strcpy( refpath, dirpath );
	strcat( refpath, prefix );
	strcat( refpath, "ref.ref" );

	FILE* pRef = fopen( refpath, "rb" );
	FILE* pDB = fopen( dbpath, "rb" );
	if ( pDB && pRef )
	{
		int startid, endid;
		fread( &startid, sizeof(int), 1, pRef );
		fread( &endid, sizeof(int), 1, pRef );

		long dbPos;
		for ( int i = startid; i <= id; i++ )
		{
			fread( &i, sizeof(int), 1, pRef );
			fread( &dbPos, sizeof(long), 1, pRef );
		}
		fseek( pDB, dbPos, SEEK_SET );

		pBmp = decompress_Jpeg( pDB, &img_width, &img_height );

		fclose ( pDB );
		fclose ( pRef );
	}

	return pBmp;
}