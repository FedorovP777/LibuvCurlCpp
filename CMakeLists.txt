cmake_minimum_required(VERSION 3.16)
project(libuv_curl_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/lib/x86_64-linux-gnu)
set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS_RELEASE "-O0")
SET(CMAKE_C_FLAGS_RELEASE "-O0")
find_package(CURL REQUIRED)
set(SRC_LIST
        libuv_curl_cpp)
add_executable(libuv_curl_cpp main.cpp)
set(LINK_LIBS
        uv
        ${CURL_LIBRARIES}
        )
set(INCLUDE_DIR ${CURL_INCLUDE_DIRS})
target_include_directories(libuv_curl_cpp PRIVATE ${INCLUDE_DIR})
target_link_libraries(libuv_curl_cpp PRIVATE ${LINK_LIBS})
################################
# PVS-Studio
################################
#include(./PVS-Studio.cmake)
#pvs_studio_add_target(TARGET pvs-studio ALL
#        OUTPUT FORMAT errorfile
#        ANALYZE ${PROJECT_NAME}
#        MODE GA:1,2 OP
#        LOG target.err)
################################
# GoogleTest
################################
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(
        run_tests
        tests/test.cpp
)
target_link_libraries(
        run_tests
        gtest_main
        ${LINK_LIBS}
)
target_include_directories(run_tests PRIVATE ${INCLUDE_DIR})
include(GoogleTest)
gtest_discover_tests(run_tests)