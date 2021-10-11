cmake_minimum_required(VERSION 3.15)
project(quickaccess VERSION 2.0.9 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM 5.66 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(FeatureSummary)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX QUICKACCESS
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/quickaccess-version.h"
)

find_package(Qt5Widgets)
set_package_properties(Qt5Widgets PROPERTIES TYPE REQUIRED)

find_package(Qt5DBus)
set_package_properties(Qt5DBus PROPERTIES TYPE REQUIRED)

find_package(KF5Config)
set_package_properties(KF5Config PROPERTIES TYPE REQUIRED)

find_package(KF5ConfigWidgets)
set_package_properties(KF5ConfigWidgets PROPERTIES TYPE REQUIRED)

find_package(KF5I18n)
set_package_properties(KF5I18n PROPERTIES TYPE REQUIRED)

find_package(KF5IconThemes)
set_package_properties(KF5IconThemes PROPERTIES TYPE REQUIRED)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_subdirectory(data)
add_subdirectory(src)
