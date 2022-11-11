cmake_minimum_required(VERSION 3.2)
project(mooon)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/cmake-build-debug) # by CLion
message(STATUS "Begin " ${PROJECT_NAME})

include(mooon.cmake)

add_subdirectory(src)

set(TEST OFF)
if (${TEST})
    add_subdirectory(test)
endif ()

set(TOOL OFF)
if (${TOOL})
    add_subdirectory(tool)
endif ()

# using CMAKE_INSTALL_PREFIX
message(STATUS "CMAKE_INSTALL_PREFIX is " ${CMAKE_INSTALL_PREFIX})
install(
        FILES mooon.cmake
        DESTINATION .
       )
