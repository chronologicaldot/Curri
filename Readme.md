# Curri - Boilerplate Application Code

Curri is basic boilerplate code needed for creating applications using [Irrlicht](http://irrlicht.sourceforge.net/) and [Copper](github.com/chronologicaldot/CopperLang). It utilizes Cupric Bridge to create the GUI elements, but it provides the basic setup, including script loading, font loading, error logging, command-line argument parsing, and application exit.

## Usage

Create a copy of this project's contents for your application, then either modify App or create a class that inherits App and implement its virtual functions. You'll need to change the main.cpp to run your class if you do the latter.

A logging class (AppLogger) is provided separate from the App class. To incorporate it, simply create an instance of it as a class member of your main application and pass in a reference to it to the Copper engine via Cu::Engine::setLogger( Cu::Logger* ).

### Irrlicht Setup

You will need to Irrlicht 1.8 dev (called 1.9) from the SVN repository. Download from the SVN repository with:
```bash
svn checkout svn://svn.code.sf.net/p/irrlicht/code/trunk irrlicht-trunk
```

### Copper Engine and Cupric Bridge Setup

Copper and Cupric Bridge are expected to be placed in directories side-by-side with the Curri outermost directory. However, you can plop them into your project just about anywhere since they aren't compiled. Just remember to set the include path for GCC so it can find them.

### Copper Scripts Setup

Copper files are loaded from the "scripts" directory. The first script loaded is named "project.cu". Within this folder are import() calls to include other files into the project. Once project.cu has been processed, the other files that have been included will be processed in the order they have been imported. Within these other files, the function require() can be called with a file name to require that such a file have been imported first. In the case that it wasn't, processing is halted.

## Requirements
- [Irrlicht](http://irrlicht.sourceforge.net/)
- [Copper Interpreter](github.com/chronologicaldot/CopperLang)
- CupricBridge
- Freetype2

### Requirements for Irrlicht

There are VisualStudio projects for Windows users.

### Linux build

The OpenGL libraries
```bash
$ sudo apt-get install freeglut3 freeglut3-dev freeglut3-dbg"
```
(Note: This will install the entire GLUT development files. If you use Debian you only need libgl1-mesa)

Misc
```bash
$ sudo apt-get install build-essential xserver-xorg-dev x11proto-xf86vidmode-dev libxxf86vm-dev mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev libxext-dev libxcursor-dev
```

"make" instructions are in the MakeFile in the source/Irrlicht directory, but it's as simple as:
```bash
$ make
$ make sharedlib
$ sudo make install
```

## License

Please see license.txt.

The font "Exo2-LightExpanded.otf" is copyright [Natanael Gama](http://www.ndiscovered.com/).
The font "Rounded Elegance.ttf" is copyright Douglas Charles Cunha.
