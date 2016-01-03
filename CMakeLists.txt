cmake_minimum_required(VERSION 3.0)

set(PRODUCT_NAME DYLDSharedCache)
project(${PRODUCT_NAME})

set(HOPPER_SDK_PATH ${CMAKE_SOURCE_DIR}/HopperSDK/include)
set(HOPPER_PLUGINS_PATH "~/Library/Application Support/Hopper/Plugins")
set(HOPPER_PLUGIN_LOADER_DIRNAME Loaders)
set(HOPPER_PLUGIN_LOADER_EXTENSION hopperLoader)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_INSTALL_PREFIX ${HOPPER_PLUGINS_PATH} CACHE PATH "Plugins install prefix" FORCE)

add_definitions(
    -std=c++11
    -fno-rtti
)

include_directories("${HOPPER_SDK_PATH}" "${CMAKE_SOURCE_DIR}")

add_library(
    dyld_shared_cache
    dyld/launch-cache/dsc_iterator.cpp
    dyld/launch-cache/dsc_extractor.cpp
)

add_library(
    ${PRODUCT_NAME} MODULE
    DYLDSharedCacheLoader.mm
    DYLD.cpp
)

target_compile_options(
    ${PRODUCT_NAME} PUBLIC
    -fno-exceptions
    -pedantic
    -Wall
    -Wextra
    -fobjc-arc
)

set_target_properties(
    ${PRODUCT_NAME} PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION ${HOPPER_PLUGIN_LOADER_EXTENSION}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
)

target_link_libraries(${PRODUCT_NAME} "-framework Foundation" dyld_shared_cache)

install(
    TARGETS ${PRODUCT_NAME}
    LIBRARY DESTINATION ${HOPPER_PLUGIN_LOADER_DIRNAME}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
