// ObjectWindow.h

#ifndef _H_ObjectWindow
#define _H_ObjectWindow	1

#define ObjectWindowID	1000
#define SBarSize		16

#if THINK_C
#define WindowPtrToObj(wp)	((ObjectWindow *) ((long) wp-4))
#else
#define WindowPtrToObj(wp)	((ObjectWindow *) wp)
#endif



class ObjectWindow {
public:
	CWindowRecord	theWin;
	Boolean			floating,active,activeClick;

	ObjectWindow(short resID, Boolean isFloating);
	virtual ~ObjectWindow();

	// virtual void	Dispose(void);
	virtual void	Update(void);
	virtual void	Refresh(void);
	virtual void	Activate(Boolean active);
	virtual void	HandleClick(Point, EventRecord *theEvent);
	virtual void	Draw(Rect *rr);
	virtual void	Idle(EventRecord *theEvent);
	// virtual Boolean	Save(SFReply *);
	virtual void	ProcessKey(EventRecord *theEvent);
	virtual void	Resize(short w, short h);
	virtual Boolean	ConfirmClose();
	virtual void	ProcessCommand(short theMenu, short theItem);
	virtual void	AdjustMenus();
	virtual void	BringWindowToFront();
};

/*
WindowPtr FrontDocument(void);
WindowPtr FrontFloating(void);
void SelectDocument(WindowPtr theWindow);
void SelectFloating(WindowPtr theWindow);
void LocateWindows(void);
*/

#endif