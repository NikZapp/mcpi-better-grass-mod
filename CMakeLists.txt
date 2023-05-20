cmake_minimum_required(VERSION 3.16.0)

if(NOT MODS_BEING_BUILD STREQUAL "all")
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
endif()

# Start Project
project(niksbettergrass-proj)
# Include SDK
include("$ENV{HOME}/.minecraft-pi/sdk/lib/minecraft-pi-reborn-client/sdk/sdk.cmake")
# Build
add_library(niksbettergrass SHARED niksbettergrass.cpp helpers.cpp)
target_link_libraries(niksbettergrass mods-headers reborn-patch symbols misc)