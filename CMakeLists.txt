set(TARGET_NAME "deepin-chameleon")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-DTARGET_NAME=\\"${TARGET_NAME}\\")

find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH}
        )

find_package(Qt5 CONFIG REQUIRED COMPONENTS Core DBus Qml)
find_package(KF5CoreAddons REQUIRED)
find_package(KF5Config REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KDecoration2 REQUIRED)
find_package(Qt5X11Extras REQUIRED)
find_package(KF5GuiAddons REQUIRED)
include(ECMQtDeclareLoggingCategory)

include_directories(${Qt5Core_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

option(DISABLE_LIB_KWIN_XCB "Don't use libkwin-xcb" OFF)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory("appmenu")

add_library(
    ${TARGET_NAME}
    MODULE
        main.cpp
        chameleon.h
        chameleon.cpp
        chameleontheme.cpp
        chameleonbutton.cpp
        chameleonshadow.cpp
        chameleonconfig.cpp
        chameleonwindowtheme.cpp
        kwinutils.cpp
        themes.qrc
        Button.cpp
        TextButton.cpp
        AppMenuButton.cpp
        MenuOverflowButton.cpp
        debug.h AppMenuButtonGroup.cpp AppMenuButtonGroup.h)

target_link_libraries(${TARGET_NAME}
PUBLIC
    Qt5::Core
    Qt5::X11Extras
    Qt5::Qml
    dde-appmenu
PRIVATE
    KDecoration2::KDecoration
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::WindowSystem
    KF5::GuiAddons
)

install(TARGETS ${TARGET_NAME} DESTINATION ${QT_INSTALL_PLUGINS}/org.kde.kdecoration2)
