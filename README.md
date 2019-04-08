**QuickAccess** is a program providing a menu to quickly access user defined folders and their subfolders.
The menu can be opened through dbus.

![Showcase](images/quickaccess-showcase.gif)

# Example

toggle menu with qdbus
```
qdbus org.QuickAccess /QuickAccess showMenu
```

# Dependencies
- Qt5 Widgets
- Qt5 Dbus
- KF5 CoreAddons
- KF5 I18n
- KF5 Config