# Find all the source files
file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)
file(GLOB RESOURCES *.qrc)

# Group the source files in the IDE
source_group("resources" FILES ${RESOURCES})
source_group("" FILES ${SRCS} ${HDRS})

# Find the QtWidgets library, minimum 5.15 or 6.0
# https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html
find_package(Qt6 QUIET COMPONENTS Widgets)
if(NOT Qt6_FOUND)
  find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)
endif()

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

qt_add_resources(RESOURCE_ADDED ${RESOURCES})

# Create the executable target
add_executable(sample WIN32
  ${SRCS}
  ${HDRS}
  ${RESOURCE_ADDED}
)

target_link_libraries(sample
  Qt::Widgets
  animationeditor
)

if(WIN32)
  find_path(BIN_QT_BINARIES
    NAMES 
      Qt6Core.dll Qt6Cored.dll
      Qt5Core.dll Qt5Cored.dll
    HINTS
      ${Qt6Core_DIR}/../../../bin
      ${Qt6Core_DIR}/../../..
      ${Qt5Core_DIR}/../../../bin
      ${Qt5Core_DIR}/../../..
  )
  find_path(BIN_QT_PLUGINS
    NAMES 
      platforms styles imageformats
    HINTS
      ${Qt6Core_DIR}/../../../plugins
      ${Qt6Core_DIR}/../../..
      ${Qt5Core_DIR}/../../../plugins
      ${Qt5Core_DIR}/../../..
  )
  set_target_properties(sample PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=${BIN_QT_BINARIES};${BIN_QT_PLUGINS};%PATH%\nQT_PLUGIN_PATH=${BIN_QT_PLUGINS}")
endif()
