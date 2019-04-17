cmake_minimum_required(VERSION 3.0)
project(quickaccess)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Widgets
    DBus
)

find_package(KF5 REQUIRED COMPONENTS
    I18n            # KLocalizedString
    Config
)

set(quickaccess_SRC
    src/main.cpp
    src/mainwindow.cpp
    src/pathsmenu.cpp
    src/com.georgefb.quickaccess.xml
)

ki18n_wrap_ui(quickaccess_SRC
   src/AboutDialog.ui
)

qt5_generate_dbus_interface(
    mainwindow.h
    com.georgefb.quickaccess.xml
    OPTIONS -a
)

qt5_add_dbus_adaptor(quickaccess_SRC src/com.georgefb.quickaccess.xml
                     src/mainwindow.h MainWindow)
                     
# Tell CMake to create the helloworld executable
add_executable(quickaccess ${quickaccess_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(quickaccess
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::ConfigCore
)

# Install the executable
install(TARGETS quickaccess DESTINATION bin)
install(FILES src/com.georgefb.quickaccess.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(FILES src/com.georgefb.quickaccess.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(PROGRAMS src/com.georgefb.quickaccess.desktop DESTINATION ${KDE_INSTALL_APPDIR})
