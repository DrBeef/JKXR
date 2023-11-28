
####################################################
#  INSTALL AND RUN INSTRUCTIONS FOR JKXR FOR PCVR  #
####################################################



NOTES ON OPENXR
===============


This build of JKXR has so far only been tested on Meta and Pico devices. We recommend if you are using a Meta device that you set Oculus as the default OpenXR runtime. For all other devices you can set SteamVR as the OpenXR runtime. We're looking for feedback on what works and what doesn't (with Vive Wands expected not to work as yet - but still need info). If your headset manufacturer includes an OpenXR runtime better to use that.... use SteamVR as a fallback.




STEAM INSTALLS
==============

Please note, any instructions below that are for Jedi Outcast, the exact same instructions are applicable to Jedi Academy, though the folder/file names need to be change as appropriate.


To Install
----------

Simply double click on the file:   Steam__Install.bat

This should copy all the necessary bits to the right places. 
If it fails, then you will have to find your steam install folder and follow the non-steam instructions below.

NOTE: If you've played JKO/JKA using OpenJK in the past, then there will probably be an old config file lying around that may mess stuff up.. browse to the install folder and delete openjo_sp.cfg or openjk_sp.cfg


To Run
------

Simply double click on the file:   Steam__Run_JKXR.bat


To Uninstall
------------

Simply double click on the file:   Steam__Uninstall.bat




FOR OTHER (NON-STEAM)
=====================

Please note, the instructions below are for Jedi Outcast, the exact same instructions are applicable to Jedi Academy, though the folder names need to be change as appropriate.


To Install
----------

You'll need to locate the folder in which your copy of Jedi Knight: Jedi Outcast/Academy is installed.


For Jedi Outcast copy the following files into the same folder as the jk2sp.exe:

openjk_sp.x86_64.exe
jagamex86_64.dll
rdsp-vanilla_x86_64.dll
SDL2.dll


For Jedi Academy copy the following files into the same folder as the jasp.exe:

openjo_sp.x86_64.exe
jospgamex86_64.dll
rdjosp-vanilla_x86_64.dll
SDL2.dll

And copy all the pk3 files from the base folder into the base folder of your install.

NOTE: If you've played JKO/JKA using OpenJK in the past, then there will probably be an old config file lying around that may mess stuff up.. browse to the install folder and delete openjo_sp.cfg or openjk_sp.cfg


ANOTHER NOTE: Alternatively you can actually play JKXR from the extracted folder location.. all you need to do in that case is copy the pk3 files from your installation of Jedi Outcast/Academy into the base folder in the location you extracted the 7zip archive to. In this case if you are playing the single player campaign on flatscreen, none of the VR modifications will interfere with your game.


To Start
--------

Simply double click the executable you copied in to your JKO install folder:  openjo_sp.x86_64.exe


To Uninstall
------------

Delete all the files you copied over originally



THINGS TO NOTE
==============

1. There are bugs - feedback appreciated
2. You will probably have to map your controllers if you aren't using a Quest or a Pico as we've not touched any controller mappings and configuration yet
3. If you have no sound, ensure the JKXR window has focus and that you have selected your headset as the primary sound output



MODS
====

This package contains some mods, please see the packaged_mods_credits.txt file for credits

We highly recommend the Expanded Menu for Jedi Outcast mod, this has a lot of excellent functionality:
https://www.moddb.com/mods/jedi-outcast-expanded-menu

We also recommend The Ladder for Jedi Outcast:
https://mrwonko.de/jk3files/Jedi%20Outcast/Maps/Single%20Player/2198/

Other mods will work with this build, however YMMV
