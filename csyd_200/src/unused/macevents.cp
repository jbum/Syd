// MacEvents.cp
#include "SS.h"
#include "MacEvents.h"
#include "ObjectWindow.h"
#include "MainWin.h"

// #include "MainWin.h"

#include <GestaltEqu.h>
#include <AppleEvents.h>

// extern BoardWindow	*mw;

// Standard Macintosh Initialization
extern SysEnvRec	gSysEnv;

void MyInitMacintosh(void)
{
	MaxApplZone();
	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

void MyCheckEnvironment(void)
{
	OSErr	oe;
	short	eID = 0;
	static		char *eStr[7] = 
				{"This Mac is ancient history - sorry!",
				 "This Program needs a Color Macintosh - sorry!",
				 "This Program needs 32-bit Quickdraw - sorry!"};
	long		gResp,gestaltAnswer;

	oe = SysEnvirons(curSysEnvVers,&gSysEnv);
	if (eID != noErr) {
		eID = 1;
		goto ErrorExit;
	}
	if (!gSysEnv.hasColorQD) {
		eID = 2;
		goto ErrorExit;
	}
	oe = Gestalt(gestaltVersion, &gestaltAnswer);
	if (!gestaltAnswer)
		goto ErrorExit;
	oe = Gestalt(gestaltQuickdrawVersion,&gResp);
	if (gResp < gestalt32BitQD) {
		eID = 3;
		goto ErrorExit;
	}

	// if (gSysEnv.processor >= 1 && gSysEnv.processor <= 4)
	//	gSlowMachine = true;


	return;
ErrorExit:
	ErrorExit(eStr[eID-1]);
}

// The Main Event Dispatcher - this routine should be called repeatedly

void MyHandleEvent(void)

{
	EventRecord	theEvent;
	Boolean		ok;

	ok = WaitNextEvent(everyEvent,&theEvent,0L,NULL);

	if (ok) {
		// Handle the Event
		MyDoEvent(&theEvent);
	}
	else
		// Nothing happened, kick back...
		IdleObjects(&theEvent);
}


void MyDoEvent(EventRecord *theEvent)
{
	short 		windowCode;
	WindowPtr	theWindow;

	switch (theEvent->what) {
	//
	// Was the mouse button pressed?
	case mouseDown:
		// Find out where the mouse went down
		windowCode = FindWindow (theEvent->where, &theWindow);

	  	switch (windowCode) {
		case inSysWindow: 	// Desk Accessory?
		    SystemClick (theEvent, theWindow);
		    break;
		    
		case inMenuBar:		// Menu Bar?
		  	MyAdjustMenus();
		    MyHandleMenu(MenuSelect(theEvent->where));
		    break;

		default:			// Cursor was inside our window
			// If the window isn't in the front
			if (theWindow != FrontWindow()) {
				// Make it so...
				SelectWindow(theWindow);
				if (((WindowPeek) theWindow)->refCon != ObjectWindowID ||
					WindowPtrToObj(theWindow)->activeClick)
					return;
			}
			// Window is already in the front, handle the click
			switch (windowCode) {

			case inContent:		// Content area?
				if (((WindowPeek) theWindow)->refCon == ObjectWindowID)
					WindowPtrToObj(theWindow)->HandleClick(theEvent->where, theEvent);
				break;

			case inDrag:		// Dragbar?
				{
					Rect	dragRect;
					dragRect = qd.screenBits.bounds;
					// Handle the dragging of the window
					DragWindow(theWindow, theEvent->where, &dragRect);
				}
				break;

			 case inGoAway:						// close box?
				// Only Grid Windows can be closed
			  	if (((WindowPeek) theWindow)->refCon == ObjectWindowID) {

					// Handle the mouse tracking for the close box
			  		if (TrackGoAway(theWindow, theEvent->where))
						// If mouse is released inside the close box
						// Hide or close the window
						if (WindowPtrToObj(theWindow)->ConfirmClose()) {
							// WindowPtrToObj(theWindow)->Dispose();
							delete(WindowPtrToObj(theWindow));
						}
				}
			  	break;

			case inGrow:						// Grow box?
				{
					long	growResult;
					Rect	growRect;

					SelectWindow(theWindow);

					SetRect(&growRect,20,20,
							qd.screenBits.bounds.right,qd.screenBits.bounds.bottom);

					// Handle the mouse tracking for the resizing
					growResult = GrowWindow(theWindow,theEvent->where,&growRect);

					if (((WindowPeek) theWindow)->refCon == ObjectWindowID)
						WindowPtrToObj(theWindow)->Resize(LoWord(growResult),HiWord(growResult));
				}
				break;
			}
			break;
		}
		break;
		
	// Was a key pressed?
	case keyDown: 
	case autoKey:
		// Was the cmd-key being held down?  If so, process menu bar short cuts.
	    if ((theEvent->modifiers & cmdKey) != 0) {
	      MyAdjustMenus();
		  MyHandleMenu(MenuKey((char) (theEvent->message & charCodeMask)));
		}
		else {
			theWindow = FrontWindow();
			if (theWindow && ((WindowPeek) theWindow)->refCon == ObjectWindowID)
				WindowPtrToObj(theWindow)->ProcessKey(theEvent);
		}
		break;

	// Does a window need to be redrawn?
	case updateEvt:
		theWindow = (WindowPtr) theEvent->message;
		if (((WindowPeek) theWindow)->refCon == ObjectWindowID)
			WindowPtrToObj(theWindow)->Update();
	    break;

	// Has a window been activated or deactivated?
	case activateEvt:
		theWindow = (WindowPtr) theEvent->message;

		// Force it to be redrawn
		if (((WindowPeek) theWindow)->refCon == ObjectWindowID)
			WindowPtrToObj(theWindow)->Activate( theEvent->modifiers & 1);

		break;
	case osEvt:
		// Force it to be redrawn
		switch (theEvent->message >> 24) {
		case suspendResumeMessage:
			theWindow = FrontWindow();
			if (theWindow && ((WindowPeek) theWindow)->refCon == ObjectWindowID)
				WindowPtrToObj(theWindow)->Activate( (theEvent->message & resumeFlag) > 0);
			gSuspended = !((theEvent->message & resumeFlag) > 0);
			break;
		}
		break;
	case kHighLevelEvent:
		if (gSysEnv.systemVersion >= 0x0700)
			AEProcessAppleEvent(theEvent);
		break;
    }
}	    

// Do Idle Time Processing

void IdleObjects(EventRecord *theEvent)
{
	WindowPeek	theWin;
	theWin = (WindowPeek) FrontWindow();
	while (theWin) {
		if (((WindowPeek) theWin)->refCon == ObjectWindowID)
			WindowPtrToObj(theWin)->Idle(theEvent);
		theWin = theWin->nextWindow;
	}
}


Boolean GotRequiredParams(AppleEvent *theEvent)
{
   DescType returnedType;
   Size 	actualSize;
   OSErr	err;
   err = AEGetAttributePtr ( theEvent, keyMissedKeywordAttr, 
						typeWildCard, &returnedType, NULL, 0, 
						&actualSize);
   
   return err == errAEDescNotFound;
   
 }	/* CAppleEvent::GotRequiredParams */


void DoOpenEvent(AppleEvent *theEvent)
{
	Handle		docList = NULL;
	long		i;
	FSSpec		myFSS;
	AEDescList	theList;
	AEKeyword	aeKeyword=keyDirectObject;
	long		itemCount;
	DescType	actualType;
	Size		actualSize;
	OSErr		oe;

	if ((oe = AEGetParamDesc( theEvent, keyDirectObject, typeAEList, &theList)) != noErr) {
		DebugStr("\pAEGetParamDesc");
		return;
	}


	if (!GotRequiredParams(theEvent)) {
		DebugStr("\pGotRequiredParams");
		return;
	}

	if ((oe = AECountItems( &theList, &itemCount)) != noErr) {
		DebugStr("\pAECountItems");
		return;
	}


	for (i = 1; i <= itemCount; i++)
	{
		oe = AEGetNthPtr( &theList, i, typeFSS, &aeKeyword, &actualType,
						(Ptr) &myFSS, sizeof( FSSpec), &actualSize);

		if (oe == noErr) {
			MainWindow	*oldMW = mw;
			// Open file based on myFSS
			mw = new MainWindow();
			if (mw)
				mw->Open(&myFSS);
			else
				mw = oldMW;
		}
	}
	AEDisposeDesc(&theList);
	// event was handled successfully
}

pascal OSErr AppleEventHandler(AppleEvent *theEvent,AppleEvent *reply, long refCon)
{	
	DescType	actualType;
	Size		actualSize;
	DescType	eventClass, eventID;
	OSErr		oe;

	if ((oe = AEGetAttributePtr( (AppleEvent*) theEvent, keyEventClassAttr,
					typeType, &actualType, (Ptr) &eventClass, 
					sizeof(eventClass), &actualSize)) != noErr)
			return oe;
							
	
	if ((oe = AEGetAttributePtr(  (AppleEvent*) theEvent, keyEventIDAttr,
					typeType, &actualType, (Ptr) &eventID, 
					sizeof(eventID), &actualSize)) != noErr)
			return oe;
									
	if (eventClass == kCoreEventClass)
	{
		switch (eventID)
		{
		case kAEOpenApplication:
			if (GotRequiredParams(theEvent))
			{
				mw = new(MainWindow);
			}
			break;
				
		case kAEOpenDocuments:
			DoOpenEvent( theEvent);
			break;
				
		case kAEPrintDocuments:
			break;
			
		case kAEQuitApplication:
			if (GotRequiredParams(theEvent))
			{
				gQuitFlag = true;
			}
			break;
		}		
	}
		
	return noErr;
}

Boolean CloseAllWindows()
{
	WindowPeek	theWin;
	while ((theWin = (WindowPeek) FrontWindow()) != NULL) {
		if (theWin->refCon == ObjectWindowID) {
			if (WindowPtrToObj(theWin)->ConfirmClose())
				delete(WindowPtrToObj(theWin));
			else
				return false;
		}
		else
			ErrorExit("Invalid Window Type");
	}
	return true;
}

AEEventHandlerUPP	aeEventHandler;

void InitAppleEvents(void)
{
	if (gSysEnv.systemVersion >= 0x0700) {
	aeEventHandler = NewAEEventHandlerProc(AppleEventHandler);
	AEInstallEventHandler(typeWildCard, typeWildCard,
								aeEventHandler,
								0,FALSE);
	}
}