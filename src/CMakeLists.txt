add_executable(quickaccess)

qt_add_dbus_adaptor(quickaccess_dbus_src com.georgefb.quickaccess.xml mainwindow.h MainWindow)

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

target_include_directories(quickaccess PRIVATE ${CMAKE_CURRENT_BINARY_DIR} settings)

target_link_libraries(quickaccess
    PRIVATE
        Qt6::Widgets
        Qt6::DBus
        KF6::ConfigCore
        KF6::ConfigWidgets
        KF6::CoreAddons
        KF6::I18n
        KF6::IconThemes
        KF6::IconWidgets
)

install(TARGETS quickaccess DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES com.georgefb.quickaccess.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
