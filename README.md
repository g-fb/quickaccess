**QuickAccess** is a program providing a menu to quickly access user defined folders and their subfolders.
The menu can be opened through dbus.

![Showcase](images/quickaccess-showcase.gif)

# Example

toggle menu with qdbus

```
qdbus com.georgefb.QuickAccess /QuickAccess showMenu
```

[KDE Plasma shortcut:](https://docs.kde.org/trunk5/en/kde-workspace/kcontrol/khotkeys/shortcuts.html)

![Set shortcut](images/quickaccess-plasma-shortcut.png)


# Dependencies
- Qt5 Widgets
- Qt5 Dbus
- KF5 CoreAddons
- KF5 I18n
- KF5 Config
