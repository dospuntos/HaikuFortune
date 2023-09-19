#include "App.h"

#include <FindDirectory.h>
#include <OS.h>
#include <Path.h>
#include <stdlib.h>

#include "FortuneFunctions.h"
#include "MainWindow.h"

App::App(void)
	:	BApplication("application/x-vnd.dpd-HaikuFortune")
{
	BPath path;
	
	// We have to use an #ifdef here because the fortune files under R5
	// and Zeta are in the system/etc/ directory, but in Haiku they're
	// kept in the system/data directory. We detect the platform by using
	// a compiler definition. __HAIKU__ is defined under Haiku, but not BeOS
	// R5 or Zeta. Zeta has its own __ZETA__ definition. All three have the
	// __BEOS__ definition which, in this case, isn't at all useful.
	#ifdef __HAIKU__
	find_directory(B_SYSTEM_DATA_DIRECTORY,&path);
	#else
	find_directory(B_BEOS_ETC_DIRECTORY,&path);
	#endif
	
	path.Append("fortunes");
	gFortunePath = path.Path();
	
	// If we want the rand() function to actually be pretty close to random
	// we will need to seed the random number generator with the time. If we
	// don't, we will get the same "random" numbers each time the program is
	// run. rand() isn't really random, but this makes it close enough for us.
	srand(system_time());
	
	MainWindow *mainwin = new MainWindow();
	mainwin->Show();
}


int
main(void)
{
	App *app = new App();
	app->Run();
	delete app;
	
	return 0;
}
