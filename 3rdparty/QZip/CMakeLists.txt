project(QZip)

# Required cmake version
cmake_minimum_required(VERSION 2.6.0)


set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})

set(SRCS
    qzip.cpp
)

set(HDRS
    qzipreader.h
    qzipwriter.h
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
  ${QT_QTXML_INCLUDE_DIR}
)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

add_library(qzip
  ${SRCS}
  ${HDRS}
  ${UI_HDRS}
  ${RC_SRCS}
  ${MOC_SRCS}
  ${QM_FILES}
)

target_link_libraries( qzip
      ${QT_LIBRARIES}
)
