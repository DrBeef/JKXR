![JKXR Banner](https://github.com/DrBeef/JKXR/blob/main/assets/JKXRGithub.jpg)
===

# JK XR

[SideQuest Latest Version (Meta Headsets)](https://sidequestvr.com/app/15472)

[SideQuest Latest Version (Pico Headsets)](https://sidequestvr.com/app/15541)

JK XR is a VR port of the Jedi Knight games using OpenXR (the open standard for virtual and augmented reality devices) and is based on the excellent OpenJK port, originally forked from: https://github.com/JACoders/OpenJK


The easiest way to install this on your device is using SideQuest. Download SideQuest here:
https://sidequestvr.com/setup-howto



### Jedi Knight: Jedi Outcast / Jedi Academy

The public release of JK XR now supports the games Jedi Outcast and Jedi Academy.

## Team Beef Patreon
[![Team Beef Patreon](https://github.com/DrBeef/JKXR/blob/main/assets/PatreonBanner.jpg)](https://www.patreon.com/teambeef)

The Team Beef Patreon where you can find all the in-development early-access builds other active Team Beef projects.


## Gameplay and VR Features

### VR Features

* New Fully Modelled VR Weapons
* Full Motion Controlled Light Saber
* Real Collision based Laser Deflections
* Weapon / Force wheels 
* Gesture Based Use / Interact
* Gesture based Force Actions (Push, Pull and Grab)
* Weapon Scopes
* Gesture Based Saber Throw 
* Companion App

### Gameplay Modes (accessible via Setup -> Difficulty in the Menu)

**Team Beef Directors Cut (TBDC) - On (Default is On)**
This version uses faithful enemy speeds and aggression from the original game, which are fast and challenging by modern standards. To balance this projectile speeds and gun power are raised to feel more canon to the Star Wars movies and prevent Stormtroopers and other enemies from being able to avoid gunfire by strafing. There are also exaggerated knockback effects. This mode is more arcade-y fast paced affair whilst feeling similar to the difficulty level of the twitch-based gameplay of the original game. 

**Team Beef Directors Cut (TBDC) - Off**
Projectile speeds are faithful to the original game, but enemy movement and aggression are toned back, where stormtroopers don't have an easy time to flank you. You may need to still "lead" shots slightly ahead of enemies when they are on the move. Recommended for a slower paced tactical encounter

To switch between modes change the option and if already in-game, restart the level you are on.

### Companion App

The companion app will be installed at the same time when using the SideQuest official page. This allows you to download mods and addons and switch between launching single player missions and the main game. This also detects if you've correctly set up the main application and asset files so is a good place to look if you are having issues. 

![image](https://user-images.githubusercontent.com/4569081/230429909-2df64bb6-e200-496f-ba5f-bda763539de0.png)


## IMPORTANT NOTE

*This is just an engine port*; the apk does not contain any of the of Jedi Knight game assets. If you wish to play the full game you must purchase it yourself, steam is most straightforward:  https://store.steampowered.com/app/6030/STAR_WARS_Jedi_Knight_II__Jedi_Outcast/

# Installation and Setup

You can find the latest version, which also includes the Companion App hosted on Sidequest. Use the links at the top of the page (separated by headset). Before installing via SideQuest you must have enabled "Developer Mode" on your headset. You can find the details on how to do that below:

Pico 3/4 Instructions
---------------------
https://trello.com/c/Idb627uv/47-pico-4-installation-instructions

Meta Quest Install Instructions
-------------------------------
https://trello.com/c/C0YTFpvX/48-quest-quest-2-installation-instructions

PCVR
----
Download the PCVR installers from the release page and use them to install to the game location.

### OpenXR runtimes information for PCVR Headsets

We recommend the following combinations to get the optimal experience while playing JKXR on PCVR:

**Valve Index** -> Via SteamVR (SteamVR OpenXR Runtime) -> This will be standard for Valve Index Users

**HTC Vive** -> Via SteamVR (SteamVR OpenXR Runtime) -> This will be standard for Valve Index Users

**Meta Headsets** -> Link / Airlink (Oculus OpenXR Runtime) -> Make sure this is set via the Oculus desktop app Settings->General->OpenXR Runtime

**Windows Mixed Reality (WMR) Devices** (I.e. HP G2) -> Make sure you set the SteamVR to be the default OpenXR runtime. 

**Pimax** -> Currently unplayable. We have been in discussions with Pimax and there is a new PimaxPlay that fixes the issues (upside down screens). 
When released the game must be played via unofficial PimaxXR OpenXR runtime (https://github.com/mbucchia/Pimax-OpenXR). Do not play via SteamVR OpenXR runtime

**Pico** - Our honest option is the best version on Pico is the standalone version on Sidequest. If you are determined to use PCVR, the only working option is via Virtual Desktop. Ensure you have set SteamVR as the OpenXR Runtime in the Virtual Desktop Streamer app (see picture)

Do not play via Streaming Assistant / VDXR


## Copying the Full Game files to your Meta Quest / Pico

Before you are able to run the full game of Jedi Knight: Jedi Outcast in VR you will need to:

- Install JK XR thought SideQuest (this will also install the companion app)
- Start JK XR for the first time, it will ask for appropriate permissions (which you must allow), create the necessary folders and then close down
- You can now run the Companion App. This will check that it has the files for the full game. 
- Copy only the assets files (assets0.pk3, assets1.pk3, assets2.pk3, assets5.pk3) from your PC install of JKO (Jedi Outcast\GameData\base) into the following folder on your device:

\JKXR\JK2\base

or 

\JKXR\JK3\base


- You must then use the Companion App to download any mods and start the game. It is possible to play the JK Demo without having copied across the full game assets. All other mods require the full game. 


### Save Games File

Save game files are stored on the internal memory of your device. This means that if you uninstall/reinstall JK XR all the saves are retained. If you change headset just make sure you copy your saves to the new device.
Any update will not affect any of your save game files.


## Controls and configuration

### Tutorials 

You can find tutorial videos on how to use the special VR features in the in-game Controls -> JKXR HELP menu. 

![image](https://user-images.githubusercontent.com/4569081/230427577-59d77ff2-b960-4817-bbcd-d7722dcd1ead.png)

### Control Scheme

This control scheme on how to play can also be found in the Controls -> JKXR HELP in the game.

![Control Scheme](https://github.com/DrBeef/JKXR/blob/main/z_vr_assets_base/gfx/menus/control_scheme.jpg)





## Building from Source

If you wish to build JK XR from source, then you need the following:

* Android Developer Studio
* Android SDK API level 24
* Latest Android Native Development Kit
* The OpenXR release archive for your headset

There is no dependency on any specific headset native libraries, as this port uses OpenXR and links against the opensource Android openxr_loader.so. If you wish to build for a specific headset, then you need to place the openxr_loader.so for the headset into the following folder and rename it to:  openxr_loader_{device}.so  where {device} is either *meta* or *pico* (further device support hopefully in the future):

JKXR\Projects\Android\libs\arm64-v8a


PCVR instructions will follow in the future (when I get round to adding them), 

## Credits

* Team Beef are DrBeef,  Baggyg,  Bummser
* Lead programmer: DrBeef
* JKXR Companion App: BaggyG
* Additional Development Contributions: MuadDib, BaggyG
* VR Compatible Weapon Models: Vince Crusty  and  Elin
* VR Compatible Hand Models: LennyGuy20

With Special Thanks to: Team Beef patrons, all Team Beef discord members, 
the OpenJK Development Team and Raven Software for
creating and open-sourcing these wonderful games

## DISCLAIMER

THIS ENGINE PORT IS NOT MADE, DISTRIBUTED, OR SUPPORTED BY ACTIVISION PUBLISHING, INC., RAVEN SOFTWARE, OR LUCASARTS ENTERTAINMENT COMPANY, LLC. ELEMENTS™ & © LUCASFILM LTD.™ & DISNEY, INC.™ AND/OR ITS LICENSORS. STAR WARS®, JEDI®, & JEDI KNIGHT® ARE REGISTERED TRADEMARKS OF LUCASFILM LTD™ AND WALT DISNEY, INC.™ STAR WARS®, JEDI®, & JEDI KNIGHT® ARE REGISTERED TRADEMARKS OF LUCASFILM LTD™ & DISNEY, INC.™
