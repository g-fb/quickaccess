cmake_minimum_required(VERSION 3.0)
project(quickaccess)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(ECMInstallIcons)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Widgets
    DBus
)
find_package(KF5 REQUIRED COMPONENTS
    Config
    ConfigWidgets
    I18n
    IconThemes
)

set(quickaccess_SRC
    src/main.cpp
    src/mainwindow.cpp
    src/pathsmenu.cpp
    src/treewidget.cpp
    src/settings/settingswindow.cpp
    src/settings/generalsettingspage.cpp
    src/settings/folderssettingspage.cpp
    src/settings/commandssettingspage.cpp
    data/com.georgefb.quickaccess.xml
    data/icons/icons.qrc
)

ki18n_wrap_ui(quickaccess_SRC
    ui/aboutdialog.ui
    ui/addcommand.ui
    ui/addmenu.ui
    ui/settings.ui
    ui/startupdialog.ui
)

qt5_add_dbus_adaptor(quickaccess_SRC data/com.georgefb.quickaccess.xml
    src/mainwindow.h MainWindow)

kconfig_add_kcfg_files(quickaccess_SRC GENERATE_MOC settings.kcfgc)

add_executable(quickaccess ${quickaccess_SRC})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        src
        src/settings)
target_link_libraries(quickaccess PRIVATE
    Qt5::Widgets
    Qt5::DBus
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::I18n
    KF5::IconThemes
)

ecm_install_icons(ICONS
    data/icons/512-apps-quickaccess.svg
    DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)

install(TARGETS quickaccess DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES data/com.georgefb.quickaccess.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(FILES data/com.georgefb.quickaccess.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(PROGRAMS data/com.georgefb.quickaccess.desktop DESTINATION ${KDE_INSTALL_APPDIR})
