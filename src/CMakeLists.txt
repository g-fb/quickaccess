add_executable(quickaccess)

qt5_add_dbus_adaptor(quickaccess_dbus_src com.georgefb.quickaccess.xml mainwindow.h MainWindow)

ki18n_wrap_ui(quickaccess_ui_src
    ../ui/aboutdialog.ui
    ../ui/startupdialog.ui
)

target_sources(quickaccess
    PRIVATE
        main.cpp
        mainwindow.cpp
        pathsmenu.cpp
        settings/commandssettingspage.cpp
        settings/folderssettingspage.cpp
        settings/generalsettingspage.cpp
        settings/migrations.cpp
        settings/settingswindow.cpp
        treewidget.cpp
        com.georgefb.quickaccess.xml
        ../data/icons/icons.qrc
        ${quickaccess_dbus_src}
        ${quickaccess_ui_src}
)

kconfig_add_kcfg_files(quickaccess GENERATE_MOC settings/settings.kcfgc)

target_include_directories(quickaccess
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        src
        settings
)

target_link_libraries(quickaccess
    PRIVATE
        Qt5::Widgets
        Qt5::DBus
        KF5::ConfigCore
        KF5::ConfigWidgets
        KF5::I18n
        KF5::IconThemes
)

install(TARGETS quickaccess DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES com.georgefb.quickaccess.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
