project(QLandkarteGT)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/./bin)

find_package(Qt4 REQUIRED)

set(VER_MAJOR     0)
set(VER_MINOR     4)
set(VER_STEP      2)

add_definitions(-DVER_MAJOR=${VER_MAJOR} -DVER_MINOR=${VER_MINOR} -DVER_STEP=${VER_STEP})

IF(WIN32)
  LINK_DIRECTORIES( C:/Programme/FWTools2.1.1/lib )
  INCLUDE_DIRECTORIES( C:/Programme/FWTools2.1.1/include )
ENDIF(WIN32)

IF(UNIX)
    # find GDAL headers
    FIND_PATH(GDAL_INCLUDE gdal.h /usr/include /usr/include/gdal)

    IF(NOT ${GDAL_INCLUDE} STREQUAL "/usr/include")
        INCLUDE_DIRECTORIES( ${GDAL_INCLUDE} )
    ENDIF(NOT ${GDAL_INCLUDE} STREQUAL "/usr/include")

    IF(NOT GDAL_INCLUDE )
        MESSAGE(FATAL_ERROR "\n\nCan't find GDAL headers. Is GDAL installed correctly?\n")
    ENDIF(NOT GDAL_INCLUDE )
    MESSAGE("Using " ${GDAL_INCLUDE} " for GDAL")

    # find Proj4 headers
    FIND_PATH(PROJ_INCLUDE projects.h /usr/include /usr/include/proj4)

    IF(NOT ${PROJ_INCLUDE} STREQUAL "/usr/include")
        INCLUDE_DIRECTORIES( ${PROJ_INCLUDE} )
    ENDIF(NOT ${PROJ_INCLUDE} STREQUAL "/usr/include")

    IF(NOT PROJ_INCLUDE )
        MESSAGE(FATAL_ERROR "\n\nCan't find Proj4 headers. Is Proj4 installed correctly?\n")
    ENDIF(NOT PROJ_INCLUDE )
    MESSAGE("Using " ${PROJ_INCLUDE} " for Proj4")

ENDIF(UNIX)

add_subdirectory(./src)

INSTALL(PROGRAMS ${PROJECT_BINARY_DIR}/./bin/qlandkartegt DESTINATION /usr/bin)
