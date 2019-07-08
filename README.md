**QuickAccess** is a program providing a menu to quickly access user defined folders and their subfolders.

The menu can be opened through two dbus methods: showMenu and showDelayedMenu.

**showMenu** shows the menu instantly, but in certain circumstances there are problems with the menu not showing or not closing. In these cases use the **showDelayedMenu**, there is a default delay of 150 miliseconds, but can be changed (see example below).

# Example

toggle menu with qdbus

```
qdbus com.georgefb.quickaccess /QuickAccess showMenu
qdbus com.georgefb.quickaccess /QuickAccess showDelayedMenu 200
```

toggle menu with dbus-send

```
dbus-send --type=method_call --dest=com.georgefb.quickaccess /QuickAccess com.georgefb.QuickAccess.showMenu
dbus-send --type=method_call --dest=com.georgefb.quickaccess /QuickAccess com.georgefb.QuickAccess.showDelayedMenu int32:200
```
[KDE Plasma shortcut:](https://docs.kde.org/trunk5/en/kde-workspace/kcontrol/khotkeys/shortcuts.html)

![Set shortcut](images/quickaccess-plasma-shortcut.png)

# Dependencies
- Extra CMake Modules
- Qt5 Widgets
- Qt5 DBus
- KF5 I18n
- KF5 Config

Install dependencies:

- **Ubuntu** `sudo apt install build-essential cmake extra-cmake-modules qtbase5-dev libkf5config-dev libkf5i18n-dev`
- **Solus** `sudo eopkg it -c system.devel extra-cmake-modules qt5-base-devel kconfig-devel ki18n-devel`

# Build

```
cd /path/to/quickaccess_project
mkdir build && cd build
cmake ..
make
./quickaccess
```

![Showcase](images/quickaccess-showcase.gif)
