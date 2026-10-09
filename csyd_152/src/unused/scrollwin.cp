// ScrollWin.cp

#include "ScrollWin.h"
#include <PPCToolBox.h>

// Descendent window must maintain lineHeight, nbrLines

ScrollWindow::ScrollWindow(short winID, Boolean vertScroll, Boolean horizScroll, Boolean isFloating) : ObjectWindow(winID, isFloating)
{
	WindowPtr	theWin = (WindowPtr) &this->theWin;

	vScrollBar = NULL;
	hScrollBar = NULL;
	scrollOffset.h = scrollOffset.v = 0;
	nbrUnits.h = nbrUnits.v = 0;
	unitsPerScreen.h = unitsPerScreen.v = 0; 
	unitHeight.h = unitHeight.v = 0;
	
	SetupScrollBars(horizScroll,vertScroll);

	// Show the window
	// ShowWindow(theWin);

	// Make it the current grafport
	// SetPort(theWin);
}

ScrollWindow::~ScrollWindow()
{
}

void ScrollWindow::SetupScrollBars(Boolean horiz, Boolean vert)
{
	Rect			sRect,r;
	WindowPtr		theWin = (WindowPtr) &this->theWin;
	// !! Set up vertical scroll bar
	//
	if (vert) {
		r = theWin->portRect;
		sRect.left = r.right - (SBarSize - 1);
		sRect.top = r.top - 1;
		sRect.right = r.right + 1;
		sRect.bottom = r.bottom  - GrowIconSize;
	 	vScrollBar = NewControl(theWin,&sRect,"\p",true,
	 								0,0,sRect.bottom - sRect.top, scrollBarProc,1L);
	}
	if (horiz) {
		r = theWin->portRect;
		sRect.top = r.bottom - (SBarSize - 1);
		sRect.left = r.left - 1;
		sRect.bottom = r.bottom + 1;
		sRect.right = r.right  - GrowIconSize;
	 	vScrollBar = NewControl(theWin,&sRect,"\p",true,
	 								0,0,0, scrollBarProc,1L);
	}
	AdjustScrollBars(true);
}

void ScrollWindow::Resize(short w, short h)
{
	inherited::Resize(w,h);
	ComputeUnitsPerScreen();
	ComputeUnits();
	AdjustScrollBars(true);
}

void ScrollWindow::Activate(Boolean activeFlag)
{
	inherited::Activate(activeFlag);

	if (vScrollBar)
		HiliteControl(vScrollBar, activeFlag? 0 : 255);
	if (hScrollBar)
		HiliteControl(hScrollBar, activeFlag? 0 : 255);
}

void ScrollWindow::ComputeUnitsPerScreen()
{
	WindowPtr		theWin = (WindowPtr) &this->theWin;
	
	if (unitHeight.v == 0)
		unitsPerScreen.v = 1;
	else
		unitsPerScreen.v = ((theWin->portRect.bottom - theWin->portRect.top) - SBarSize)/unitHeight.v;
	if (unitHeight.h == 0)
		unitsPerScreen.h = 1;
	else
		unitsPerScreen.h = ((theWin->portRect.right - theWin->portRect.left) - SBarSize)/unitHeight.h;
}

void ScrollWindow::ComputeUnits()
{
	// Assume they are already correct
	AdjustScrollBars(false);
}


void ScrollWindow::SetScrollUnits(Point p)
{
	nbrUnits.h = p.h;
	nbrUnits.v = p.v;
	AdjustScrollBars(false);
}


void ScrollWindow::AdjustScrollBars(Boolean resizeFlag)
{
	short			w,h;
	GrafPtr			savePort;
	WindowPtr		theWin = (WindowPtr) &this->theWin;

	GetPort(&savePort);

	if (resizeFlag) {
		// Adjust Lines Per Page
		ComputeUnitsPerScreen();

		// Move sliders to new position
		//
		MoveControl(vScrollBar, theWin->portRect.right - (SBarSize - 1),
											theWin->portRect.top - 1);	
		// Change their sizes to fit new window dimensions
		//
		w = 16;
		h = theWin->portRect.bottom - theWin->portRect.top - (GrowIconSize - 1);
		SizeControl(vScrollBar,w,h);

		// Reset their maximum values
		//
		SetCtlMax(vScrollBar,h);
	}

	// Set the value of the sliders accordingly
	//
	h = theWin->portRect.bottom - theWin->portRect.top - (GrowIconSize - 1);
	if (nbrUnits.v > 0) {
		h = nbrUnits.v - unitsPerScreen.v;
		if (h < 0)
			h = 0;
		SetCtlMax(vScrollBar,h);
		if (scrollOffset.v > h)
			scrollOffset.v = h;
		SetCtlValue(vScrollBar,scrollOffset.v);
	}
	else {
		SetCtlMax(vScrollBar,0);
		SetCtlValue(vScrollBar,0);
	}
		
	SetPort(savePort);
}

//
// Callback routine to handle arrows and page up, page down
//
ScrollWindow		*gDWin;

pascal void MyScrollAction(ControlHandle theControl, short thePart);

pascal void MyScrollAction(ControlHandle theControl, short thePart)
{
	Point		curPos,newPos;
	short		pageWidth;
	Rect		myRect;
	WindowPtr	gp = (GrafPtr) gDWin;

	curPos = gDWin->scrollOffset;
	newPos = curPos;

	myRect = ((WindowPtr) gp)->portRect;
	myRect.right -= SBarSize-1;
	myRect.bottom -= SBarSize-1;

	pageWidth = (gDWin->unitsPerScreen.v-1);

	if (theControl == ((ScrollWindow *) gp)->vScrollBar) {
		switch (thePart) {
		  case inUpButton:		newPos.v = curPos.v - 1;			break;
		  case inDownButton:	newPos.v = curPos.v + 1;			break;
	  	  case inPageUp:		newPos.v = curPos.v - pageWidth;	break;
		  case inPageDown:		newPos.v = curPos.v + pageWidth;	break;
		}
	}
	else {
		switch (thePart) {
		  case inUpButton:		newPos.h = curPos.h - 1;			break;
		  case inDownButton:	newPos.h = curPos.h + 1;			break;
	  	  case inPageUp:		newPos.h = curPos.h - pageWidth;	break;
		  case inPageDown:		newPos.h = curPos.h + pageWidth;	break;
		}
	}

	if (newPos.h > gDWin->nbrUnits.h - gDWin->unitsPerScreen.h)
		newPos.h = gDWin->nbrUnits.h - gDWin->unitsPerScreen.h;
	if (newPos.h < 0)
		newPos.h = 0;
	if (newPos.v > gDWin->nbrUnits.v - gDWin->unitsPerScreen.v)
		newPos.v = gDWin->nbrUnits.v - gDWin->unitsPerScreen.v;
	if (newPos.v < 0)
		newPos.v = 0;

	if (!EqualPt(newPos,curPos)) {
		gDWin->ScrollToPosition(newPos);
	}
}

// Intercept Handler for scroll bars
// Returns true if user clicked on scroll bar
//

Boolean ScrollWindow::HandleControlClick(Point where)
{
	short 			ctlPart;
	ControlHandle	whichControl;
	short			vPos;
	ProcPtr			trackActionProc;
	ControlActionUPP clickProc;
	Boolean			trackResult;
	WindowPtr		theWin = (WindowPtr) &this->theWin;


	// Check if user clicked on scrollbar
	//
	if ((ctlPart = FindControl(where,theWin,&whichControl)) == 0)
		return false;

	// Identify window for callback procedure
	//
	gDWin = this;

	// Use default behavior for thumb
	// Program will crash if you don't!!
	//
	if (ctlPart == inThumb)
		trackActionProc = 0L;
	else
		trackActionProc = (ProcPtr) MyScrollAction;

	// Perform scrollbar tracking
	//
	clickProc = NewControlActionProc(trackActionProc);
	trackResult = ((ctlPart = TrackControl(whichControl, where, clickProc)) != 0);
	DisposeRoutineDescriptor(clickProc);
	if (!trackResult)
		return false;

	// if ((ctlPart = TrackControl(whichControl, where, trackActionProc)) == 0)
	//	return false;
	if (ctlPart == inThumb) {
		Point	newPos;
		vPos = GetCtlValue(vScrollBar);
		newPos = scrollOffset;
		newPos.v = vPos;
		ScrollToPosition(newPos);
	}

	return true;
}

// Scroll to an explicit position
//
void ScrollWindow::ScrollToPosition(Point newPos)
{
	WindowPtr		theWin = (WindowPtr) &this->theWin;

	SetPort(theWin);

	if (newPos.h > nbrUnits.h-unitsPerScreen.h)
		newPos.h = nbrUnits.h-unitsPerScreen.h;
	if (newPos.h < 0)
		newPos.h = 0;
	if (newPos.v > nbrUnits.v-unitsPerScreen.v)
		newPos.v = nbrUnits.v-unitsPerScreen.v;
	if (newPos.v < 0)
		newPos.v = 0;

	scrollOffset = newPos;
	
	// Adjust Scrollbars
	AdjustScrollBars(false);

	UpdateToScrollPosition();
}

void ScrollWindow::UpdateToScrollPosition()
{
	WindowPtr		theWin = (WindowPtr) &this->theWin;
	Draw(&theWin->portRect);
}

void ScrollWindow::AutoScroll(Point pos)
{
	Point	offset;
	offset = scrollOffset;
	WindowPtr		theWin = (WindowPtr) &this->theWin;

	if (pos.v < theWin->portRect.top)
		offset.v--;
	else if (pos.v >= theWin->portRect.bottom)
		offset.v++;
	if (pos.h < theWin->portRect.left)
		offset.h--;
	else if (pos.h >= theWin->portRect.right)
		offset.h++;
	if (!EqualPt(offset,scrollOffset))
		ScrollToPosition(offset);
}

/*

#define FooterHeight	72
#define TopMargin		0
#define HeaderHeight	72
#define BotMargin		0
#define DescendHeight	4

void ScrollWindow::PrintWindow()
{
	TPPrPort	printPort;
	TPrStatus	prStatus;
	GrafPtr		savePort;
	Boolean		ok;
	Rect		r;
	short		pageNbr,startPage,endPage,nbrPages;
	short		linesPerPage;


	GetPort(&savePort);
	PrOpen();

	ok = PrValidate(gHPrint);
	ok = PrJobDialog(gHPrint);
	if (ok) {
		printPort = PrOpenDoc(gHPrint,NULL,NULL);

		r = printPort->gPort.portRect;
		linesPerPage = ((r.bottom - FooterHeight) - TopMargin - BotMargin - HeaderHeight)/sWin->unitHeight.v;
		nbrPages = sWin->nbrUnits.v/linesPerPage + 1;

		startPage = (**gHPrint).prJob.iFstPage;
		endPage = (**gHPrint).prJob.iLstPage;
		if (startPage > nbrPages) {
			PrCloseDoc(printPort);
			// ErrorAlert(ES_Caution, "Selected pages are out of range 1-%d",nbrPages);
			goto ErrorExit;
		}
		if (endPage > nbrPages)
			endPage = nbrPages;

		gIsPrinting = true;

		for (pageNbr = 1; pageNbr <= nbrPages; ++pageNbr) {
			SetPort(&printPort->gPort);
			PrOpenPage(printPort, NULL);
	
			if (pageNbr >= startPage && pageNbr <= endPage) {
				r = printPort->gPort.portRect;
				// DrawHeader(dWin, &r);	
					
				r.top += (HeaderHeight+TopMargin+sWin->unitHeight.v-DescendHeight);
				r.bottom -= (FooterHeight + DescendHeight + BotMargin);
		
				Draw();
	
				// r.top = r.bottom + DescendHeight + BotMargin;
				// r.bottom = r.top + FooterHeight;
				// DrawFooter(dWin, &r, pageNbr, nbrPages);
			}
			PrClosePage(printPort);
		}
		gIsPrinting = false;

		PrCloseDoc(printPort);
		if ((**gHPrint).prJob.bJDocLoop == bSpoolLoop && PrError() == noErr)
			PrPicFile(gHPrint, NULL, NULL, NULL, &prStatus);
	}
ErrorExit:
	PrClose();
	SetPort(savePort);
}

*/