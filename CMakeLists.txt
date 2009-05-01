project(QLandkarteGT)

# Required cmake version
cmake_minimum_required(VERSION 2.6.0)

set(APPLICATION_NAME ${PROJECT_NAME})

set(APPLICATION_VERSION_MAJOR "0")
set(APPLICATION_VERSION_MINOR "12")
set(APPLICATION_VERSION_PATCH "0.pre")

add_definitions(-DVER_MAJOR=${APPLICATION_VERSION_MAJOR} -DVER_MINOR=${APPLICATION_VERSION_MINOR} -DVER_STEP=${APPLICATION_VERSION_PATCH})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

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

# disallow in-source build
include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of source build. Please create a separate build directory and run 'cmake /path/to/${PROJECT_NAME} [options]' there.")

# config.h generation
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)

if (UNIX AND NOT WIN32)
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
endif (UNIX AND NOT WIN32)

