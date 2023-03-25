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

# Create the library target
add_library(animationeditor SHARED
  ${SRCS}
  ${HDRS}
  ${RESOURCE_ADDED}
)

target_link_libraries(animationeditor
	Qt::Widgets
)

target_compile_definitions(animationeditor PRIVATE -DANIMATIONEDITOR_LIB)
