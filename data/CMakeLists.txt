ecm_install_icons(ICONS icons/sc-apps-quickaccess.svg
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)

install(FILES com.georgefb.quickaccess.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES com.georgefb.quickaccess.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
