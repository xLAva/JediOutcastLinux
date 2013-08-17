JediOutcastLinux
================

Jedi Knight II: Jedi Outcast (Single Player Linux Port).
https://github.com/xLAva/JediOutcastLinux

The current state is playable without any major problems.
- loading/saving and a lot of other open issues are fixed

I only had time to test the first two levels, but everything looks fine.


OpenPandora
===========

This fork is aimed at OpenPandra, so get:
 * ARM compatibilty (using -DARM)
 * Some NEON optimized code (using -DNEON)
 * GLES renderer (using -DHAVE_GLES)
 * Joystick support (using -DJOYSTICK)
 * Toggle Crouch function (using -DCROUCH)
 * Aiming assist, usefull when aiming with joystick or joystick like devices (single player only, there are changes in the server side) (using -DAUTOAIM)
 * OpenPandora support of course (using -DPANDORA), for screen resolution mainly.
 
For the Pandora version, I used only the CMake  make files, I didn't update the codeblock project file. I use the Code::Blocks PND to compile.


For more info on the OpenPandora go here: http://boards.openpandora.org/
Specific thread for Jedi Academy on the OpenPandora here: http://boards.openpandora.org/index.php/topic/13621-jedi-knight-ii-jedi-outcast/
