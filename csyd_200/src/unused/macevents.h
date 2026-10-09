void MyInitMacintosh(void);
void MyCheckEnvironment(void);
void InitAppleEvents(void);
void MyHandleEvent(void);
void MySetUpMenus(void);
void MyDoEvent(EventRecord *theEvent);
void IdleObjects(EventRecord *theEvent);
void MyAdjustMenus(void);
void MyHandleMenu(long menuSelect);
Boolean CloseAllWindows();

extern SysEnvRec	gSysEnv;

Boolean GotRequiredParams(AppleEvent *theEvent);
void DoOpenEvent(AppleEvent *theEvent);
pascal OSErr AppleEventHandler(AppleEvent *theEvent,AppleEvent *reply, long refCon);