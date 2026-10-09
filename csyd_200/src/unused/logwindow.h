// LogWindow.h

#ifndef _H_LogWin
#define _H_LogWin	1

#include <QDOffscreen.h>
#include <sound.h>

#include "TEWindow.h"

#define		LogWIND		130
#define		MaxLines	300

class LogWindow : public TEWindow
{
public:
	RGBColor			curColor;	// 6/21/95
	short				inMiddle;
	short				logFile;

	LogWindow();
	~LogWindow();

	void LogString(char *str);
	void SetLogColor(RGBColor *color);

	// Overrides
	void ProcessCommand(short theMenu, short theItem);
	void AdjustMenus();
	void HandleClick(Point where, EventRecord *theEvent);
};

extern LogWindow *gLogWin;

#endif