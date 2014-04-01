project(conan)

# Required cmake version
cmake_minimum_required(VERSION 2.6.0)


set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

if(QK_QT5_PORT)
  find_package(Qt5Widgets)
else(QT_QT5_PORT)
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
endif(QT_QT5_PORT)

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


if(QK_QT5_PORT)
  qt5_wrap_cpp(MOC_SRCS ${HDRS})
  qt5_wrap_ui(UI_HDRS ${UIS})
  qt5_add_resources(RC_SRCS ${RCS})
else(QK_QT5_PORT)
  qt4_wrap_cpp(MOC_SRCS ${HDRS})
  qt4_wrap_ui(UI_HDRS ${UIS})
  qt4_add_resources(RC_SRCS ${RCS})
endif(QK_QT5_PORT)

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

if(QK_QT5_PORT)
  qt5_use_modules(conan Widgets)
endif(QK_QT5_PORT)

target_link_libraries( conan
      ${QT_LIBRARIES}
)
