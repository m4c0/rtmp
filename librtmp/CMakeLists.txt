add_library(rtmp rtmp.c log.c amf.c hashswf.c parseurl.c)
target_include_directories(rtmp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
