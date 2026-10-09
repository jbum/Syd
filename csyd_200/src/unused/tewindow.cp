#include "SS.h"
#include "TEWindow.h"
#include "SSMenus.h"

// Create a new main window using a 'WIND' template from the resource fork
//
TEWindow::TEWindow(short winID, Boolean floating) : ScrollWindow(winID, true, false, floating)
{
	WindowPtr	theWin = (WindowPtr) &this->theWin;
	Rect			r;
	TextStyle		ts;
	short			lineHeight,ascent;

	teH = NULL;
	dirty = false;

	// Show the window
	ShowWindow(theWin);

	// Make it the current grafport
	SetPort(theWin);

	TextFont(monaco);
	TextSize(9);

	r = theWin->portRect;
	r.right -= 15;
	r.bottom -= 15;
	InsetRect(&r,2,2);
	teH = TEStylNew(&r,&r);
	TECalText(teH);
	TEAutoView(true,teH);
	TEGetStyle(0,&ts,&lineHeight,&ascent,teH);
	unitHeight.v = lineHeight;
	unitHeight.h = 0;
	ComputeUnitsPerScreen();
}

TEWindow::~TEWindow()
{
	TEDispose(teH);
}

// ScrollWindow Override
void TEWindow::UpdateToScrollPosition()
{
	WindowPtr	theWin = (WindowPtr) &this->theWin;
	Rect			r;

	r = (*teH)->viewRect;
	r.bottom -= (r.bottom - r.top) % unitHeight.v;
	(*teH)->viewRect = r;

	r.bottom = r.top + unitHeight.v * (*teH)->nLines;
	OffsetRect(&r,0,-unitHeight.v * scrollOffset.v);
	(*teH)->destRect = r;

	Draw(&theWin->portRect);
}

// Respond to an update event - BeginUpdate has already been called.
//
void TEWindow::Draw(Rect *rr)
{
	WindowPtr	theWin = (WindowPtr) &this->theWin;
	// Erase the content area
	EraseRect(&theWin->portRect);

	// Draw the grow icon in the corner
	DrawGrowIcon(theWin);
	UpdtControl(theWin,theWin->visRgn);

	TEUpdate(rr, teH);
}


// Respond to a mouse-click - highlight cells until the user releases the button
//
void TEWindow::HandleClick(Point where, EventRecord *theEvent)
{
	Point	p = where;
	WindowPtr	theWin = (WindowPtr) &this->theWin;
	// Rect	r;


	SetPort(theWin);
	GlobalToLocal(&p);

	if (HandleControlClick(p)) {
		return;
	}
	else
		TEClick(p,(theEvent->modifiers & shiftKey) > 0,teH);
}

void TEWindow::ProcessKey(EventRecord *theEvent)
{
	WindowPtr	theWin = (WindowPtr) &this->theWin;
	char	theChar,theCode;
	SetPort(theWin);

	theChar = theEvent->message & charCodeMask;
	theCode = (theEvent->message & keyCodeMask) >> 8;

	switch (theCode) {
	case 0x73:		// Home
			TESetSelect(0,0,teH);
			break;
	case 0x77:		// End
			TESetSelect(32767,32767,teH);
			break;
	case 0x75:		// KP Del
		if ((*teH)->selEnd > (*teH)->selStart) {
			TEKey('\b',teH);
		}
		else {
			TESetSelect((*teH)->selStart,(*teH)->selStart+1,teH);
			if ((*teH)->selEnd > (*teH)->selStart)
				TEKey('\b',teH);
		}
		dirty = true;
		break;
	default:
		TEKey(theChar,teH);
		dirty = true;
	}
}


void TEWindow::Idle(EventRecord *theEvent)
{
	TEIdle(teH);
}

void TEWindow::Activate(Boolean activeFlag)
{
	if (activeFlag)
		TEActivate(teH);
	else
		TEDeactivate(teH);
	inherited::Activate(activeFlag);
}

void TEWindow::Resize(short width, short height)
{
	WindowPtr	theWin = (WindowPtr) &this->theWin;
	Rect	r;

	inherited::Resize(width,height);

	r = theWin->portRect;
	r.right -= 15;
	r.bottom -= 15;
	InsetRect(&r,2,2);
	(*teH)->destRect = r;
	(*teH)->viewRect = r;
	TECalText(teH);
	AdjustScrollBars(true);
}

void TEWindow::ProcessCommand(short theMenu, short theItem)
{
	WindowPtr	theWin = (WindowPtr) &this->theWin;

	switch (theMenu) {
/*
	case FileMENU:
		switch (theItem) {
		case FM_Save:
			if (((ObjectWindowPtr) theWin)->Save) {
				((ObjectWindowPtr) theWin)->Save(theWin);
				tw->dirty = false;
				return;
			}
			break;
		}
		break;
*/
	case EditMENU:
		switch (theItem) {
		case EM_Copy:
			TECopy(teH);
			return;
		case EM_Cut:
			TECut(teH);
			dirty = true;
			return;
		case EM_Clear:
			TEDelete(teH);
			dirty = true;
			return;
		case EM_Paste:
			TEPaste(teH);
			dirty = true;
			return;
		case EM_SelectAll:
			TESetSelect(0,32767,teH);
			return;
		}
		break;
	}
	inherited::ProcessCommand(theMenu,theItem);
}

void TEWindow::AdjustMenus()
{
	Boolean				hasSelection;
	Boolean				hasClip;
	hasSelection = (*teH)->selEnd > (*teH)->selStart;
	// hasClip = false;

	hasClip = CanPaste('TEXT');

	inherited::AdjustMenus();

	MyEnableMenuItem(gFileMenu, FM_Save, dirty);

	MyEnableMenuItem(gEditMenu, EM_Copy, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Cut, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Clear, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Paste, hasClip);
	MyEnableMenuItem(gEditMenu, EM_SelectAll, true);
}
