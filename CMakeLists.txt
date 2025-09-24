cmake_minimum_required(VERSION 3.15)
project(quickaccess VERSION 3.2.0 LANGUAGES CXX)

set(QT_MIN_VERSION 6.6.0)
set(KF6_MIN_VERSION 6.0.0)

include(FeatureSummary)

find_package(ECM ${KF6_MIN_VERSION} NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED
    URL "https://api.kde.org/ecm/"
    DESCRIPTION "extra cmake modules")
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt6Widgets ${QT_MIN_VERSION})
set_package_properties(Qt6Widgets PROPERTIES TYPE REQUIRED)

find_package(Qt6DBus ${QT_MIN_VERSION})
set_package_properties(Qt6DBus PROPERTIES TYPE REQUIRED)

find_package(KF6Config ${KF6_MIN_VERSION})
set_package_properties(KF6Config PROPERTIES TYPE REQUIRED)

find_package(KF6ConfigWidgets ${KF6_MIN_VERSION})
set_package_properties(KF6ConfigWidgets PROPERTIES TYPE REQUIRED)

find_package(KF6CoreAddons ${KF6_MIN_VERSION})
set_package_properties(KF6CoreAddons PROPERTIES TYPE REQUIRED)

find_package(KF6I18n ${KF6_MIN_VERSION})
set_package_properties(KF6I18n PROPERTIES TYPE REQUIRED)

find_package(KF6IconThemes ${KF6_MIN_VERSION})
set_package_properties(KF6IconThemes PROPERTIES TYPE REQUIRED)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

include(KDEInstallDirs)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX QUICKACCESS
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/quickaccess-version.h"
)

add_subdirectory(data)
add_subdirectory(src)
