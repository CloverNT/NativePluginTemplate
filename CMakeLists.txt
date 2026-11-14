cmake_minimum_required(VERSION 4.0)

set(CLOVER_MANIFEST_IN "${CMAKE_CURRENT_SOURCE_DIR}/clover.json.in")
file(READ "${CLOVER_MANIFEST_IN}" CLOVER_MANIFEST)
string(JSON PLUGIN_NAME GET "${CLOVER_MANIFEST}" name)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CLOVER_MANIFEST_IN}")

project(${PLUGIN_NAME} LANGUAGES C CXX)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "This plugin must be compiled with Clang (clang-cl on Windows). Current compiler: ${CMAKE_CXX_COMPILER_ID}")
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(CLOVERNT_GENERATE_DTS OFF CACHE BOOL "" FORCE) # trash
FetchContent_Declare(CloverCore
        GIT_REPOSITORY https://github.com/CloverNT/CloverCore.git
        GIT_TAG main
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(CloverCore)

add_library(${PROJECT_NAME} SHARED src/main.cpp) # add your source files here
set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX "" # libfoo.so -> foo.so
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
)
target_include_directories(${PROJECT_NAME} PRIVATE "${clovercore_SOURCE_DIR}/include")

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE CloverCore)
else ()
    target_link_options(${PROJECT_NAME} PRIVATE "LINKER:--exclude-libs,ALL")
endif ()

set(LIBCXX_GITHUB_REPO "CloverNT/libcxx-build")
set(LIBCXX_ABI_NAMESPACE "__Cr")
include("${clovercore_SOURCE_DIR}/cmake/Libc++.cmake")
target_use_libcxx(${PROJECT_NAME})

set(PLUGIN_DIST "${CMAKE_BINARY_DIR}/dist/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGIN_DIST}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${PLUGIN_DIST}/"
        VERBATIM)

if (WIN32)
    set(PLUGIN_PLATFORM "windows")
else ()
    set(PLUGIN_PLATFORM "linux")
endif ()
set(PLUGIN_ENTRY "${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
configure_file("${CLOVER_MANIFEST_IN}" "${PLUGIN_DIST}/clover.json" @ONLY)
