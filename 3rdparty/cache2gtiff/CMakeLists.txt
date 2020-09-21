project(map2gcm)

cmake_minimum_required(VERSION 3.1)

find_package(PkgConfig REQUIRED)

find_package(GDAL REQUIRED)
pkg_check_modules(PROJ REQUIRED IMPORTED_TARGET proj)
add_definitions(-DACCEPT_USE_OF_DEPRECATED_PROJ_API_H=1)

if(QK_QT5_PORT)
  find_package(Qt5Widgets)
  find_package(Qt5Xml)
else(QK_QT5_PORT)
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
endif(QK_QT5_PORT)

set(SRCS
    main.cpp
    ../../src/CDiskCache.cpp
    ../../src/IDiskCache.cpp
)

set(HDRS
    ../../src/CDiskCache.h
    ../../src/IDiskCache.h
)

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
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GDAL_INCLUDE_DIRS}
    ${PROJ_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
    ../../src/
)

if (NOT QK_QT5_PORT)
include_directories(
  ${QT_QTXML_INCLUDE_DIR}
)
endif(NOT QK_QT5_PORT)


if(WIN32)
    include_directories(
        ${CMAKE_SOURCE_DIR}/Win32/
    )
endif(WIN32)

add_executable( cache2gtiff
    ${SRCS}
    ${HDRS}
    ${UI_HDRS}
    ${RC_SRCS}
    ${MOC_SRCS}
    ${QM_FILES}
)

add_definitions(-DSTANDALONE)

if(APPLE)
set_target_properties( cache2gtiff PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${MAC_BUNDLE_PATH}/Contents/Resources
)
endif(APPLE)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE )
endif(MSVC)

target_link_libraries(cache2gtiff
    ${GDAL_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${QT_LIBRARIES}
    ${QT_QTXML_LIBRARIES}
    PkgConfig::PROJ
)



if(QK_QT5_PORT)
qt5_use_modules(cache2gtiff  Widgets)
qt5_use_modules(cache2gtiff  Xml)
endif(QK_QT5_PORT)

install(
    TARGETS
      cache2gtiff
    DESTINATION
      ${BIN_INSTALL_DIR}
)
