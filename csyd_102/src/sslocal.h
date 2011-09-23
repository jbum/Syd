// DeckLocal.h

// Machine Specific Code

PicHandle	GetPictureFromFile(StringPtr fName);
void RefreshDeckMan();
void DrawTableFrame(Rect *inFrame);

extern CCrsrHandle	openHand,closedHand;

#define malloc		NewPtrClear
#define free		DisposePtr