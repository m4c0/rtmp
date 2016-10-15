cmake_minimum_required(VERSION 3.5)
project(rtmp VERSION 2.4)

add_definitions(-DRTMPDUMP_VERSION="v${CMAKE_PROJECT_VERSION}")
add_definitions(-DNO_CRYPTO)
add_subdirectory(librtmp)

