#include "stdafx.h"
#include "Global.h"
#include "PSStage.h"

const char* CPSStage::szTextureFiles[] = {
	"Resource/Image/BGIcon.jpg",
	"Resource/Image/TxtTitle.png",
	"Resource/Image/New.png",
	"Resource/Image/New_Over.png",
	"Resource/Image/New_Down.png",
	"Resource/Image/Saved.png",
	"Resource/Image/Saved_Over.png",
	"Resource/Image/Saved_Down.png",
	"Resource/Image/Exit.png",
	"Resource/Image/Exit_Over.png",
	"Resource/Image/Exit_Down.png",

	"Resource/Image/TxtScene.png",
	"Resource/Image/Building1.jpg",
	"Resource/Image/Building1_Select.jpg",
	"Resource/Image/Building2.jpg",
	"Resource/Image/Building2_Select.jpg",
	"Resource/Image/Building3.jpg",
	"Resource/Image/Building3_Select.jpg",
	"Resource/Image/Next.png",
	"Resource/Image/Next_Over.png",
	"Resource/Image/Next_Down.png",
	"Resource/Image/Previous.png",
	"Resource/Image/Previous_Over.png",
	"Resource/Image/Previous_Down.png",

	"Resource/Image/TxtWorker.png",
	"Resource/Image/TxtSex.png",
	"Resource/Image/TxtClothes.png",
	"Resource/Image/TxtMan.png",
	"Resource/Image/TxtMan_Over.png",
	"Resource/Image/TxtMan_Down.png",
	"Resource/Image/TxtMan_Select.png",
	"Resource/Image/TxtWomen.png",
	"Resource/Image/TxtWomen_Over.png",
	"Resource/Image/TxtWomen_Down.png",
	"Resource/Image/TxtWomen_Select.png",
	"Resource/Image/Man_Uniform1.png",
	"Resource/Image/Man_Uniform1_Select.png",
	"Resource/Image/Man_Uniform2.png",
	"Resource/Image/Man_Uniform2_Select.png",
	"Resource/Image/Women_Uniform1.png",
	"Resource/Image/Women_Uniform1_Select.png",
	"Resource/Image/Women_Uniform2.png",
	"Resource/Image/Women_Uniform2_Select.png",

	"Resource/Image/Start.png",
	"Resource/Image/Start_Over.png",
	"Resource/Image/Start_Down.png",

	"Resource/Image/Save.png",
	"Resource/Image/Save_Over.png",
	"Resource/Image/Save_Down.png",
	"Resource/Image/Play.png",
	"Resource/Image/Play_Over.png",
	"Resource/Image/Play_Down.png",
	"Resource/Image/Export.png",
	"Resource/Image/Export_Over.png",
	"Resource/Image/Export_Down.png",
	"Resource/Image/Main.png",
	"Resource/Image/Main_Over.png",
	"Resource/Image/Main_Down.png",

	"Resource/Image/Loading.png",

	"Resource/Image/Help.jpg",
	"Resource/Image/Help_EquipU.jpg",
	"Resource/Image/Help_EquipI.jpg"
};

CString GetAppDirectory()
{
	static CString szAppDir;
	if (szAppDir.IsEmpty())
	{
		TCHAR tcsFileName[512];
		GetModuleFileName(AfxGetInstanceHandle(), tcsFileName, 512*sizeof(TCHAR));

		szAppDir = tcsFileName;

		szAppDir = szAppDir.Left(szAppDir.ReverseFind('\\'));
	}
	return szAppDir;
}

void Token(CString line, CString& key, CString& val)
{
	static const CString filter = "\t :   \n";
	int pos = 0;
	key = line.Tokenize(filter, pos);
	val = line.Tokenize(filter, pos);
}

void GetServerAddress( CString &serverhost, CString &serviceurl )
{
	static CString szServerHost;
	static CString szServiceUrl;
	if ( szServerHost.IsEmpty() || szServiceUrl.IsEmpty() )
	{
		FILE* pFile = fopen( GetAppDirectory() + "\\conf.txt", "rt" );
		char temp[1024];
		CString key, val;

		fgets( temp, 1024, pFile );
		Token(temp, key, val);
		if ( key == "server_host" )
			szServerHost = val;
		else if ( key == "service_url" )
			szServiceUrl = val;

		fgets( temp, 1024, pFile );
		Token(temp, key, val);
		if ( key == "server_host" )
			szServerHost = val;
		else if ( key == "service_url" )
			szServiceUrl = val;

		fclose (pFile);
	}

	serverhost = szServerHost;
	serviceurl = szServiceUrl;
}