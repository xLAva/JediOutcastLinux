JediOutcastLinux
================

Jedi Knight II: Jedi Outcast (Single Player Linux Port).
https://github.com/xLAva/JediOutcastLinux

The current state is playable without any major problems.
- loading/saving and a lot of other open issues are fixed

I only had time to test the first two levels, but everything looks fine.


Binary

If you just want to play the game without compiling anything, the binary files are located here: "jedioutcast/code/Release/".
The binary files are not build or tested to run on every Linux system, but they should work on Ubuntu 12.10.
I just wanted to add a little short cut.

The following files are needed to run the game:
- jk2sp
- jk2gamex86.so

In order to start Jedi Outcast, the "base" folder from your original game has to be copied into the folder of the Linux binary files.
Be sure to mark "jk2sp" as executable and start the game with it.

For those not having the Steam version: you'll need the 1.04 update from here http://www.lucasarts.com/support/update/jedioutcast2.html 

Example:
~/jedioutcast/Release/jk2sp
~/jedioutcast/Release/jk2gamex86.so
~/jedioutcast/Release/base/

Needed libraries on Ubuntu 12.10 32bit:
sudo apt-get install libopenal1

Needed libraries on Ubuntu 12.10 64bit:
sudo apt-get install ia32-libs


Development

I used Code:Blocks as starting point for this port.
The whole compile process is based on it, but I think it should be pretty easy to add Makefiles or other compiling structures.
The source code is compiled with GCC.

My development system: Ubuntu 12.10 64bit
- I'm using a 64bit system, but this port is still a 32 bit program!

Needed development libraries (and codeblocks):
sudo apt-get install codeblocks g++-multilib libgl1-mesa-dev:i386 libxrandr-dev:i386 libopenal-dev:i386


Building

	cmake: These are the steps necessary to build the binaries using cmake, the directory names are only for reference,
	       you can change them as you wish.

			$ mkdir -p ~/src/jk2build
			$ cd ~/src/jk2build
			$ cmake ~/src/JediOutcastLinux 
			$ make

Known Issues:

- multi-monitor handling is still experimental
- some font rendering issues with the Intel Mesa driver (on my test machine)
- input handling in window mode is not perfect


Widescreen Feature:

This is the only thing I changed from the original code to improve the gaming experience.
- added some tweaks for widescreen support (show more content left and right instead of cutting content from top and bottom)
- if you want to play in your native monitor resolution you have to choose the following option "2048x1536". I change this to the current resolution of the main monitor during OpenGL start up.


Porting Notes

This was a fast port, so don't be surprised to see some bad hacks in place.
The first goal was to get it to run. Making it better is the next step.
I share the code now (in it's current shape), because I know you want to play with it and maybe help out.

There is still a lot of work left to do:
- CLEANUP: I have a lot of porting helpers in place (comments and other stuff)
- stay closer to the original code: play with compiler flags to avoid/revert some bigger changes
- make my widescreen tweaks optional


Feel free to contact me: jochen.leopold@model-view.com
