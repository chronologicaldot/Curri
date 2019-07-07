// (C) 2018 Nicolaus Anderson
#include <irrlicht.h>
#include "App.h"

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
// Suppress console creation (after debugging)
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif // _MSC_VER

int main( int argc, const char* argv[] ) {
	App app;
	app.parseArgs(argc, argv);
	return app.run();
}
