// SSButtons.c

#include "SS.h"
#include "MainWin.h"
#include "DeckSounds.h"
#include "SSFolder.h"
#include "SSStorage.h"

#define GadgetIconHeight	24
#define GadgetIconWidth		24
#define GadgetTileHeight	26
#define GadgetTileWidth		26
#define GadgetXGap			1
#define GadgetYGap			1
#define GadgetTopMargin		8
#define GadgetLeftMargin	8

#define GadgetPanelHeight	GadgetIconHeight*2+GadgetYGap*4+GadgetTopMargin*2
#define MaxHorizGadgets		19


#define NbrButtons	3
Boolean	bActive[NbrButtons];

#define ButtonWidth		62
#define ButtonLM		12
#define ButtonGap		(ButtonWidth+10)
#define BHeight			18
#define BTop			314
#define BBot			BTop+BHeight
#define BL(n)			(ButtonLM+ButtonGap*((n)-1))
#define BR(n)			(ButtonLM+ButtonGap*((n)-1)+ButtonWidth)
#define HelpLeft		BL(NbrButtons+1)

Rect	bRect[NbrButtons] = {
	BTop, BL(1), 	BBot, BR(1),
	BTop, BL(2), 	BBot, BR(2),
	BTop, BL(3),	BBot, BR(3)};

StringPtr	bText[NbrButtons] = {
	"\pSynthesize",
	"\pPlay",
	"\pStop"};

void MainWindow::GadgetIdle(Point p)
{
	int	n;
	SetCursor(*cursors[CT_OpenHand]);
	p.h -= GadgetLeftMargin;
	p.v -= GadgetTopMargin;

	if (p.h < 0 || p.h >= GadgetTileWidth*MaxHorizGadgets ||
		p.v < 0 || p.v >= GadgetTileHeight*(1 + MT_NbrModules/MaxHorizGadgets))
		helpPtr = NULL;
	else {
		n = p.h/GadgetTileWidth + p.v/GadgetTileHeight*MaxHorizGadgets;
		if (n >= 0 && n < MT_NbrModules)
			helpPtr = mainInst->GetModuleDesc(n);
		else
			helpPtr = NULL;
	}
	SetHelp(helpPtr);
}

void MainWindow::DrawWindowButton(short i, Boolean state)
{
	RGBColor	fc;
	Rect		r;
	if (state == 0) {
		if (bActive[i]) {
			fc.red = fc.blue = fc.green = 0xCCCC;
		}
		else {
			fc.red = fc.blue = fc.green = 0xAAAA;
		}
	}
	else {
		fc.red = fc.blue = fc.green = 0xFFFF;
	}
	RGBForeColor(&fc);
	r = bRect[i];
	DrawTableFrame(&bRect[i], false);
	InsetRect(&r,1,1);
	DrawTableFrame(&bRect[i], false);
	InsetRect(&r,1,1);
	PaintRect(&r);

	fc.red = fc.blue = fc.green = 0x0000;
	RGBForeColor(&fc);
	MoveTo(bRect[i].left + ((bRect[i].right-bRect[i].left)-StringWidth(bText[i]))/2,
		   bRect[i].top + ((bRect[i].bottom-bRect[i].top)/2+3));
	DrawString(bText[i]);
}

void MainWindow::DrawWindowButtons(Rect *rr)
{
	short		i;
	Rect		dmy;
	for (i = 0; i < NbrButtons; ++i) {
		if (SectRect(rr,&bRect[i],&dmy))
			DrawWindowButton(i,0);
	}
}

void MainWindow::SetActiveButtons()
{
	short	i;
	GrafPtr	theWin = (WindowPtr) &this->theWin;

	SetPort(theWin);

	for (i = 0; i < NbrButtons; ++i)
		bActive[i] = 1;
	DrawWindowButtons(&theWin->portRect);
}

void MainWindow::PushButton(short buttonNbr)
{
	DrawWindowButton(buttonNbr,true);
	switch (buttonNbr) {
	case 0:	// Synthesize();
		if (windowState != WS_Synthesize) {
			storageMod->StopPlayback();
			PerformSynthesis();
		}
		break;
	case 1:	// Play();
		if (windowState != WS_Synthesize) {
			storageMod->StopPlayback();
			PlayBuffer();
		}
		break;
	case 2: // Stop();
		if (windowState == WS_Synthesize)
			CleanUpSynthesis();
		storageMod->StopPlayback();
		break;
	}
	DrawWindowButton(buttonNbr,false);
	SetActiveButtons();
}

Boolean MainWindow::HandleButtonClick(Point p)
{
	int		i;
	Boolean	inButton;
	for (i = 0; i < NbrButtons; ++i) {
		if (PtInRect(p, &bRect[i]) && bActive[i]) {
			DrawWindowButton(i,1);
			inButton = true;
			while (WaitMouseUp()) {
				GetMouse(&p);
				if (PtInRect(p, &bRect[i]) != inButton) {
					inButton = !inButton;
					DrawWindowButton(i,inButton);
				}
			}
			if (inButton) {
				PushButton(i);
			}
			return true;
		}
	}
	return false;
}


// Gadget Panel stuff
void MainWindow::InitGadgetPanel()
{
	GrafPtr	theWin = (WindowPtr) &this->theWin;

	SetRect(&gpRect,0,0,theWin->portRect.right,GadgetPanelHeight);


	SetRect(&helpRect,HelpLeft,
					theWin->portRect.bottom-(BottomMargin+HelpPanelHeight+2),
					theWin->portRect.right-RightMargin,
					theWin->portRect.bottom-BottomMargin);
	helpPtr = NULL;
}

void MainWindow::SetHelp(StringPtr ptr)
{
	static StringPtr	lastHelpPtr;
	GrafPtr	theWin = (WindowPtr) &this->theWin;

	helpPtr = ptr;
	if (helpPtr != lastHelpPtr) {
		SetPort(theWin);
		lastHelpPtr = helpPtr;
		DrawGadgetPanel(&helpRect);
	}
}

void MainWindow::PrintToHelp(char *tmp,...)
{
	static char tbuf[128];
	GrafPtr	theWin = (WindowPtr) &this->theWin;

	va_list args;
	va_start(args,tmp);
	vsprintf(tbuf,tmp,args);
	va_end(args);
	
	CtoPstr(tbuf);
	helpPtr = (StringPtr) tbuf;
	SetPort(theWin);
	DrawGadgetPanel(&helpRect);
}

void MainWindow::DrawGadgetPanel(Rect *rr)
{
	Rect	dmy,r;
	int		i,x,y;
	// GrafPtr	theWin = (WindowPtr) &this->theWin;

	// SetPort(theWin);

	if (SectRect(rr,&gpRect,&dmy)) 
	{
		r = gpRect;
		InsetRect(&r,4,4);
		DrawTableFrame(&r, true);
		InsetRect(&r,1,1);
		DrawTableFrame(&r, false);

		for (i = 0; i < MT_NbrModules; ++i) {
			x = GadgetLeftMargin + GadgetTileWidth*(i%MaxHorizGadgets);
			y = GadgetTopMargin + GadgetTileHeight*(i/MaxHorizGadgets);
			SetRect(&r,x,y,x+GadgetIconWidth,y+GadgetIconHeight);
			OffsetRect(&r,GadgetXGap,GadgetYGap);
			if (mDesc[i].itsSmallIcon)
				PlotCIcon(&r,mDesc[i].itsSmallIcon);
			else if (mDesc[i].itsIcon)
				PlotCIcon(&r,mDesc[i].itsIcon);
			// InsetRect(&r,-GadgetXGap,-GadgetYGap);
			// DrawTableFrame(&r, false);
		}
	}
	if (SectRect(rr,&helpRect,&dmy)) {
		Rect		r;
		RGBColor	fc;
		DrawTableFrame(&helpRect, true);
		r = helpRect;
		InsetRect(&r,1,1);
		fc.red = fc.blue = fc.green = 0x8888;
		RGBForeColor(&fc);
		PaintRect(&r);
		fc.red = fc.blue = fc.green = 0;
		RGBForeColor(&fc);
		if (helpPtr) {
			MoveTo(r.left + ((r.right-r.left)-StringWidth(helpPtr))/2,
			   r.top + ((r.bottom-r.top)/2+4));
			DrawString(helpPtr);
		}
	}
}


Boolean MainWindow::HandleGadgetPanelClick(Point p)
{
	short		nbr;
	SSModule	*nn;
	Rect		dmyRect;

	if (PtInRect(p, &gpRect)) {
		p.h -= GadgetLeftMargin;
		p.v -= GadgetTopMargin;

//			x = GadgetLeftMargin + GadgetTileWidth*(i%MaxHorizGadgets);
//			y = GadgetTopMargin + GadgetTileHeight*(i/MaxHorizGadgets);


		if (p.h < 0 || p.h >= GadgetTileWidth*MaxHorizGadgets ||
			p.v < 0 || p.v >= GadgetTileHeight*(1 + MT_NbrModules/MaxHorizGadgets))
			return false;
		nbr = p.h/GadgetTileWidth + p.v/GadgetTileHeight*MaxHorizGadgets;
		if (nbr < 0 || nbr >= MT_NbrModules)
			return false;
		mainInst->AddModule(nbr);
		if (mainInst->nbrModules == 0)
			return false;
		dirty = true;
		nn = mainInst->mods[mainInst->nbrModules-1];
		if (nn && nn->moduleType == nbr) {
			// OffsetRect(&nn->cellBounds,(p.h - nn->bounds.left)-16,
			//						(p.v - nn->bounds.top)-16);
			nn->ComputeBounds(nn->cellBounds.left+(p.h - nn->cellBounds.left)-16,
								nn->cellBounds.top+(p.v - nn->cellBounds.top)-16);
			DeselectAll();
			FullRefresh();
			nn->Select(true);
			DragModule(nn,p);
			if (SectRect(&nn->cellBounds,&gpRect,&dmyRect) && dmyRect.bottom-dmyRect.top > 16)
				Delete();

			return true;
		}
	}
	return false;
}

void	MainWindow::ProcessModuleClick(SSModule *nn, Point p, EventRecord *er)
{
	Rect		outputArea;
	static long	lastClick;
	Rect		dmyRect;

	if (!PtInRect(p,&nn->cellBounds))
		return;

	DeselectAll();

	if (er->when - lastClick < GetDblTime()) {
		if (nn->moduleType == MT_Folder && (er->modifiers & (optionKey | cmdKey)) > 0) {
			mw = new MainWindow();
			mw->Open( &((SSFolder *) this)->instFileSpec);
		}
		else if (nn->ProcessDoubleClick() == OK)
			dirty = true;
		return;
	}
	lastClick = er->when;

	outputArea = nn->cellBounds;
	outputArea.right += 4;
	outputArea.left = outputArea.right-7;
	dirty = true;

	if (PtInRect(p,&outputArea)) {
		DragNewLink(nn,p,0);
	}
	else {
		nn->Select(true);
		DragModule(nn,p);
		if (SectRect(&nn->cellBounds,&gpRect,&dmyRect) && dmyRect.bottom-dmyRect.top > 16)
			Delete();
	}
}

void MainWindow::DragModule(SSModule *nn, Point p)
{
	Rect	refreshRect,linksRect;
	Point	oldPt,lp;
	oldPt = lp = p;
	SetCursor(*cursors[CT_ClosedHand]);
	while (WaitMouseUp()) {
		GetMouse(&lp);
		if (!EqualPt(oldPt,lp)) {
			nn->ComputeLinksRect(&linksRect);
			nn->ComputeBounds(nn->cellBounds.left+lp.h - oldPt.h,
							  nn->cellBounds.top+lp.v - oldPt.v);
			// OffsetRect(&bounds,,);
			UnionRect(&linksRect,&nn->bounds,&refreshRect);
			PartialRefresh(&refreshRect);
			oldPt = lp;
		}
	}
}

// !!! Fix refreshing bug with this module

void MainWindow::DragNewLink(SSModule *nn, Point p, int suggestedType)
{
	Point		lp,anchor,oldPt;
	SSModule	*getSelect = NULL;
	LinkDescPtr	ld = nn->GetLinkDesc(suggestedType);
	RGBColor	blackColor={0,0,0};
	RGBColor	lineColor = {0x7777,0x7777,0x7777};

	// drag of output patch cord, if clicked on edge
	SetCursor(*cursors[CT_CrossHair]);

	nn->GetOutputDock(&anchor,p);
	oldPt = p;
	RGBForeColor(&lineColor);	// ld->color
	PenMode(patXor);
	PenSize(2,2);
	MoveTo(anchor.h,anchor.v);
	LineTo(p.h,p.v);
	while (WaitMouseUp()) {
		GetMouse(&lp);
		if (!EqualPt(lp,oldPt)) {
			LineTo(anchor.h,anchor.v);
			RGBForeColor(&blackColor);
			PenMode(patCopy);
			PenSize(1,1);
			// PenNormal();
			// Erased, reselect if need be
			if (getSelect && !PtInRect(lp,&getSelect->cellBounds)) {
				getSelect->Select(false);
				getSelect = NULL;
				SetCursor(*cursors[CT_CrossHair]);
			}

			getSelect = mainInst->GetPickedModule(lp);

			if (getSelect == nn)
				getSelect = NULL;

			if (getSelect != NULL) {
				getSelect->Select(true);
				SetCursor(*cursors[CT_PatchCord]);
			}
			// Unerase
			nn->GetOutputDock(&anchor,lp);
			PenMode(patXor);
			PenSize(2,2);
			RGBForeColor(&lineColor);	// ld->color
			MoveTo(anchor.h,anchor.v);
			LineTo(lp.h,lp.v);				
			oldPt = lp;
		}
	}
	LineTo(anchor.h,anchor.v);
	PenMode(patCopy);
	PenSize(1,1);
	RGBForeColor(&blackColor);
	if (getSelect) {
		getSelect->Select(false);
		if (getSelect != nn)
			// Set up patch relationship
			getSelect->AddInput(nn, suggestedType);
		dirty = true;
	}
	FullRefresh();
}

