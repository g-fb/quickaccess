cmake_minimum_required(VERSION 3.0)
project(quickaccess)

set(KF5_MIN_VERSION "5.2.0")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons      # KAboutData
    I18n            # KLocalizedString
    XmlGui          # KXmlGuiWindow, KActionCollection
)

set(quickaccess_SRC
  src/main.cpp
  src/mainwindow.cpp
)

# Tell CMake to create the helloworld executable
add_executable(quickaccess ${quickaccess_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(quickaccess
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
)

# Install the executable
install(TARGETS quickaccess DESTINATION bin)
