// ChessMenus.h

#define MenuBaseID	128

// Menu Resource IDs
enum	{
	AppleMENU = 128, FileMENU, EditMENU, OptionsMENU};

extern MenuHandle	gAppleMenu, gFileMenu, gEditMenu, gOptionsMenu;

// Menu Item Numbers
enum	{FM_New=1, FM_Open, FM_Close, FM_Save, FM_SaveAs, FM_Div1,
		 FM_PageSetup, FM_Print, FM_Div3, 
		 FM_Quit};

enum 	{EM_Undo=1, EM_Div1, 
		 EM_Cut, EM_Copy, EM_Paste, EM_Clear, EM_Div2,
		 EM_Duplicate, EM_SelectAll};
		 
enum	{OM_LogWindow=1, OM_Graph, OM_ShowMemory, OM_Listen, OM_Div1,OM_OutputTo};

void MySetUpMenus(void);
void MyEnableMenuItem (MenuHandle menu, short item, short ok);
void MyHandleMenu (long mSelect);
void MyAdjustMenus(void);