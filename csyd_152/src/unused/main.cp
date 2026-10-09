#include "SS.h"
#include "MainWin.h"
#include "MacEvents.h"
#include "DeckSounds.h"
#include "LogWindow.h"

#if __profile__
#include <profiler.h>		// MW Profiling
#endif


MainWindow	*mw;

int 		debugLevel=1;					// Set by Menu
Boolean		gQuitFlag=false, gSuspended = false;
SysEnvRec	gSysEnv;

void ShowVersionNumber()
{
	LogMessage("Syd Vers 1.0.6\r");
	LogMessage("by Jim Bumgardner (jbum@jbum.com)\r\r");
}

void main(int argc, char **argv)
{

	// Standard Mac Initialization
	MyInitMacintosh();

	// Make sure we're legal...
	MyCheckEnvironment();

	// Install Apple Event Handler
	InitAppleEvents();

	// Set up the menu bar
	MySetUpMenus();

	new LogWindow();
	ShowVersionNumber();
	LogMessage("Initializing Sounds...\r");

	InitSounds();

	LogMessage("Initializing WaveTables...\r");
	InitWaveTables();

	// Start Profiling
#if __profile__
	ProfilerInit(collectDetailed,bestTimeBase,20,5);
	ProfilerSetStatus(false);
#endif

	LogMessage("Done Initialization\r");

	// main event loop
	while (!gQuitFlag)			// Till the End of Time...
		MyHandleEvent();		// Get an Event, do something about it

#if __profile__
	ProfilerDump("\pSyd.profile");
	ProfilerTerm();
#endif
}