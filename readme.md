A patched version of Chameleon(default window decoration) on DeepinV20, which allows to show your windows' menu on the titlebar.

Inspired by [material-decoration](https://github.com/Zren/material-decoration) and copied lots of code from it :)

![screenshot](./screenshot/screenshot.png)

### How to use on Deepin V20:
1. install [dde-globalmenu-service](https://github.com/SeptemberHX/dde-globalmenu-service) and set it with autostart(right click on the launcher)
2. `sudo apt install libkf5config-bin libqt5quickparticles5 libqt5quickshapes5` (please use `ldd` to check the dependencies)
3. modify `~/.gtkrc-2.0`, add `gtk-modules=appmenu-gtk-module`
4. modify `～/.config/gtk-3.0/settings.ini`，add `gtk-modules=appmenu-gtk-module` under [Settings] 
4. download the release zip file in release page(only for deepinv20. other distributions need to compile from source)
5. copy the libdde-chameleon.so to `/usr/lib/x86_64-linux-gnu/qt5/plugins/org.kde.kdecoration2`(you should backup the one provided by the system)

### How to build from source on UOS:
1. `sudo apt install extra-cmake-modules qtbase5-dev qtdeclarative5-dev libkf5coreaddons-dev libkf5config-dev libkf5windowsystem-dev libkdecorations2-dev libqt5x11extras5-dev libkf5guiaddons-dev kwin-dev`
2. `cd Chameleon-patched`
3. `mkdir build; cd build`
4. `cmake ..`
5. `make -j8`

### How to restore if something wrong happens:
1. press ctrl+alt+F2 and login your account
2. copy raw *.so back or `sudo apt reinstall dde-kwin`
