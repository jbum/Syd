/************************************************************************************
 * ChessMenus.c
 *
 *	Routines for Mu Chess.
 *
 *
 ************************************************************************************/
#include "SS.h"
#include "SSMenus.h"
#include "ObjectWindow.h"
#include "MainWin.h"
#include "GraphWin.h"
#include "LogWindow.h"
#include "MacEvents.h"
#include "ModList.h"

extern MainWindow	*mw;

// Menu Handles
MenuHandle	gAppleMenu, gFileMenu, gEditMenu, gOptionsMenu, gOutputMenu;

void MySetUpMenus(void)
{
	Handle	myMenuBar;
	myMenuBar = GetNewMBar(MenuBaseID);
	SetMenuBar(myMenuBar);

	gAppleMenu = GetMHandle(AppleMENU);
	gFileMenu = GetMHandle(FileMENU);
	gEditMenu = GetMHandle(EditMENU);
	gOptionsMenu = GetMHandle(OptionsMENU);

//	gOutputMenu = GetMenu(OutputMENU);
//	InsertMenu(gOutputMenu, hierMenu);

	AddResMenu(gAppleMenu, 'DRVR');

	DrawMenuBar();
}


//	Enable or disable the items in the Edit menu if a DA window
//	comes up or goes away. Our application doesn't do anything with
//	the Edit menu.
void AppAdjustMenus(void)
{
}


// Code to simplify enabling/disabling menu items.
//
void MyEnableMenuItem(MenuHandle menu, short item, short ok)
{
	if (ok)
		EnableItem(menu, item);
	else
		DisableItem(menu, item);
	if (item == 0)
		DrawMenuBar();
}


//	Handle the menu selection. mSelect is what MenuSelect() and
//	MenuKey() return: the high word is the menu ID, the low word
//	is the menu item
//
void AppProcessCommand(short menuID, short menuItem)
{
	Str255		name;
	GrafPtr		savePort;
	WindowPeek	frontWindow;

	switch (menuID) {
	case AppleMENU:
		if (menuItem == 1) {
			// mw->AboutInfo();
			if (gLogWin == NULL)
				new LogWindow();
			else
				gLogWin->BringWindowToFront();
			ShowVersionNumber();
		}
		else {
			GetPort(&savePort);
			GetItem(gAppleMenu, menuItem, name);
			OpenDeskAcc(name);
			SetPort(savePort);
		}
		break;
	
	case FileMENU:
		switch (menuItem) {
		case FM_New:
			mw = new(MainWindow);
			break;
		case FM_Open:
			{
				MainWindow	*oldMW;
				oldMW = mw;
				if (mw == NULL || mw->mainInst->nbrModules > 1) {
					mw = new(MainWindow);
					if (mw == NULL) {
						mw = oldMW;
						break;
					}
				}
				if (mw != NULL)
					mw->Open();
			}
			break;
		case FM_Close:
			if ((frontWindow = (WindowPeek) FrontWindow()) == 0L)
				break;
 			if (frontWindow->windowKind < 0)
			 	CloseDeskAcc(frontWindow->windowKind);
			else if (frontWindow->refCon == ObjectWindowID) {
				if (WindowPtrToObj(frontWindow)->ConfirmClose()) {
					// WindowPtrToObj(frontWindow)->Dispose();
					delete(WindowPtrToObj(frontWindow));
				}
			}
  			break;
		case FM_Quit:
			if (CloseAllWindows())
				gQuitFlag = true;
			break;
		case FM_Save:
		case FM_SaveAs:
			if (mw)
				mw->ProcessCommand(menuID, menuItem);
			break;
		}
		break;
	case EditMENU:
		break;
	case OptionsMENU:
		switch (menuItem) {
		case OM_Graph:
			if (gw)
				gw->Regraph();
			else
				new GraphWindow();
			break;
		case OM_LogWindow:
			if (gLogWin == NULL)
				new LogWindow();
			else
				gLogWin->BringWindowToFront();
			break;
		case OM_ShowMemory:
		case OM_Listen:
			if (mw)
				mw->ProcessCommand(menuID, menuItem);
			break;
		}
		break;
	}
}

void MyAdjustMenus(void)
{
	register WindowPeek wp;
	Boolean	isObjectWindow;
	wp = (WindowPeek) FrontWindow();
	isObjectWindow = (wp && wp->refCon == ObjectWindowID);
	if (isObjectWindow)
		WindowPtrToObj(wp)->AdjustMenus();
	else
		AppAdjustMenus();
}



void MyHandleMenu (long mSelect)
{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	register 	WindowPeek wp;
	Boolean		isObjectWindow;

	wp = (WindowPeek) FrontWindow();
	isObjectWindow = (wp && wp->refCon == ObjectWindowID);
	if (isObjectWindow)
		WindowPtrToObj(wp)->ProcessCommand( menuID, menuItem);
	else
		AppProcessCommand(menuID, menuItem);
	HiliteMenu(0);
}