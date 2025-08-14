# What is HAI-Priiloader? #
This is an deprecated modification of [Priiloader](http://wiibrew.org/wiki/Priiloader) that retains all standard Priiloader features and enables autobooting an arbitrary vWii channel, irrespective of the injected iso being used. \
This is intended to only be used on vWii. Use [Priiloader](https://github.com/DacoTaco/priiloader) on a Wii (mini)
Due to HAI-IOS limitations, its use is limited to its current state to apply Wiimfi patches to injects by using the disc patcher.

## Requirements to Install ##
An installed wii vc title (original or injection).
A way to launch FTPiiU Everywhere on the Wii U and a FTP client installed on a pc.
Open Homebrew Channel. FIx94's version with gamepad support is strongly recommended.

## Installation instructions ##
It is strongly recommended to create a NAND backup before proceeding with the installation.
1. Follow the priiloader installation instructions to copy all needed files to the sd card. 
2. Replace priiloader's boot.dol with HAI-Priiloader's boot.dol (not included in this preliminary version)
3. Run the dol from the Open Homebrew Channel

## ANCAST image replacement ##
In this step the COMPAT-BS ANCAST image will be replaced with the BC-NAND ANCAST image. \
The BC-NAND ANCAST image is part of vWii title 0000000700000200 It can be obtained by using FTPiiU Everywhere to copy it from vWii. \
The image must be trimmed. Everything before the magic (0xEFA282D9) must be removed. \
Replace boot.bin located in OSv1 with the trimmed BC-NAND (renamed to boot.bin).

## fw.img patches ##
Use fix94's fork of nfs2iso2nfs with the --homebrew option. \
The patches for setting the titleID to boot and the aspect ratio are not yet included in nfs2iso2nfs. \
They must be manually applied to fw.img \
Note: The default behavior is to boot the virtual disk without enabling 4:3 aspect ratio. Manual patches are not required for this case. \
Replace the contents of the 9 bytes starting from address 0x0015D7C3 in revision r590 with [titleID,boot4by3]

## Important ##
If HAI-priiloader is uninstalled for any reason booting any VC inject produces a black screen. \
HAI-priiloader must be reinstalled or the ANCAST image replacement must be reversed for VC injects to work again.


## Thanks ##
DacoTaco for priiloader \
Fix94 for wiivc_chan_booter, libwiidrc, OHBC modifications

## ☕Donations ##
Feel free to donate to Dacotaco on [ko-fi](https://ko-fi.com/dacotaco) 
