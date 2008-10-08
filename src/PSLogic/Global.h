#ifndef _GLOBAL_H_
#define _GLOBAL_H_

// Stage Manage
enum PSStage
{
	INIT = 0,
	SELECT_SCENE,
	SELECT_HUMEN,
	SHOW_PREVIEW,
	LOADING,
	OVERVIEW_SCENE,
	STAND,
	EXITING,
	NAVIGATING,
	PLAYING,
	HELP,
	EXIT
};

enum PSEquipOpStage
{
	EQUIP_OP_NONE = 0,
	EQUIP_OP_START,
	EQUIP_OP_OPERATING,
};

enum PSHelp
{
	HELP_NONE,
	HELP_DEFAULT,
	HELP_EQUIPU,
	HELP_EQUIPI,
};

enum TEXTURE_ID
{
	TEXID_FIRST = 0,

	TEXID_BGICON = TEXID_FIRST,
	TEXID_TXT_TITLE,
	TEXID_NEW,
	TEXID_NEW_OVER,
	TEXID_NEW_DOWN,
	TEXID_SAVED,
	TEXID_SAVED_OVER,
	TEXID_SAVED_DOWN,
	TEXID_EXIT,
	TEXID_EXIT_OVER,
	TEXID_EXIT_DOWN,

	TEXID_TXT_SCENE,
	TEXID_BUILDING1,
	TEXID_BUILDING1_SELECT,
	TEXID_BUILDING2,
	TEXID_BUILDING2_SELECT,
	TEXID_BUILDING3,
	TEXID_BUILDING3_SELECT,
	TEXID_NEXT,
	TEXID_NEXT_OVER,
	TEXID_NEXT_DOWN,
	TEXID_PREVIOUS,
	TEXID_PREVIOUS_OVER,
	TEXID_PREVIOUS_DOWN,

	TEXID_TXT_WORKER,
	TEXID_TXT_SEX,
	TEXID_TXT_CLOTHES,
	TEXID_TXT_MAN,
	TEXID_TXT_MAN_OVER,
	TEXID_TXT_MAN_DOWN,
	TEXID_TXT_MAN_SELECT,
	TEXID_TXT_WOMEN,
	TEXID_TXT_WOMEN_OVER,
	TEXID_TXT_WOMEN_DOWN,
	TEXID_TXT_WOMEN_SELECT,
	TEXID_MAN_UNIFORM1,
	TEXID_MAN_UNIFORM1_SELECT,
	TEXID_MAN_UNIFORM2,
	TEXID_MAN_UNIFORM2_SELECT,
	TEXID_WOMEN_UNIFORM1,
	TEXID_WOMEN_UNIFORM1_SELECT,
	TEXID_WOMEN_UNIFORM2,
	TEXID_WOMEN_UNIFORM2_SELECT,

	TEXID_START,
	TEXID_START_OVER,
	TEXID_START_DOWN,
	
	TEXID_SAVE,
	TEXID_SAVE_OVER,
	TEXID_SAVE_DOWN,
	TEXID_PLAY,
	TEXID_PLAY_OVER,
	TEXID_PLAY_DOWN,
	TEXID_EXPORT,
	TEXID_EXPORT_OVER,
	TEXID_EXPORT_DOWN,
	TEXID_MAINMENU,
	TEXID_MAINMENU_OVER,
	TEXID_MAINMENU_DOWN,

	TEXID_LOADING,

	TEXID_HELP,
	TEXID_HELP_EQUIPU,
	TEXID_HELP_EQUIPI,

	TEXID_LAST = TEXID_HELP_EQUIPI,
	TEXID_COUNT
};

enum HIT_ID
{
	HITID_NONE = 0,

	HITID_NEW,
	HITID_NEW_OVER,
	HITID_NEW_DOWN,
	HITID_SAVED,
	HITID_SAVED_OVER,
	HITID_SAVED_DOWN,
	HITID_EXIT,
	HITID_EXIT_OVER,
	HITID_EXIT_DOWN,

	HITID_BUILDING1,
	HITID_BUILDING1_SELECT,
	HITID_BUILDING2,
	HITID_BUILDING2_SELECT,
	HITID_BUILDING3,
	HITID_BUILDING3_SELECT,
	HITID_NEXT,
	HITID_NEXT_OVER,
	HITID_NEXT_DOWN,
	HITID_PREVIOUS,
	HITID_PREVIOUS_OVER,
	HITID_PREVIOUS_DOWN,

	HITID_TXT_MAN,
	HITID_TXT_MAN_OVER,
	HITID_TXT_MAN_DOWN,
	HITID_TXT_MAN_SELECT,
	HITID_TXT_WOMEN,
	HITID_TXT_WOMEN_OVER,
	HITID_TXT_WOMEN_DOWN,
	HITID_TXT_WOMEN_SELECT,
	HITID_MAN_UNIFORM1,
	HITID_MAN_UNIFORM1_SELECT,
	HITID_MAN_UNIFORM2,
	HITID_MAN_UNIFORM2_SELECT,
	HITID_WOMEN_UNIFORM1,
	HITID_WOMEN_UNIFORM1_SELECT,
	HITID_WOMEN_UNIFORM2,
	HITID_WOMEN_UNIFORM2_SELECT,

	HITID_START,
	HITID_START_OVER,
	HITID_START_DOWN,

	HITID_SAVE,
	HITID_SAVE_OVER,
	HITID_SAVE_DOWN,
	HITID_PLAY,
	HITID_PLAY_OVER,
	HITID_PLAY_DOWN,
	HITID_EXPORT,
	HITID_EXPORT_OVER,
	HITID_EXPORT_DOWN,
	HITID_MAINMENU,
	HITID_MAINMENU_OVER,
	HITID_MAINMENU_DOWN,
};

#define BUFFER_LENGTH				64
//
CString GetAppDirectory();
void GetServerAddress( CString &serverhost, CString &serviceurl );

#endif //_GLOBAL_H_