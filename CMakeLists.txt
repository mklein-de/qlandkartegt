project(QLandkarteGT)

# Required cmake version
cmake_minimum_required(VERSION 2.6.0)

set(APPLICATION_NAME ${PROJECT_NAME})

set(APPLICATION_VERSION_MAJOR "0")
set(APPLICATION_VERSION_MINOR "18")
set(APPLICATION_VERSION_PATCH "2")

add_definitions(-DVER_MAJOR=${APPLICATION_VERSION_MAJOR} -DVER_MINOR=${APPLICATION_VERSION_MINOR} -DVER_STEP=${APPLICATION_VERSION_PATCH})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
if (APPLE)
  set(PROGNAME "QLandkarte GT")
  set(CONFIGDIR "Library/Application Support/${PROGNAME}/")
  set(MAC_BUNDLE_PATH ${EXECUTABLE_OUTPUT_PATH}/${PROGNAME}.app)
  set(MAC_ICON_FILE qlandkartegt.icns)
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH})
  set(MACOSX_BUNDLE_ICON_FILE ${MAC_ICON_FILE})
  set(MACOSX_BUNDLE_BUNDLE_NAME ${PROGNAME})
  set(MACOSX_BUNDLE_BUNDLE_VERSION ${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR})
  set(MACOSX_BUNDLE_GUI_IDENTIFIER org.qlandkarte.QLandkarteGT)
else(APPLE)
  set(PROGNAME qlandkartegt)
  set(CONFIGDIR ".config/QLandkarteGT/")
endif(APPLE)
if(MSVC)
  # Add link-time code generation to improve performance
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
  set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /LTCG")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
endif(MSVC)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake/Modules
)

# add definitions
include(DefineCMakeDefaults)
include(DefineCompilerFlags)
include(DefineInstallationPaths)
include(DefineOptions.cmake)
include(CPackConfig.cmake)

# config.h generation
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(3rdparty/SerialPort)
add_subdirectory(3rdparty/QMKToolBox)
add_subdirectory(3rdparty/getopt)
add_subdirectory(src)

if (UNIX AND NOT WIN32 AND NOT APPLE)
  install(
    FILES
      qlandkartegt.desktop
    DESTINATION
      ${XDG_APPS_DIR}
  )

  install(
    FILES
      qlandkartegt.png
    DESTINATION
      ${DATA_INSTALL_PREFIX}/pixmaps
  )

  install(
    FILES
      qlandkartegt.1
    DESTINATION
      ${MAN_INSTALL_DIR}/man1
  )
endif (UNIX AND NOT WIN32 AND NOT APPLE)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


