// GraphWin.h
#ifndef _H_GraphWin
#define _H_GraphWin	1

#include <QDOffscreen.h>

#include "ObjectWindow.h"

#define GraphWindowID		129
#define GraphWidth			256
#define GraphHeight			64
class SSStorage;

class GraphWindow : ObjectWindow
{
	// Rendering stuff
	GWorldPtr			offWorld;
	PixMapHandle		offMap;
	long				nbrSamples;
	ssfloat				minPt, maxPt;	// Normalized zooming info
	SSStorage			*storageMod;

public:
	GraphWindow();
	~GraphWindow();

	void			InitGraphics();
	void			RefreshRect(Rect *rr);
	void			Paint(Rect *rr);
	void			FullRefresh(void);
	void			PartialRefresh(Rect *rr);
	void			GraphSamples(SSStorage *sm, long nbrFrames);
	void			Regraph();
	void			ScrollLeft();
	void			ScrollRight();
	// Overrides
	void			Dispose(void);
	void			Draw(Rect *rr);
	void			HandleClick(Point where, EventRecord *theEvent);
	void			ProcessKey(EventRecord *er);

};

extern GraphWindow *gw;

#endif