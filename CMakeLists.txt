cmake_minimum_required(VERSION 3.16.1)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")

project(lightscript)

file(GLOB_RECURSE SOURCES "source/*.cpp")
file(GLOB_RECURSE HEADERS "source/*.hpp")

add_executable(lightscript ${SOURCES} ${HEADERS})

set_target_properties(lightscript PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED OFF
        CXX_EXTENSIONS OFF
)
