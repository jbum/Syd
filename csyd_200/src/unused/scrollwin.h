#ifndef _H_ScrollWin
#define _H_ScrollWin	1

#include <QDOffscreen.h>
#include <sound.h>

#include "ObjectWindow.h"

// #define SBarSize		15
#define GrowIconSize	14

class ScrollWindow : public ObjectWindow
{
public:
	ControlHandle	vScrollBar,hScrollBar;
	Point			scrollOffset;
	Point			nbrUnits,unitsPerScreen,unitHeight;

	ScrollWindow(short winID, Boolean vertScroll, Boolean horizScroll, Boolean isFloating);
	~ScrollWindow();

	// Internals
	void		SetupScrollBars(Boolean horiz, Boolean vert);
	void		AdjustScrollBars(Boolean resizeFlag);
	Boolean		HandleControlClick(Point where);
	void		ScrollToPosition(Point newPos);
	void		AutoScroll( Point pos);
	void 		SetScrollUnits(Point p);
	// void 		PrintWindow();

	// Local Overrides
	virtual void	ComputeUnitsPerScreen();
	virtual void	ComputeUnits();
	virtual void	UpdateToScrollPosition();

	// Overrides
	void 		Resize(short w, short h);
	void		Activate(Boolean activeFlag);

};


#endif