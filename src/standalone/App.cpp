// Copyright 2018-2019 Nicolaus Anderson

#include <cubridge.h>
#include <cubr_event.h>
#include "App.h"

App::App()
	: device(nullptr)
	, videoDriver(nullptr)
	, guiEnvironment(nullptr)
	, sceneManager(nullptr)
	, copperEngine()
	, copperBridge(nullptr)
	, copperEventHandler(new cubr::EventHandler(copperEngine))
	, copperFileRunner(copperEngine)
	, screenSize(1000,600)
	, videoDriverType( video::EDT_OPENGL )
	, exitReturnValue(0)
	, pauseRendering(false)
	, pauseDeviceWhenWindowInactive(false)
	, commandLineValueWait(CL_AWAIT_NONE)
{
	copperEngine.setIgnoreBadForeignFunctionCalls(false);
	copperFileRunner.setRootDirectoryPath("./scripts");

	// Foreign methods this app provides
	Cu::addForeignMethodInstance<App>(copperEngine, "close_application", this, &App::closeApp);
}

App::~App() {

	if ( copperEventHandler ) {
		delete copperEventHandler;
		copperEventHandler = nullptr;
	}

	if ( copperBridge ) {
		delete copperBridge;
		copperBridge = nullptr;
	}
}

void
App::parseArgs( int argc, const char* argv[] ) {
	int a = 0;
	for (; a < argc; ++a)
		if ( ! parseCommandArg( core::stringc(argv[a]) ) )
			break;
}

bool
App::parseCommandArg( const core::stringc&  arg ) {
	// Useful for special app settings such as startup screen-size or font choice or video driver or testing
	switch ( commandLineValueWait )
	{
	case CL_AWAIT_NONE: break;

	case CL_AWAIT_SCREEN_SIZE:
		return parseAndSetScreenSize(arg);

	case CL_AWAIT_DRIVER_TYPE:
		return parseAndSetDriverType(arg);

	default: break;
	}

	if ( arg == "--screen-size" ) {
		commandLineValueWait = CL_AWAIT_SCREEN_SIZE;
	}
	else if ( arg == "--driver" ) {
		commandLineValueWait = CL_AWAIT_DRIVER_TYPE;
	}
	else if ( arg == "--inactive-pause" ) {
		pauseDeviceWhenWindowInactive = true;
	}
	return true;
}

int
App::run() {
	device = irr::createDevice(videoDriverType, screenSize);

	if ( !device )
		return 1;

	videoDriver = device->getVideoDriver();
	guiEnvironment = device->getGUIEnvironment();
	sceneManager = device->getSceneManager();

	// Copper Bridge  (destroyed in ~App)
	if ( copperBridge ) {
		copperBridge->setGUIEnvironment(guiEnvironment);
	} else {
		copperBridge = new cubr::CuBridge(copperEngine, guiEnvironment, nullptr);
	}

	device->setEventReceiver(this);

	init();

	// Copper loop
	if ( copperFileRunner.run("project.cu") )
	{
		// Irrlicht loop
		while ( device->run() ) {
			if ( ! device->isWindowActive() && pauseDeviceWhenWindowInactive ) {
				device->yield();
			} else {
				if ( pauseRendering ) {
					onRenderPause();
				} else {
					videoDriver->beginScene();
					drawAll();
					videoDriver->endScene();
				}
			}
		}
	}
	else {
		checkFileRunnerErrorFlags();
	}

	device->setEventReceiver(0); // Note: Delinking unnecessary since no pointer is saved

	//device->run(); // Digest close

	// Causes a segmentation fault when there is an error in the Copper multi-file runner.
	// Perhaps some GUI element hasn't been properly reference counted.
	device->drop(); // Destroys GUI with it
	device = nullptr;

	return exitReturnValue;
}

IrrlichtDevice*
App::getDevice() {
	return device;
}

Cu::Engine&
App::getCopperEngine() {
	return copperEngine;
}

bool
App::OnEvent(const SEvent&  event) {
	bool result = false;

	if ( copperEventHandler )
		result = copperEventHandler->OnEvent(event);

	if ( !result )
		return OnExtraEvent(event);
}

Cu::ForeignFunc::Result
App::closeApp( Cu::FFIServices&  ffi ) {
	if ( device )
		device->closeDevice();

	if ( ffi.getArgCount() == 1 && ffi.demandArgType(0, Cu::ObjectType::Integer) ) {
		exitReturnValue = ((Cu::IntegerObject&)ffi.arg(0)).getIntegerValue();
	}

	return Cu::ForeignFunc::EXIT;
}

bool
App::parseAndSetScreenSize( const core::stringc&  arg ) {
	if ( arg == "small" ) {
		screenSize = core::dimension2du(300,200);
	}
	else if ( arg == "medium" ) {
		screenSize = core::dimension2du(800,600);
	}
	else if ( arg == "large" ) {
		screenSize = core::dimension2du(1024,768);
	}
	else if ( arg == "xlarge" ) {
		screenSize = core::dimension2du(1200, 900);
	}
	else {
		screenSize = initialScreenSize();
	}
	commandLineValueWait = CL_AWAIT_NONE;
	return true;
}

bool
App::parseAndSetDriverType( const core::stringc&  arg ) {
	if ( arg == "burnings" ) {
		videoDriverType = video::EDT_BURNINGSVIDEO;
	} else {
		videoDriverType = video::EDT_OPENGL;
	}
	commandLineValueWait = CL_AWAIT_NONE;
	return true;
}

void
App::checkFileRunnerErrorFlags() {
	
	cubr::MultifileRunner::ErrorFlags  errorFlag = copperFileRunner.getErrorFlags();

	if ( errorFlag == cubr::MultifileRunner::ERROR_NONE )
		return;

	if ( errorFlag == cubr::MultifileRunner::ERROR_FILE_NONEXISTENT
		|| errorFlag == cubr::MultifileRunner::ERROR_ENGINE_ERROR
		|| errorFlag == cubr::MultifileRunner::ERROR_IMPORT_FAILED
		|| errorFlag == cubr::MultifileRunner::ERROR_REQUIRE_FAILED
	) {
		exitReturnValue = 1;
	}
}

void
App::defaultAdjustSkin() { // Default implementation

	gui::IGUISkin* skin = guiEnvironment->getSkin();
	gui::IGUIFont* font = guiEnvironment->getFont( "font/sansfont.xml" ); // TODO: Set font

	if ( !skin )
		return;

	if ( font )
		skin->setFont( font );

	skin->setColor(gui::EGDC_BUTTON_TEXT, video::SColor(255,255,255,255));
	skin->setColor(gui::EGDC_GRAY_TEXT, video::SColor(255,66,66,66));
	skin->setColor(gui::EGDC_TOOLTIP, video::SColor(255,240,240,100));
	skin->setColor(gui::EGDC_TOOLTIP_BACKGROUND, video::SColor(255,50,60,150));

	u32  c=0;
	video::SColor  color;
	s32  defaultColorDrop = 0;
	s32  colorDrop = defaultColorDrop;
	for(; c < gui::EGDF_COUNT; ++c) {
		color = skin->getColor( (gui::EGUI_DEFAULT_COLOR)c );
		color.setAlpha(255);
		switch( (irr::gui::EGUI_DEFAULT_COLOR)c ) {
			case irr::gui::EGDC_3D_FACE:
				colorDrop = 50;
				break;

			//case irr::gui::EGDC_3D_SHADOW:
			case irr::gui::EGDC_3D_DARK_SHADOW:
				colorDrop = -10;
				break;

			default:
				colorDrop = defaultColorDrop;
				break;
		}
		color.setRed( (u32) core::clamp((s32)color.getRed() - colorDrop, 0, 255) );
		color.setGreen( (u32) core::clamp((s32)color.getGreen() - colorDrop, 0, 255) );
		color.setBlue( (u32) core::clamp((s32)color.getBlue() - colorDrop, 0, 255) );
		skin->setColor( (gui::EGUI_DEFAULT_COLOR)c, color );
	}
}

bool
App::areSameElement( gui::IGUIElement* e1, gui::IGUIElement* e2 ) {
	return ( e1 != nullptr && e2 != nullptr && e1 == e2 );
}

core::dimension2du
App::initialScreenSize() {
	return core::dimension2du(1024, 768);
}

void
App::init() { // Default implementation
	// TODO: This method is meant for adding other initialization code prior to running the main loops.
	// It is safe to use:
	// device, videoDriver, guiEnvironment, copperEngine, copperBridge, copperEventHandler, copperFileRunner.

	defaultAdjustSkin();
}

void
App::onRenderPause() {

}

void
App::drawAll() {
	sceneManager->drawAll();
	guiEnvironment->drawAll();
}

bool
App::OnExtraEvent(const SEvent&) {
	return false;
}
