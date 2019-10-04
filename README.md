**QuickAccess** is a program providing a menu to quickly access user defined folders and their subfolders.

The menu can be opened through two dbus methods: showMenu and showDelayedMenu.

**showMenu** shows the menu instantly, but in certain circumstances there are problems with the menu not showing or not closing. In these cases use the **showDelayedMenu**, there is a default delay of 150 miliseconds, but can be changed (see example below).

# Installation

Easiest way to install is through [flatpak](https://flathub.org/apps/details/com.georgefb.quickaccess), flatpaks should work on all distros.

[Flatpak setup guide](https://flatpak.org/setup/)
```
flatpak install flathub com.georgefb.quickaccess
flatpak run com.georgefb.quickaccess
```

If you don't like flatpak you can build from source or ask the your distro to provide a package.

# Usage

Start with tray icon

```
quickaccess
quickaccess --tray-icon=show
```

Start without tray icon

```
quickaccess --tray-icon=hide
```

Open menu with qdbus

```
qdbus com.georgefb.quickaccess /QuickAccess showMenu
qdbus com.georgefb.quickaccess /QuickAccess showDelayedMenu 200
```

Open menu with dbus-send

```
dbus-send --type=method_call --dest=com.georgefb.quickaccess /QuickAccess com.georgefb.QuickAccess.showMenu
dbus-send --type=method_call --dest=com.georgefb.quickaccess /QuickAccess com.georgefb.QuickAccess.showDelayedMenu int32:200
```
[KDE Plasma shortcut:](https://docs.kde.org/trunk5/en/kde-workspace/kcontrol/khotkeys/shortcuts.html)

![Set shortcut](data/images/quickaccess-plasma-shortcut.png)

# Dependencies
- Extra CMake Modules
- Qt5 Widgets
- Qt5 DBus
- KF5 I18n
- KF5 Config
- KF5 XmlGui

Install dependencies:

- **Ubuntu** `sudo apt install build-essential cmake extra-cmake-modules qtbase5-dev libkf5config-dev libkf5i18n-dev libkf5xmlgui-dev`
- **Solus** `sudo eopkg it -c system.devel extra-cmake-modules qt5-base-devel kconfig-devel ki18n-devel kxmlgui-devel`

# Build

```
cd /path/to/quickaccess_project
mkdir build && cd build
cmake ..
make
./quickaccess
```

![Showcase](data/images/quickaccess-showcase.gif)
