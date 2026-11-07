cmake_minimum_required(VERSION 3.16)
project(aeroqt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt6 only (Qt5 support removed).
find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(QT_VERSION_MAJOR 6)

set(AEROQT_SOURCES
  src/aerostyle.cpp
  src/theme.cpp
  src/glass.cpp
  src/modules/buttons.cpp
  src/modules/inputs.cpp
  src/modules/progress.cpp
  src/modules/scrollbar.cpp
  src/modules/slider.cpp
  src/modules/spincombo.cpp
  src/modules/tabs.cpp
  src/modules/menus.cpp
  src/modules/frames.cpp
  src/modules/itemviews.cpp
  src/modules/docks.cpp
  src/modules/bars.cpp
  src/modules/misc.cpp
  src/modules/titlebar.cpp
  themes/themes.qrc
)

# Core as a static lib so the plugin AND the preview tool share it.
add_library(aeroqt_core STATIC ${AEROQT_SOURCES})
target_include_directories(aeroqt_core PUBLIC src)
target_link_libraries(aeroqt_core PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)
set_target_properties(aeroqt_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# The style plugin: <qt plugins>/styles/libaeroqt.so
add_library(aeroqt MODULE src/plugin.cpp)
target_link_libraries(aeroqt PRIVATE aeroqt_core)
set_target_properties(aeroqt PROPERTIES PREFIX "lib")

# Visual debug tool: renders a widget gallery to PNG (works offscreen).
add_executable(aeroqt-preview tools/preview.cpp)
target_link_libraries(aeroqt-preview PRIVATE aeroqt_core)

# Headless animation checks (hover fade, marquee). Run: ctest, or ./aeroqt-animtest
add_executable(aeroqt-animtest tools/animtest.cpp)
target_link_libraries(aeroqt-animtest PRIVATE aeroqt_core)
enable_testing()
add_test(NAME anim_on COMMAND aeroqt-animtest)
set_tests_properties(anim_on PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
add_test(NAME anim_off COMMAND aeroqt-animtest)
set_tests_properties(anim_off PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;AEROQT_NO_ANIM=1")

# Core regression tests: theme parsing/merge, HiDPI slab cache, animator bookkeeping.
add_executable(aeroqt-coretest tools/coretest.cpp)
target_link_libraries(aeroqt-coretest PRIVATE aeroqt_core)
add_test(NAME core COMMAND aeroqt-coretest)
set_tests_properties(core PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

include(GNUInstallDirs)
set(AEROQT_PLUGIN_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/plugins/styles"
    CACHE STRING "Where to install the style plugin")
install(TARGETS aeroqt LIBRARY DESTINATION ${AEROQT_PLUGIN_DIR})
install(TARGETS aeroqt-preview RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# QQC2 style for Kirigami/QML KDE apps (selected with QT_QUICK_CONTROLS_STYLE=AeroQt)
set(AEROQT_QML_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/qml/QtQuick/Controls"
    CACHE STRING "Where to install the QQC2 style")
install(DIRECTORY qml/QtQuick/Controls/AeroQt DESTINATION ${AEROQT_QML_DIR})
install(PROGRAMS tools/aeroqt-env.sh DESTINATION ${CMAKE_INSTALL_DATADIR}/aeroqt)
