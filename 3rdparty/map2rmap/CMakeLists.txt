project(map2rmap)

cmake_minimum_required(VERSION 3.1)

find_package(PkgConfig REQUIRED)

find_package(PkgConfig REQUIRED)

find_package(GDAL REQUIRED)
pkg_check_modules(PROJ REQUIRED IMPORTED_TARGET proj)
add_definitions(-DACCEPT_USE_OF_DEPRECATED_PROJ_API_H=1)
find_package(JPEG REQUIRED)

if(QK_QT5_PORT)
  find_package(Qt5Widgets)
else(QK_QT5_PORT)
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
endif(QK_QT5_PORT)

set(SRCS
    main.cpp
    CInputFile.cpp
)

set(HDRS
    main.h
    CInputFile.h
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
)
if(WIN32)
    include_directories(
        ${CMAKE_SOURCE_DIR}/Win32/
    )
endif(WIN32)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

add_executable( map2rmap
    ${SRCS}
    ${HDRS}
    ${UI_HDRS}
    ${RC_SRCS}
    ${MOC_SRCS}
    ${QM_FILES}
)

if(APPLE)
set_target_properties( map2rmap PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${MAC_BUNDLE_PATH}/Contents/Resources
)
endif(APPLE)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE )
endif(MSVC)



target_link_libraries(map2rmap
    ${GDAL_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${QT_LIBRARIES}
    PkgConfig::PROJ
)

if(QK_QT5_PORT)
  qt5_use_modules(map2rmap Widgets)
endif(QK_QT5_PORT)

install(
    TARGETS
      map2rmap
    DESTINATION
      ${BIN_INSTALL_DIR}
)
