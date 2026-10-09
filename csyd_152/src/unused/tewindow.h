// TEWindow.h

#ifndef _H_TEWin
#define _H_TEWin	1

#include <QDOffscreen.h>
#include <sound.h>

#include "ScrollWin.h"

class TEWindow : public ScrollWindow
{
public:
	TEHandle			teH;
	Boolean				dirty;

	TEWindow(short winID, Boolean floating);
	~TEWindow();

	// Scroll Overrides
	void UpdateToScrollPosition();

	// Overrides
	void Draw(Rect *rr);
	void HandleClick(Point where, EventRecord *theEvent);
	void ProcessKey(EventRecord *er);
	void Resize( short width, short height);
	void Idle(EventRecord *theEvent);
	void Activate(Boolean activeFlag);
	void AdjustMenus();
	void ProcessCommand(short theMenu, short theItem);
};


#endif