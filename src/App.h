// Copyright 2018-2019 Nicolaus Anderson

#ifndef APP_H
#define APP_H

#include <irrlicht.h>
#include <Copper.h>
#include <cubr_mfrunner.h>

namespace cubr {
	class CuBridge;
	class EventHandler;
}

using namespace irr;

/*
	Application

	Handles the program loop and creates an interface for Copper.
	Methods for the programmer to implement are at the bottom.
*/
class App : public irr::IEventReceiver {
protected:
	IrrlichtDevice*  device;
	video::IVideoDriver*  videoDriver;
	gui::IGUIEnvironment*  guiEnvironment;
	scene::ISceneManager*  sceneManager;
	Cu::Engine  copperEngine;
	cubr::CuBridge*  copperBridge;
	cubr::EventHandler*  copperEventHandler;
	cubr::MultifileRunner  copperFileRunner;
	core::dimension2du  screenSize;
	video::E_DRIVER_TYPE  videoDriverType;

	int  exitReturnValue;
	bool  pauseRendering;
	bool  pauseDeviceWhenWindowInactive;

	// For awaiting command-line argument values
	enum {
		CL_AWAIT_NONE=0,
		CL_AWAIT_SCREEN_SIZE,
		CL_AWAIT_DRIVER_TYPE,
		CL_AWAIT_FORCE32BIT = 0x7fffffff // Not a type
	} commandLineValueWait;

public:
	App();
	~App();
	void parseArgs( int argc, const char* argv[] );
	int run();
	IrrlichtDevice*  getDevice(); // Only works during run()
	Cu::Engine&  getCopperEngine();

	// Irrlicht
	virtual bool OnEvent(const SEvent&);

	// Public for Copper
	Cu::ForeignFunc::Result  closeApp( Cu::FFIServices& );

	/*
		TODO:
		Information from a "print()" foreign function could be siphoned to a pre-existing
		GUI text box that you have the power to adjust and display (but not delete) within Copper.
		This would give the option of displaying errors too.
	*/

protected:
	bool parseAndSetScreenSize( const core::stringc& );
	bool parseAndSetDriverType( const core::stringc& );
	void checkFileRunnerErrorFlags();
	void defaultAdjustSkin();

	// Copper and GUI Helpers
	bool areSameElement( gui::IGUIElement*, gui::IGUIElement* );


	//**** IMPLEMENT THESE ****

		//! Called for arguments given on the command line. Return "false" to stop parsing args.
	virtual bool parseCommandArg( const core::stringc& );

		//! Called by parseAndSetScreenSize() when all options failed.
	virtual core::dimension2du  initialScreenSize();

		//! Called in run() after all of the main components (e.g. the IrrlichtDevice) have been initialized.
	virtual void init();

		//! Called during the main loop when the rendering is paused
	virtual void onRenderPause();

		//! Called during main loop when the video driver has begun its rendering
	virtual void drawAll();

		//! Called when the Copper Bridge EventHandler did not process the event.
	virtual bool onExtraEvent(const SEvent&);
};

#endif
