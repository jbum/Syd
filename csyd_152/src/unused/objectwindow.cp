/************************************************************************************
 * ObjectWindow.c						
 *
 * Grid Window Routines
 ************************************************************************************/
#include "ObjectWindow.h"
#include "SS.h"

ObjectWindow::ObjectWindow(short resID, Boolean isFloating)
{
	WindowPtr	wp;
	wp = GetNewCWindow(resID, (WindowPtr) &this->theWin, (WindowPtr) -1L);
	((WindowPeek) wp)->refCon = ObjectWindowID;
	activeClick = false;
	floating = isFloating;
	active = false;
}

ObjectWindow::~ObjectWindow(void)
{
	CloseWindow((WindowPtr) &this->theWin);
}

void ObjectWindow::Update(void)
{
	GrafPtr	savePort,theWin;
	theWin = (WindowPtr) &this->theWin;
	GetPort(&savePort);
	SetPort(theWin);
	BeginUpdate(theWin);
	this->Draw(&theWin->portRect);
	EndUpdate(theWin);
	SetPort(savePort);
}

void ObjectWindow::Activate(Boolean activeFlag)
{
	GrafPtr	savePort,theWin;
	theWin = (WindowPtr) &this->theWin;
	GetPort(&savePort);
	SetPort(theWin);
	InvalRect(&theWin->portRect);
	SetPort(savePort);
	this->active = activeFlag;
}


void ObjectWindow::Refresh(void)
{
	GrafPtr	savePort,theWin;
	theWin = (WindowPtr) &this->theWin;
	GetPort(&savePort);
	SetPort(theWin);
	InvalRect(&theWin->portRect);
	SetPort(savePort);
}

void ObjectWindow::HandleClick(Point where, EventRecord *theEvent)
{
}

void ObjectWindow::ProcessKey(EventRecord *theEvent)
{
}

void ObjectWindow::Draw(Rect *rr)
{
}

void ObjectWindow::Idle(EventRecord *theEvent)
{
}

Boolean ObjectWindow::ConfirmClose()
{
	return true;
}

void ObjectWindow::Resize(short w, short h)
{
	GrafPtr	theWin = (WindowPtr) &this->theWin;
	SizeWindow(theWin, w, h, true);
	// Adjust scroll bars?
	// Draw Page ?
	SetPort(theWin);
	InvalRect(&theWin->portRect);
}

void ObjectWindow::ProcessCommand(short theMenu, short theItem)
{
	AppProcessCommand(theMenu, theItem);
}

void ObjectWindow::AdjustMenus()
{
	AppAdjustMenus();
}

void ObjectWindow::BringWindowToFront()
{
	GrafPtr	theWin = (WindowPtr) &this->theWin;
	SelectWindow(theWin);
}

/*
Boolean ObjectWindow::Save(SFReply *itsReply)
{
	return true;
}
*/