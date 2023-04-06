![JKXR Banner](https://github.com/DrBeef/JKXR/blob/main/assets/JKXRGithub.jpg)
===

# JK XR

JK XR is a VR port of the Jedi Knight games using OpenXR (the open standard for virtual and augmented reality devices) and is based on the excellent OpenJK port, originally forked from: https://github.com/JACoders/OpenJK

This is currently built for standalone VR HMDs (see the Building section below) and will currently **not** run on any other device, the list of supported devices is: Meta Quest (1, 2, Pro) and Pico (3 & 4).

Support for PCVR based devices will hopefully come in the future.

The easiest way to install this on your device is using SideQuest. Download SideQuest here:
https://github.com/the-expanse/SideQuest/releases



### Jedi Knight: Jedi Outcast

The public release of JK XR currently supports the game Jedi Outcast; access to the Jedi Academy early-access build can be found on the Team Beef patreon (link below). However there are lots of mods and access to the free demo level available through the accomapnying JK XR Companion App, which is also installed alongside JK XR on SideQuest.



## Team Beef Patreon
[![Team Beef Patreon](https://github.com/DrBeef/JKXR/blob/main/assets/PatreonBanner.jpg)](https://www.patreon.com/teambeef)

The Team Beef Patreon where you can find all the in-development early-access builds for JK XR with Jedi Academy support, as well as other active projects can be found.



## IMPORTANT NOTE


*This is just an engine port*; the apk does not contain any of the of Jedi Knight game assets. If you wish to play the full game you must purchase it yourself, steam is most straightforward:  https://store.steampowered.com/app/6030/STAR_WARS_Jedi_Knight_II__Jedi_Outcast/



### Copying the Full Game files to your Oculus Quest

Before you are able to run the full game of Jedi Knight: Jedi Outcast in VR you will need to:

- Install JK XR thought SideQuest (this will also install the companion app)
- Start JK XR for the first time, it will ask for appropriate permissions (which you must allow), create the necessary folders and then close down
- You can now run the Companion App and download the Demo level if you wish, or...
- (optional) Copy only the assets files from your PC install of JKO (Jedi Outcast\GameData\base) into the following folder on your device:

\JKXR\JK2\base

- You must then use the Companion App to start the game and apply any mods you wish to also use that you downloaded through the companion app



### Save Games File

Save game files are stored on the internal memory of your device. This means that if you uninstall/reinstall JK XR all the saves are retained. If you change headset just make sure you copy your saves to the new device.
Any update will not affect any of your save game files.



## Installation and Setup

The same APK will work on all supported devices.

Meta Quest Install Instructions
https://trello.com/c/C0YTFpvX/48-quest-quest-2-installation-instructions

Pico 3/4 Install Instructions
https://trello.com/c/Idb627uv/47-pico-4-installation-instructions



## Controls and configuration

This control scheme and helpful short videos on how to play can be found in the Controls -> JKXR HELP in the game.

![Control Scheme](https://github.com/DrBeef/JKXR/blob/main/z_vr_assets_base/gfx/menus/control_scheme.jpg)


## Building from Source

If you wish to build JK XR from source, then you need the following:

* Android Developer Studio
* Android SDK API level 24
* Latest Android Native Development Kit
* The OpenXR release archive for your headset

There is no dependency on any specific headset native libraries, as this port uses OpenXR and links against the opensource Android openxr_loader.so. If you wish to build for a specific headset, then you need to place the openxr_loader.so for the headset into the following folder and rename it to:  openxr_loader_{device}.so  where {device} is either *meta* or *pico* (further device support hopefully in the future):

JKXR\Projects\Android\libs\arm64-v8a
