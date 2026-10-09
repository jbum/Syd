// GraphWin.cp

#include "SS.h"
#include "GraphWin.h"
#include "MainWin.h"
#include <Sound.h>
#include "SSStorage.h"

GraphWindow	*gw;

GraphWindow::GraphWindow() : ObjectWindow(GraphWindowID, false)
{
	WindowPtr	theWin;
	theWin = (WindowPtr) &this->theWin;
	SetWTitle(theWin, "\pGraph");
	SizeWindow(theWin,GraphWidth,GraphHeight,0);	

	SetPort(theWin);

	TextFont(geneva);
	TextSize(9);

	minPt = 0.0;
	maxPt = 1.0;

	InitGraphics();

	ShowWindow(theWin);
	gw = this;
	Regraph();
}

void GraphWindow::Regraph()
{
	if (mw != NULL && mw->storageMod && mw->storageMod->nbrFrames) {
		GraphSamples(mw->storageMod,mw->storageMod->nbrFrames);
	}
}

GraphWindow::~GraphWindow()
{
	if (offWorld)
		DisposeGWorld(offWorld);
	gw = NULL;
}


void GraphWindow::InitGraphics()
{
	OSErr		oe;
	WindowPtr	theWin = (WindowPtr) &this->theWin;
	CGrafPtr	curPort;
	GDHandle	curDevice;

	SetPort(theWin);

	TextFont(geneva);
	TextSize(9);

	if ((oe = NewGWorld(&offWorld,0,&theWin->portRect,NULL,NULL,0)) != noErr) {
		ErrorExit("Out of Mem");
		return;
	}
	offMap = GetGWorldPixMap(offWorld);
	LockPixels(offMap);

	GetGWorld(&curPort,&curDevice);
	SetGWorld(offWorld,NULL);
	TextFont(geneva);
	TextSize(9);
	EraseRect(&theWin->portRect);
	SetGWorld(curPort,curDevice);
}

// Refresh without Drawing
void GraphWindow::RefreshRect(Rect *rr)
{
	Rect		r;
	GrafPtr	theWin = (WindowPtr) &this->theWin,savePort;

	SectRect(rr,&theWin->portRect,&r);
	if (!EmptyRect(&r)) {
		GetPort(&savePort);
		SetPort(theWin);
		CopyBits((BitMap *) *offMap, &theWin->portBits, 
				&r,&r,srcCopy,NULL);
		SetPort(savePort);
	}
}

// Refresh without drawing
void GraphWindow::Draw(Rect	*rr)
{
	GrafPtr	theWin = (WindowPtr) &this->theWin;
	SetPort(theWin);
	RefreshRect(rr);
}


// Paint Actual Contents
void GraphWindow::Paint(Rect *rr)
{
	int		yd,x;
	int		maxY=-100,minY=100,cy; // ,lastX;
	int32	i,skip;
	int32	startPos = nbrSamples*minPt;
	int32	endPos = nbrSamples*maxPt;
	int32	sampleWidth = endPos - startPos;
	double	xScale = GraphWidth/(double) (endPos - startPos);
	double	yScale = (GraphHeight/2) / (double) 0x07FFF;

	if (storageMod == NULL)
		return;

	if (sampleWidth < GraphWidth*4)
		skip = 1;
	else if (sampleWidth < GraphWidth * 10)
		skip = 4;
	else
		skip = (sampleWidth/GraphWidth)/10;


	EraseRect(rr);
	cy = GraphHeight/2;
	for (i = 0; i < sampleWidth; i += skip) {
		x = (int)(i*xScale);
		if (startPos+i < storageMod->nbrFrames)
			yd = (int) (storageMod->GetSample(startPos+i) * yScale);
		else
			yd = 0;
		MoveTo(x,cy-yd);
		LineTo(x,cy-yd);
	}
}

void GraphWindow::GraphSamples(SSStorage *sm, long nbrFrames)
{
	this->nbrSamples = nbrFrames;
	this->storageMod = sm;
	if (this->storageMod) {
		FullRefresh();
	}
}

void GraphWindow::PartialRefresh(Rect *rr)
{
	GrafPtr	theWin = (WindowPtr) &this->theWin;
	CGrafPtr	curPort;
	GDHandle	curDevice;

	SetPort(theWin);

	GetGWorld(&curPort,&curDevice);
	SetGWorld(offWorld,NULL);
	Paint(rr);
	SetGWorld(curPort,curDevice);

	RefreshRect(rr);
}

// Force Paint of Actual Contents !!!
void GraphWindow::FullRefresh()
{
	GrafPtr	theWin = (WindowPtr) &this->theWin;
	PartialRefresh(&theWin->portRect);
}

void GraphWindow::HandleClick(Point where, EventRecord *theEvent)
{
	Point		p;
	GrafPtr		theWin = (WindowPtr) &this->theWin;
	Boolean		hasOptKey,hasCmdKey;

	p = where;
	SetPort(theWin);
	GlobalToLocal(&p);

	if (!PtInRect(p,&theWin->portRect))
		return;

	hasOptKey = (theEvent->modifiers & optionKey) > 0;
	hasCmdKey = (theEvent->modifiers & cmdKey) > 0;
	if (hasCmdKey) {
		// Reset
		minPt = 0.0;
		maxPt = 1.0;
	}
	else if (!hasOptKey) {
		// Zoom In
		double	newCenter = minPt + (maxPt - minPt) * p.h / (double) theWin->portRect.right;
		double	newWidth = (maxPt - minPt) * .75;
		double minWidth = theWin->portRect.right / (double) nbrSamples;
		if (newWidth < minWidth)
			newWidth = minWidth;
		minPt = newCenter - newWidth / 2;
		maxPt = newCenter + newWidth / 2;
		if (minPt < 0)
			minPt = 0;
		if (maxPt > 1)
			maxPt = 1;
		// Don't allow closer zoom than samplesize...
	}
	else {
		// Zoom Out
		double newCenter = minPt + (maxPt - minPt) * .5;
		double	newWidth = (maxPt - minPt) * 1.5;
		if (newWidth > 1)
			newWidth = 1;
		minPt = newCenter - newWidth / 2;
		maxPt = newCenter + newWidth / 2;
		if (minPt < 0)
			minPt = 0;
		if (maxPt > 1)
			maxPt = 1;
	}
	FullRefresh();
}

void GraphWindow::ProcessKey(EventRecord *er)
{
	char	code,ch;
	ch = er->message & charCodeMask;
	code = (er->message & keyCodeMask) >> 8;

	if (ch == '.' || ch == 0x1b) {
		if (mw)
			mw->ProcessKey(er);
		return;
	}

	if (code == 0x7b) {		// left
		ScrollLeft();
	}
	else if (code == 0x7c)	{	// right
		ScrollRight();
	}
	// 0x7e = up
	// 0x7d = down
}

void GraphWindow::ScrollLeft()
{
	double	incr = (maxPt - minPt) / 4;
	minPt -= incr;
	maxPt -= incr;
	if (minPt < 0) {
		maxPt -= minPt;
		minPt = 0;
	}
	if (maxPt > 1) {
		minPt -= (maxPt - 1);
		maxPt = 1;
	}
	FullRefresh();
}

void GraphWindow::ScrollRight()
{
	double	incr = (maxPt - minPt) / 4;
	minPt += incr;
	maxPt += incr;
	if (minPt < 0) {
		maxPt -= minPt;
		minPt = 0;
	}
	if (maxPt > 1) {
		minPt -= (maxPt - 1);
		maxPt = 1;
	}
	FullRefresh();
}