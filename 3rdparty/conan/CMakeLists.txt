project(conan)

# Required cmake version
cmake_minimum_required(VERSION 2.6.0)


set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})

set(SRCS
    src/ObjectUtility.cpp
    src/SignalSpy.cpp
    src/ConnectionModel.cpp
    src/ObjectModel.cpp
    src/ConanWidget.cpp
    src/AboutDialog.cpp
    src/ConanDebug.cpp
    src/KeyValueTableModel.cpp
)

set(HDRS
    src/ConanDefines.h
    src/ObjectUtility.h
    src/SignalSpy.h
    src/ConnectionModel.h
    src/ConanWidget_p.h
    src/ConanWidget.h
    src/ConanDefines.h
    src/AboutDialog.h
    src/ObjectModel.h
    src/ConanDebug.h
    src/KeyValueTableModel.h
    include/Conan.h
)

set(UIS
    src/ConanWidget.ui
    src/AboutDialog.ui
)

set(RCS
    src/Conan.qrc

)


qt4_wrap_cpp(MOC_SRCS ${HDRS})
qt4_wrap_ui(UI_HDRS ${UIS})
qt4_add_resources(RC_SRCS ${RCS})

include_directories(

)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

add_library(conan
  ${SRCS}
  ${HDRS}
  ${UI_HDRS}
  ${RC_SRCS}
  ${MOC_SRCS}
  ${QM_FILES}
)

target_link_libraries( conan
      ${QT_LIBRARIES}
)
