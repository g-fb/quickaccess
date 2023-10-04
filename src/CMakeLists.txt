add_executable(quickaccess)

if (QT_MAJOR_VERSION STREQUAL "5")
    qt5_add_dbus_adaptor(quickaccess_dbus_src com.georgefb.quickaccess.xml mainwindow.h MainWindow)
else()
    qt_add_dbus_adaptor(quickaccess_dbus_src com.georgefb.quickaccess.xml mainwindow.h MainWindow)
endif()

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
        Qt${QT_MAJOR_VERSION}::Widgets
        Qt${QT_MAJOR_VERSION}::DBus
        KF${QT_MAJOR_VERSION}::ConfigCore
        KF${QT_MAJOR_VERSION}::ConfigWidgets
        KF${QT_MAJOR_VERSION}::CoreAddons
        KF${QT_MAJOR_VERSION}::I18n
        KF${QT_MAJOR_VERSION}::IconThemes
)
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(quickaccess
        PRIVATE
            KF6::IconWidgets
    )
endif()

install(TARGETS quickaccess DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES com.georgefb.quickaccess.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
