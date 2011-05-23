project(map2gcm)

# Required cmake version
cmake_minimum_required(VERSION 2.6.0)

find_package(Qt4 REQUIRED)
find_package(GDAL REQUIRED)
find_package(PROJ REQUIRED)
find_package(JPEG REQUIRED)

include(${QT_USE_FILE})

set(SRCS
    main.cpp
)

set(HDRS

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
    ${CMAKE_SOURCE_DIR}/3rdparty/QZip
)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

add_executable( map2gcm
    ${SRCS}
    ${HDRS}
    ${UI_HDRS}
    ${RC_SRCS}
    ${MOC_SRCS}
    ${QM_FILES}
)

if(APPLE)
set_target_properties( map2gcm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${MAC_BUNDLE_PATH}/Contents/Resources
)
endif(APPLE)

if(WIN32)
target_link_libraries( map2gcm
    ${GDAL_LIBRARIES}
    ${PROJ_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${QT_LIBRARIES}
    qzip
)
else(WIN32)
target_link_libraries( map2gcm
    ${GDAL_LIBRARIES}
    ${PROJ_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${QT_LIBRARIES}
    qzip
)
endif(WIN32)

install(
    TARGETS
      map2gcm
    DESTINATION
      ${BIN_INSTALL_DIR}
)
