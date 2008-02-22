project(QLandkarteGT)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/./bin)

find_package(Qt4 REQUIRED)

set(VER_MAJOR     0)
set(VER_MINOR     1)
set(VER_STEP      0)

add_definitions(-DVER_MAJOR=${VER_MAJOR} -DVER_MINOR=${VER_MINOR} -DVER_STEP=${VER_STEP})

IF(WIN32)
  LINK_DIRECTORIES( C:/Programme/FWTools1.3.9/lib )
  INCLUDE_DIRECTORIES( C:/Programme/FWTools1.3.9/include )
ENDIF(WIN32)

FIND_PATH(GDAL_INCLUDE gdal.h PATHS /usr/include /usr/include/gdal)

IF(UNIX)
    IF(NOT ${GDAL_INCLUDE} STREQUAL "/usr/include")
        INCLUDE_DIRECTORIES( ${GDAL_INCLUDE} )
    ENDIF(NOT ${GDAL_INCLUDE} STREQUAL "/usr/include")
    MESSAGE("Using " ${GDAL_INCLUDE} " for GDAL")
ENDIF(UNIX)

add_subdirectory(./src)
