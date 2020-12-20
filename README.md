# Clight [![builds.sr.ht status](https://builds.sr.ht/~fededp/clight.svg)](https://builds.sr.ht/~fededp/clight?)

[![Packaging status](https://repology.org/badge/vertical-allrepos/clight.svg)](https://repology.org/project/clight/versions)

Clight is a C user daemon utility that aims to fully manage your display.  
It was heavily inspired by [calise](http://calise.sourceforge.net/wordpress/) in its initial intents.  

**For a guide on how to build, features and lots of other infos, head to [Clight Wiki Pages](https://github.com/FedeDP/Clight/wiki).**  
**Note that Wiki pages will always refer to master branch.**  
*For any other info, please feel free to open an issue.*  

## Features

Clight allows to match your backlight level to ambient brightness, computed by capturing frames from webcam or Ambient Light Sensors.  
It does also support adjusting external monitors and keyboard backlight.  
Moreover, it can manage your screen temperature, just like redshift does.  
Finally, it can dim your screen after a timeout and manage screen DPMS.  

Note that all its features are available on both **X, Wayland and tty** and can be turned off from its config file.  
On wayland Clight requires specific protocols to be implemented by your compositor; have a look at https://github.com/FedeDP/Clight/wiki/Modules#wayland-support.  

## Developers Corner

Clight makes use of [Clightd](https://github.com/FedeDP/Clightd), a system DBus service that exposes an [API](https://github.com/FedeDP/Clightd/wiki/Api) to manage various aspects of your screen and allows Webcam/ALS devices captures.  
Its API is as generic as possible and it has nothing specifically for Clight; this means anyone can make use of it.  
If you are interested, please have a look at its wiki pages too!  
Indeed i even developed a super simple Clight clone as an hello world application in Go: https://github.com/FedeDP/GoLight.  
It is much simpler than Clight for obvious reasons and i do not expect to develop it further.  

Both Clight and Clightd make use of [libmodule](https://github.com/FedeDP/libmodule), a C library developed with modularity in mind that offers a simple actor framework for C, with an integrated event loop.

Morever, note that Clight exposes a DBus [API](https://github.com/FedeDP/Clightd/wiki/Api) itself too; it allows quickly testing config values or building scripts around it, eg: https://github.com/FedeDP/Clight/wiki/DE-Automation.  
Finally, it can also be expanded through [Custom modules](https://github.com/FedeDP/Clight/wiki/Custom-Modules) that enable users to build their own plugins to further customize Clight behaviour.  

## Packages
Clight is available on Archlinux AUR as both stable or devel package: https://aur.archlinux.org/packages/?K=clight .  
Note that devel package may break in some weird ways during development. Use it at your own risk.  

Moreover, some brave distros are already shipping Clight as you can see from above packaging badge. Special thanks to all maintainers!

## License
This software is distributed with GPL license, see [COPYING](https://github.com/FedeDP/Clight/blob/master/COPYING) file for more informations.
