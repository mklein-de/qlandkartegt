project(map2gcm)

# Required cmake version
cmake_minimum_required(VERSION 2.6.0)

find_package(Qt4 REQUIRED)
find_package(GDAL REQUIRED)
find_package(PROJ REQUIRED)

include(${QT_USE_FILE})

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

qt4_wrap_cpp(MOC_SRCS ${HDRS})
qt4_wrap_ui(UI_HDRS ${UIS})
qt4_add_resources(RC_SRCS ${RCS})

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GDAL_INCLUDE_DIRS}
    ${PROJ_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
    ${QT_QTXML_INCLUDE_DIR}
    ../../src/
)
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

target_link_libraries( cache2gtiff
    ${GDAL_LIBRARIES}
    ${PROJ_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${QT_LIBRARIES}
    ${QT_QTXML_LIBRARIES}
)


install(
    TARGETS
      cache2gtiff
    DESTINATION
      ${BIN_INSTALL_DIR}
)
