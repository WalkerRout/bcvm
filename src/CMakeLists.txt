set(EXEC ${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false *.c)
set(SOURCES ${SOURCES})

add_executable(${EXEC}_run ${SOURCES})
add_library(${EXEC}_lib STATIC ${SOURCES})
