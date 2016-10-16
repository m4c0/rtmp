cmake_minimum_required(VERSION 3.5)
project(rtmp VERSION 2.4 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)

add_definitions(-DRTMPDUMP_VERSION="v${CMAKE_PROJECT_VERSION}")
add_definitions(-DNO_CRYPTO)

add_subdirectory(librtmp)
add_subdirectory(examples)

add_executable(rtmpsrv rtmpsrv.c thread.c)
target_link_libraries(rtmpsrv PUBLIC rtmp)

add_executable(rtmpdump rtmpdump.c thread.c)
target_link_libraries(rtmpdump PUBLIC rtmp)
