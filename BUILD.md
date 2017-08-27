How to build the code
=====================

Checkout the code from the git repository:
https://github.com/xLAva/JediOutcastLinux.git



Needed packages on Ubuntu 14.04 64bit:
--------------------------------------

I'm using a 64bit system, but this port is still a 32 bit program!

sudo apt-get install libsdl2-dev:i386 libglm-dev libopenal-dev:i386

sudo apt-get install cmake g++-multilib



Needed packages on Windows:
---------------------------

Visual Studio C++ (Express is enough)

Place this libraries in the 3rdparty folder:
* SDL2: https://www.libsdl.org/release/SDL2-2.0.3-win32-x86.zip
* GLM: http://sourceforge.net/projects/ogl-math/files/latest/download?source=files



Start building the project:
---------------------------

Create a new directory beside the repository folder.
Call this command inside the new folder:

cmake ../JediOutcastLinux/

After cmake is finished follow the last step.

#### On Linux:
make

#### On Windows: 
Open the VisualStudio solution and build the project

#### On Mac:
... sorry. Mac is not supported at the moment.

