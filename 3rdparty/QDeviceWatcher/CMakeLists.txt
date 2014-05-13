project(QDeviceWatcher)

# Required cmake version
cmake_minimum_required(VERSION 2.6.0)


set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

if(QK_QT5_PORT)
  find_package(Qt5Widgets)
else(QK_QT5_PORT)
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
endif(QK_QT5_PORT)

set(SRCS
    qdevicewatcher.cpp
    qdevicechangeevent.cpp
)

set(HDRS
    qdevicewatcher_p.h
    qdevicewatcher.h
    qdevicechangeevent.h
    qdevicewatcher_global.h
)

if(UNIX)
    SET(SRCS ${SRCS}  qdevicewatcher_linux.cpp)
endif(UNIX)
if (APPLE)
    SET(SRCS ${SRCS}  qdevicewatcher_mac.cpp)
endif(APPLE)
if(WIN32)
    SET(SRCS ${SRCS}  qdevicewatcher_win32.cpp)
endif(WIN32)


set(UIS

)

set(RCS


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

add_library(qdevicewatcher
  ${SRCS}
  ${HDRS}
  ${UI_HDRS}
  ${RC_SRCS}
  ${MOC_SRCS}
  ${QM_FILES}
)
if(QK_QT5_PORT)
  qt5_use_modules(qdevicewatcher )
endif(QK_QT5_PORT)

target_link_libraries( qdevicewatcher
      ${QT_LIBRARIES}
)
