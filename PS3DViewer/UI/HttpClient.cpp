#include <afxinet.h>
#include <afxcoll.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include "Global.h"

bool HttpDownLoad( const char* filename )
{
	if( strstr( filename, GetAppDirectory() ) )
	{
		filename += GetAppDirectory().GetLength() + 1;
	}

	CString filepath = GetAppDirectory() + "\\";
	filepath += filename;
	FILE *pFile = fopen( filepath, "rb" );
	if ( pFile )
	{
		fclose ( pFile );
		return true;
	}

	CString serverhost;
	CString serviceurl;
	GetServerAddress( serverhost, serviceurl );

	CInternetSession session;
	CHttpFile* file = 0;
	CHttpConnection* pHttpConnection = 0;
	try
	{
		pHttpConnection = session.GetHttpConnection( serverhost );

		CString downloadurl = serviceurl + "/ps3d.php?mode=download&filename=";
		downloadurl += filename;

		file = pHttpConnection->OpenRequest( CHttpConnection::HTTP_VERB_GET, downloadurl );
		file->SendRequest( );
	}
	catch (CInternetException* e)
	{
		e->ReportError();
		e->Delete();
		if ( file )
			delete file;
		if ( pHttpConnection )
			delete pHttpConnection;

		return false;
	}

	if ( file )
	{
		char buf[1024];

		int rb = file->Read( buf, 1024 );
		FILE *pFile = 0;
		fopen_s( &pFile, filename, "wb" );

		while( pFile && rb )
		{
			fwrite( buf, rb, 1, pFile );
			rb = file->Read( buf, 1024 );
		}
		fclose( pFile );

		file->Close();
		delete file;
		delete pHttpConnection;
	}

	return true;
}

CStringArray* HttpListScene( )
{
	CInternetSession session;
	CHttpFile* file = 0;
	CHttpConnection* pHttpConnection = 0;
	CStringArray* SceneList = 0;
	
	CString serverhost;
	CString serviceurl;
	GetServerAddress( serverhost, serviceurl );

	try
	{
		pHttpConnection = session.GetHttpConnection( serverhost );
		file = pHttpConnection->OpenRequest( CHttpConnection::HTTP_VERB_GET, serviceurl+"/ps3d.php?mode=listscene" );
		file->SendRequest( );
	}
	catch (CInternetException* e)
	{
		e->ReportError();
		e->Delete();
		if ( file )
			delete file;
		if ( pHttpConnection )
			delete pHttpConnection;

		return 0;
	}

	if ( file )
	{
		SceneList = new CStringArray;
		CString scene;
		while ( file->ReadString( scene ) )
			SceneList->Add( scene );
		file->Close();
		delete file;
		delete pHttpConnection;
	}

	return SceneList;
}

bool HttpUpload( const char* filepath )
{
	CInternetSession session;
	CHttpFile* file = 0;
	CHttpConnection* pHttpConnection = 0;
	
	CString serverhost;
	CString serviceurl;
	GetServerAddress( serverhost, serviceurl );

	try
	{
		pHttpConnection = session.GetHttpConnection( serverhost );
		file = pHttpConnection->OpenRequest( CHttpConnection::HTTP_VERB_POST, serviceurl+"/ps3d.php?mode=upload" );

		CString boundary( "-----------------------------7db33c2110466" );

		CString bodystart = "--" + boundary + "\r\n";
		CString bodyend = "\r\n--" + boundary + "--\r\n";

		bodystart += "Content-Disposition: form-data; name=\"userfile\"; filename=\"";
		bodystart += filepath;
		bodystart += "\"\r\n";
		bodystart += "Content-Type: application/octet-stream\r\n\r\n";

		int fLen = 0;
		int fh = 0;
		char *fhBuf = 0;
		if ( _sopen_s( &fh, filepath, _O_RDONLY|_O_BINARY, _SH_DENYNO, _S_IREAD ) == 0 )
		{
			fLen = _filelength( fh );
			fhBuf = new char[ fLen + bodystart.GetLength() + bodyend.GetLength() ];
			memcpy( fhBuf, bodystart.GetBuffer(), bodystart.GetLength() );
			_read( fh, fhBuf + bodystart.GetLength() , fLen );
			memcpy( fhBuf + bodystart.GetLength() + fLen, bodyend.GetBuffer(), bodyend.GetLength() );
			_close( fh );
		}

		CString strHeaders = "";
		strHeaders += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
		strHeaders += "Accept-Encoding: gzip, deflate\r\n";

		char temp[1024];
		sprintf_s( temp, "Content-Length: %d\r\n", fLen + bodystart.GetLength() + bodyend.GetLength() );
		strHeaders += temp;

		file->SendRequest( strHeaders, fhBuf, fLen + bodystart.GetLength() + bodyend.GetLength() );
		delete[] fhBuf;
	}
	catch (CInternetException* e)
	{
		e->ReportError();
		e->Delete();
		if ( file )
			delete file;
		if ( pHttpConnection )
			delete pHttpConnection;

		return false;
	}

	if ( file )
	{
		file->Close();

		delete file;
		delete pHttpConnection;
	}

	return true;
}