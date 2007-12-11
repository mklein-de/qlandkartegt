project(QLandkarteGT)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/./bin)

find_package(Qt4 REQUIRED)

set(VER_MAJOR     0)
set(VER_MINOR     0)
set(VER_STEP      1)

add_definitions(-DVER_MAJOR=${VER_MAJOR} -DVER_MINOR=${VER_MINOR} -DVER_STEP=${VER_STEP})

add_subdirectory(./src)
