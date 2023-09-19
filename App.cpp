#include "App.h"

#include "MainWindow.h"
#include <iostream>

#include "FortuneFunctions.h"

App::App(void)
	:	BApplication("application/x-vnd.dw-TestApp")
{
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
