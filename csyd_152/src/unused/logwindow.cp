/************************************************************************************
 * LogMessage.c - only LogMessage(char *str,...) needs to be shared
 *
 ************************************************************************************/
#include "SS.h"
#include "LogWindow.h"
#include "SSMenus.h"


LogWindow	*gLogWin;

// Create a new main window using a 'WIND' template from the resource fork
//
LogWindow::LogWindow() : TEWindow(LogWIND, false)
{
	WindowPtr	theWin = (WindowPtr) &this->theWin;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	// RestoreWindowPos(theWin, &gMacPrefs.logPos);
	// Resize(theWin, gMacPrefs.logSize.h, gMacPrefs.logSize.v);
	curColor.red = curColor.blue = curColor.green = 0;
	inMiddle = 0;
	logFile = 0;

	TEAutoView(true,teH);
	SetWTitle(theWin,"\pLog");
	gLogWin = this;
	logFile = 0;
	inMiddle = false;
	ShowWindow(theWin);
}

LogWindow::~LogWindow()
{
	gLogWin = NULL;
}

void LogWindow::LogString(char *str)
{
	GrafPtr	savePort;
	Point	units;
	TextStyle	ts;
	short		lineHeight,ascent;
	short		oldStart,oldEnd;
	WindowPtr	theWin = (WindowPtr) &this->theWin;

	if (gLogWin == NULL)
		return;

	GetPort(&savePort);
	SetPort(theWin);

	oldStart = (*teH)->selStart;
	oldEnd = (*teH)->selEnd;

	// Moved this block of code (if statement)
	if ((*teH)->nLines > MaxLines) {
		// Note: this is causing us to get off by 1 in the display
		short	startPoint = (*teH)->lineStarts[(*teH)->nLines - MaxLines];
		TEAutoView(false,teH);	// 10/11/95
		TESetSelect(0,startPoint,teH);
		TEDelete(teH);
		// Deleted these two lines 1/11/95
		// (*teH)->clikStuff = 255;	// Fix SetSelect bug
		// TESetSelect(32767,32767,teH);
		if (!gLogWin->inMiddle)
			TEAutoView(true,teH);	// 10/11/95
		oldStart -= startPoint;	if (oldStart < 0)	oldStart = 0;
		oldEnd -= startPoint; if (oldEnd < 0)		oldEnd = 0;
	}

	TESetSelect(32767,32767,teH);	// 6/21/95
	TEGetStyle((*teH)->selStart,&ts,&lineHeight,&ascent,teH);
	ts.tsColor = this->curColor;
	TESetStyle(doColor,&ts,false,teH);

	TEInsert(str,(long) strlen(str),teH);

	// Was Here!!!
	units.h = 0;
	units.v = (*teH)->nLines;
	SetScrollUnits(units);

	if (!gLogWin->inMiddle) {
		scrollOffset.v = nbrUnits.v - unitsPerScreen.v;
		AdjustScrollBars(false);
	}

	if (gLogWin->logFile) {
		long	count;
		count = strlen(str);
		FSWrite(gLogWin->logFile,&count,str);
	}
	if (oldEnd > oldStart) {
		TEAutoView(false,teH);
		TESetSelect(oldStart,oldEnd,teH);
		if (!gLogWin->inMiddle)
			TEAutoView(true,teH);
	}
	SetPort(savePort);
}

/*
void DisposeLogWin(WindowPtr theWin)
{
	if (gLogWin) {
		// SaveWindowPos(theWin, &gMacPrefs.logPos);
		// SaveWindowSize(theWin, &gMacPrefs.logSize);

		if (gLogWin->logFile) {
			FSClose(gLogWin->logFile);
			gLogWin->logFile = 0;
		}
		DisposeTEWin(theWin);
		DisposePtr((Ptr) theWin);
		gLogWin = NULL;
	}
}
*/
/*
// If Close Box Hit...
void CloseLogWin(WindowPtr theWin)
{
	((ObjectWindowPtr) theWin)->floating = false;
	HideWindow(theWin);
	SendBehind(theWin,(WindowPtr) gRoomWin);
	// SelectDocument((WindowPtr) gRoomWin);
}
*/
/*
void ToggleLogFile(void)
{
	Str255				defName,dateStr;
	unsigned long		secs;
	StandardFileReply	reply;
	OSErr				oe;

	if (gLogWin->logFile) {
		LogMessage("Closing Log\r");
		FSClose(gLogWin->logFile);
		gLogWin->logFile = 0;
	}
	else {
		GetDateTime(&secs);
		IUDateString(secs,abbrevDate,dateStr);
		sprintf((char *) defName, "ULog %.*s",dateStr[0],&dateStr[1]);
		CtoPstr((char *) defName);
		StandardPutFile("\pLog File:",defName,&reply);
		if (!reply.sfGood)
			return;
		FSpDelete(&reply.sfFile);
		if ((oe = FSpCreate(&reply.sfFile,'ttxt','TEXT',reply.sfScript)) != noErr)
		{
			ReportError(oe,"FspCreate");
			return;
		}
		if ((oe = FSpOpenDF(&reply.sfFile,fsRdWrPerm,&gLogWin->logFile)) != noErr)
		{
			ReportError(oe,"FspOpenDF");
			return;
		}
		LogMessage("Opening Log\r");
	}
}
*/

void LogWindow::ProcessCommand(short theMenu, short theItem)
{

	switch (theMenu) {
	case EditMENU:
		switch (theItem) {
		case EM_Copy:
			TECopy(teH);
			return;
		case EM_SelectAll:
			TESetSelect(0,32767,teH);
			return;
		}
		break;
	}
	inherited::ProcessCommand(theMenu,theItem);
}

void LogWindow::AdjustMenus()
{
	Boolean				hasSelection;

	inherited::AdjustMenus();
	hasSelection = (*teH)->selEnd > (*teH)->selStart;
	MyEnableMenuItem(gEditMenu, EM_Copy, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_SelectAll, true);
}

void LogWindow::SetLogColor(RGBColor *color)
{
	this->curColor = *color;
}

void LogWindow::HandleClick(Point where, EventRecord *theEvent)
{
	inherited::HandleClick(where,theEvent);
	if (scrollOffset.v >= nbrUnits.v-unitsPerScreen.v) {
		inMiddle = false;
		TEAutoView(true,teH);
	}
	else {
		inMiddle = true;
		TEAutoView(false,teH);
	}
}


int	gLogIndent;

void AddLogIndent(int n)
{
	gLogIndent += n;
}


void LogMessage(char *str,...)
{
	char 	tbuf[256];
	va_list args;

	if (gLogWin == NULL)
		return;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	if (gLogIndent) {
		char	sbuf[128];
		memset(sbuf,' ',gLogIndent*2);
		sbuf[gLogIndent*2] = 0;
		gLogWin->LogString(sbuf);
	}

	gLogWin->LogString(tbuf);
}

void LogError(char *str,...)
{
	char 	tbuf[256];
	va_list args;


	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	if (gLogWin == NULL)
		new LogWindow();

	if (gLogIndent) {
		char	sbuf[128];
		memset(sbuf,' ',gLogIndent*2);
		sbuf[gLogIndent*2] = 0;
		gLogWin->LogString(sbuf);
	}

	gLogWin->LogString(tbuf);
}
