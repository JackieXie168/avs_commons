#
#  AVSystem Commons Library
#
#  Copyright (C) 2017 AVSystem <http://www.avsystem.com/>
#
#  This code is free and open source software licensed under the MIT License.
#  See the LICENSE file for details.
#

set(SOURCES
    src/md5_common.c
    src/stream.c
    src/stream_file.c
    src/stream_inbuf.c
    src/stream_membuf.c
    src/stream_outbuf.c)

set(PRIVATE_HEADERS
    src/md5_common.h)

set(PUBLIC_HEADERS
    include_public/avsystem/commons/stream.h
    include_public/avsystem/commons/stream/stream_file.h
    include_public/avsystem/commons/stream/stream_inbuf.h
    include_public/avsystem/commons/stream/stream_membuf.h
    include_public/avsystem/commons/stream/stream_outbuf.h
    include_public/avsystem/commons/stream_v_table.h
    include_public/avsystem/commons/stream/md5.h)

set(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include_public")

set(avs_stream_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)

if(WITH_AVS_BUFFER AND WITH_AVS_NET)
    set(SOURCES ${SOURCES}
        src/net.c
        src/netbuf.c)
    set(PUBLIC_HEADERS ${PUBLIC_HEADERS}
        include_public/avsystem/commons/stream/net.h
        include_public/avsystem/commons/stream/netbuf.h)
    set(INCLUDE_DIRS ${INCLUDE_DIRS}
        ../buffer/include_public
        ../net/include_public)
endif()

if(WITH_OPENSSL)
    set(SOURCES ${SOURCES} src/openssl.c)
elseif(WITH_MBEDTLS)
    set(SOURCES ${SOURCES} src/mbedtls.c)
else()
    set(SOURCES ${SOURCES} src/md5_impl.c)
endif()

set(ALL_SOURCES ${SOURCES} ${PRIVATE_HEADERS} ${PUBLIC_HEADERS})

include_directories(${INCLUDE_DIRS})
include_directories(${AVS_TEST_INCLUDE_DIRS})
add_avs_test(avs_stream ${ALL_SOURCES})

add_library(avs_stream STATIC ${ALL_SOURCES})
if(WITH_INTERNAL_LOGS)
    target_link_libraries(avs_stream avs_log)
    if(WITH_AVS_UNIT)
        target_link_libraries(avs_stream_test avs_log)
    endif()
    include_directories(../log/include_public)
endif()
if(WITH_AVS_BUFFER AND WITH_AVS_NET)
    target_link_libraries(avs_stream avs_buffer avs_net)
endif()

avs_install_export(avs_stream stream)
avs_propagate_exports()
install(DIRECTORY include_public/
        COMPONENT stream
        DESTINATION ${INCLUDE_INSTALL_DIR}
        FILES_MATCHING REGEX "[.]h$")
