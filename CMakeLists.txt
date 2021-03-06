#
#  AVSystem Commons Library
#
#  Copyright (C) 2017 AVSystem <http://www.avsystem.com/>
#
#  This code is free and open source software licensed under the MIT License.
#  See the LICENSE file for details.
#

################# DEFINITION ###################################################

cmake_minimum_required(VERSION 2.8.1)
project(avs_commons C)
set(AVS_COMMONS_VERSION SNAPSHOT)

################# DISTRIBUTION #################################################

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AVSystem Commons Library")
set(CPACK_PACKAGE_VENDOR "AVSystem")
set(CPACK_PACKAGE_VERSION "${AVS_COMMONS_VERSION}")
set(CPACK_SOURCE_GENERATOR "TGZ")

set(CPACK_SOURCE_IGNORE_FILES
    \\\\.a\\$
    \\\\.so\\$
    \\\\.so\\\\.
    /list/avs_list_test\\$
    /buffer/avs_buffer_test\\$
    /log/avs_log_test\\$
    /nbproject
    \\\\.log\\$
    /CMakeFiles/
    /CMakeTmp/
    /Makefile\\$
    /CMakeCache\\\\.txt\\$
    \\\\.cmake\\$
    /compile_commands\\\\.json\\$
    /install_manifest\\\\.txt\\$
    /_CPack_Packages/
    /Testing/
    \\\\.tar\\\\.gz\\$
    \\\\.tgz\\$
    \\\\.deb\\$
    /\\\\.git)

include(CPack)

add_custom_target(dist COMMAND ${CMAKE_CPACK_COMMAND} --config ${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

################# INITIALIZATION ###############################################

set(WITH_ADDITIONAL_LIBRARY_SUFFIXES "" CACHE STRING "Additional library file name suffixes")
set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES} ${WITH_ADDITIONAL_LIBRARY_SUFFIXES})
set(CMAKE_USE_RELATIVE_PATHS TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

################# LIBRARIES ####################################################

macro(avs_emit_deps TNAME)
    target_link_libraries(${TNAME} ${ARGN})
    list(APPEND AVS_TARGET_DEPS_${TNAME} ${ARGN})
endmacro()

macro(avs_temp_name fname)
    if(${ARGC} GREATER 1) # Have to escape ARGC to correctly compare
        set(_base ${ARGV1})
    else(${ARGC} GREATER 1)
        set(_base ".cmake-tmp")
    endif(${ARGC} GREATER 1)
    set(_counter 0)
    while(EXISTS "${CMAKE_BINARY_DIR}/${_base}${_counter}")
        math(EXPR _counter "${_counter} + 1")
    endwhile(EXISTS "${CMAKE_BINARY_DIR}/${_base}${_counter}")
    set(${fname} "${CMAKE_BINARY_DIR}/${_base}${_counter}")
endmacro()

macro(avs_find_library_inner)
    avs_temp_name(_fname)
    file(WRITE ${_fname} "${avs_find_library_EXPR_NOW}")
    include(${_fname})
    file(REMOVE ${_fname})
    foreach(PROVIDED_LIB ${ARGN})
        set(LIBRARY_FIND_ROUTINE_${PROVIDED_LIB} "${avs_find_library_EXPR_STORE}")
        set(LIBRARY_FIND_ROUTINE_PROVIDES_${PROVIDED_LIB} ${ARGN})
    endforeach()
endmacro()

macro(avs_find_library_ex EXPR_NOW EXPR_STORE)
    set(avs_find_library_EXPR_NOW "${EXPR_NOW}")
    set(avs_find_library_EXPR_STORE "${EXPR_STORE}")
    avs_find_library_inner(${ARGN})
    set(avs_find_library_EXPR_NOW)
    set(avs_find_library_EXPR_STORE)
endmacro()

macro(avs_find_library EXPR)
    set(avs_find_library_EXPR_NOW "${EXPR}")
    set(avs_find_library_EXPR_STORE "${EXPR}")
    avs_find_library_inner(${ARGN})
    set(avs_find_library_EXPR_NOW)
    set(avs_find_library_EXPR_STORE)
endmacro()

set(LIBRARY_FIND_ROUTINES)
macro(avs_emit_find_dependency LIB)
    list(FIND LIBRARY_FIND_ROUTINE_EMITTED_LIBS "${LIB}" EMITTED_LIB_INDEX)
    if(EMITTED_LIB_INDEX LESS 0)
        if(LIBRARY_FIND_ROUTINE_${LIB})
            set(LIBRARY_FIND_ROUTINES "${LIBRARY_FIND_ROUTINES}

${LIBRARY_FIND_ROUTINE_${LIB}}")
            list(APPEND LIBRARY_FIND_ROUTINE_EMITTED_LIBS
                 ${LIBRARY_FIND_ROUTINE_PROVIDES_${LIB}})
        endif()
    endif()
endmacro()

macro(avs_install_export TNAME CNAME)
    install(TARGETS ${TNAME} COMPONENT ${CNAME} EXPORT avs_commons-targets DESTINATION ${LIB_INSTALL_DIR})
    foreach(DEP ${AVS_TARGET_DEPS_${TNAME}})
        avs_emit_find_dependency(${DEP})
    endforeach()
endmacro()

macro(avs_propagate_exports)
    set(LIBRARY_FIND_ROUTINES "${LIBRARY_FIND_ROUTINES}" PARENT_SCOPE)
    set(LIBRARY_FIND_ROUTINE_EMITTED_LIBS "${LIBRARY_FIND_ROUTINE_EMITTED_LIBS}" PARENT_SCOPE)
endmacro()

# OpenSSL
avs_find_library("
    find_package(OpenSSL)
    if(OPENSSL_FOUND)
        include_directories(\\\${OPENSSL_INCLUDE_DIR})
        add_library(ssl UNKNOWN IMPORTED)
        add_library(crypto UNKNOWN IMPORTED)
        set_property(TARGET ssl PROPERTY IMPORTED_LOCATION \\\${OPENSSL_SSL_LIBRARY})
        set_property(TARGET crypto PROPERTY IMPORTED_LOCATION \\\${OPENSSL_CRYPTO_LIBRARY})
    endif()
" ssl crypto)

# mbed TLS
set(MBEDTLS_TARGETS mbedtls mbedcrypto mbedx509)
avs_find_library("
    set(CMAKE_MODULE_PATH \\\${CMAKE_MODULE_PATH} \"\\\${CMAKE_CURRENT_LIST_DIR}/cmake\")
    set(MBEDTLS_ROOT_DIR \"${MBEDTLS_ROOT_DIR}\")
    set(MBEDTLS_USE_STATIC_LIBS ${MBEDTLS_USE_STATIC_LIBS})
    find_package(MbedTLS)
" ${MBEDTLS_TARGETS})
set(MBEDTLS_INCLUDE_DIRS "" CACHE PATH "mbed TLS include directory")

# tinyDTLS
avs_find_library("find_library(TINYDTLS_FOUND tinydtls)" tinydtls)

# SSL
option(WITH_OPENSSL "Enable OpenSSL" OFF)
option(WITH_MBEDTLS "Enable mbed TLS" OFF)
option(WITH_TINYDTLS "Enable tinyDTLS" OFF)

set(_AVS_TLS_BACKENDS "")
if(WITH_OPENSSL)
    list(APPEND _AVS_TLS_BACKENDS "openssl")
endif()
if(WITH_MBEDTLS)
    list(APPEND _AVS_TLS_BACKENDS "mbedtls")
endif()
if(WITH_TINYDTLS)
    list(APPEND _AVS_TLS_BACKENDS "tinydtls")
endif()

list(LENGTH _AVS_TLS_BACKENDS _AVS_NUM_TLS_BACKENDS)
if(_AVS_NUM_TLS_BACKENDS GREATER 1)
    string(REPLACE ";" ", " _AVS_TLS_BACKENDS "${_AVS_TLS_BACKENDS}")
    message(FATAL_ERROR "Selected ${_AVS_TLS_BACKENDS} as TLS backends but only one may be used.")
elseif(_AVS_NUM_TLS_BACKENDS GREATER 0)
    set(WITH_SSL ON)
endif()

include(CMakeDependentOption)
cmake_dependent_option(WITH_DTLS "Enable OpenSSL DTLS support" ON WITH_SSL OFF)
cmake_dependent_option(WITH_EC_KEY "Enable OpenSSL EC_KEY support" ON WITH_SSL OFF)
cmake_dependent_option(WITH_PSK "Enable OpenSSL pre-shared key support" ON WITH_SSL OFF)

################################################################################

option(WITH_EXTRA_WARNINGS "Enable extra compilation warnings" OFF)
if(WITH_EXTRA_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Wall -Wextra -Winit-self -Wmissing-declarations -Wc++-compat -Wsign-conversion -Wconversion -Wcast-qual -Wno-variadic-macros -Wno-long-long")
endif()

if(CMAKE_C_COMPILE_OPTIONS_PIC)
    option(WITH_PIC "Generate position-independent code" ON)
    if(WITH_PIC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_COMPILE_OPTIONS_PIC}")
    endif()
endif()

# -fvisibility, #pragma GCC visibility
if(NOT DEFINED HAVE_VISIBILITY)
    file(WRITE ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/visibility.c "#pragma GCC visibility push(default)\nint f();\n#pragma GCC visibility push(hidden)\nint f() { return 0; }\n#pragma GCC visibility pop\nint main() { return f(); }\n\n")
    try_compile(HAVE_VISIBILITY ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/visibility.c COMPILE_DEFINITIONS -Wall -Wextra -Werror -fvisibility=default)
endif()

include(CheckIncludeFiles)
check_include_files("net/if.h" AVS_HAS_NET_IF_H)

include(CheckFunctionExists)
check_function_exists(gai_strerror HAVE_GAI_STRERROR)
check_function_exists(getifaddrs HAVE_GETIFADDRS)
check_function_exists(getnameinfo HAVE_GETNAMEINFO)
check_function_exists(inet_ntop HAVE_INET_NTOP)
check_function_exists(inet_pton HAVE_INET_PTON)
check_function_exists(poll HAVE_POLL)
check_function_exists(rand_r HAVE_RAND_R)
check_function_exists(backtrace HAVE_BACKTRACE)
check_function_exists(backtrace_symbols HAVE_BACKTRACE_SYMBOLS)

option(WITH_LWIP "Use lwIP library for networking" OFF)

option(WITH_IPV4 "Enable IPv4 support" ON)
option(WITH_IPV6 "Enable IPv6 support" ON)

option(WITH_SOCKET_LOG "Enable socket communication logging" OFF)

set(INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include_public"
    "${CMAKE_CURRENT_BINARY_DIR}/include_public")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/config"
                    "${CMAKE_CURRENT_BINARY_DIR}/config"
                    ${INCLUDE_DIRS})


find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    option(WITH_VALGRIND "Enable usage of valgrind during unit tests" OFF)
endif()
if(WITH_VALGRIND)
    execute_process(COMMAND ${VALGRIND_EXECUTABLE} --tool=helgrind --version
                    RESULT_VARIABLE HELGRIND_TEST_RESULT
                    OUTPUT_QUIET ERROR_QUIET)
    if(HELGRIND_TEST_RESULT EQUAL 0)
        option(WITH_HELGRIND "Put valgrind in helgrind mode (test for race conditions instead of memory errors)" OFF)
    endif()
    if(WITH_HELGRIND)
        set(VALGRIND_ARGS --tool=helgrind)
    else()
        set(VALGRIND_ARGS --leak-check=full --track-origins=yes)
    endif()
    set(VALGRIND ${VALGRIND_EXECUTABLE} ${VALGRIND_ARGS} -q --error-exitcode=63)
endif()

enable_testing()

add_custom_target(avs_commons_check)
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    add_custom_target(check)
    add_dependencies(check avs_commons_check)
endif()

if(NOT LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR lib)
endif()

if(NOT INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR include)
endif()

set(DOXYGEN_SKIP_DOT TRUE)
find_package(Doxygen)

if(DOXYGEN_FOUND)
    configure_file(doc/Doxyfile.in doc/Doxyfile @ONLY)
    add_custom_target(avs_commons_doc
                      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile)

    if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
        add_custom_target(doc)
        add_dependencies(doc avs_commons_doc)
    endif()
endif()

if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    set(MODULES_ENABLED ON)
else()
    set(MODULES_ENABLED OFF)
endif()

option(WITH_AVS_ALGORITHM "AVSystem algorithm utilities library" ${MODULES_ENABLED})
option(WITH_AVS_UNIT "AVSystem unit test library" ${MODULES_ENABLED})
option(WITH_AVS_BUFFER "AVSystem buffer implementation" ${MODULES_ENABLED})
option(WITH_AVS_LIST "AVSystem generic linked list implementation" ${MODULES_ENABLED})
option(WITH_AVS_VECTOR "AVSystem generic vector implementation" ${MODULES_ENABLED})
option(WITH_AVS_NET "AVSystem network communication abstraction layer" ${MODULES_ENABLED})
option(WITH_AVS_STREAM "AVSystem IO stream abstraction layer" ${MODULES_ENABLED})
option(WITH_AVS_LOG "AVSystem logging framework" ${MODULES_ENABLED})
option(WITH_AVS_RBTREE "AVSystem generic red-black tree implementation" ${MODULES_ENABLED})

cmake_dependent_option(WITH_INTERNAL_LOGS "Enable logging from inside AVSystem Commons libraries" ON WITH_AVS_LOG OFF)
cmake_dependent_option(WITH_INTERNAL_TRACE "Enable TRACE-level logs inside AVSystem Commons libraries" OFF WITH_INTERNAL_LOGS OFF)

macro(make_absolute_sources ABSVAR)
    set(${ABSVAR})
    foreach(SRC ${ARGN})
        set(${ABSVAR} ${${ABSVAR}} ${CMAKE_CURRENT_SOURCE_DIR}/${SRC})
    endforeach()
endmacro()

set(MODULE_INCLUDE_DIRS)

macro(add_module_with_include_dirs MODULE_NAME MODULE_INCLUDE_DIRS_VAR)
    add_subdirectory(${MODULE_NAME})
    include_directories(${avs_${MODULE_NAME}_INCLUDE_DIRS})
    # Append module includes to a specified variable name (i.e. MODULE_INCLUDE_DIRS_VAR).
    set(${MODULE_INCLUDE_DIRS_VAR}
        "${${MODULE_INCLUDE_DIRS_VAR}}"
        "${avs_${MODULE_NAME}_INCLUDE_DIRS}")
endmacro()

if(WITH_AVS_UNIT)
    macro(add_avs_test NAME)
        add_executable(${NAME}_test EXCLUDE_FROM_ALL ${ARGN})
        target_link_libraries(${NAME}_test avs_unit)
        set_property(TARGET ${NAME}_test APPEND PROPERTY COMPILE_DEFINITIONS AVS_UNIT_TESTING)
        set_property(TARGET ${NAME}_test APPEND PROPERTY COMPILE_FLAGS "-Wno-clobbered -Wno-overlength-strings -Wno-c++-compat -Wno-sign-conversion")

        if(VALGRIND)
            set(VALGRIND_LOG ${VALGRIND} --log-file=VALGRIND.${NAME}.log)
        else()
            set(VALGRIND_LOG)
        endif()

        add_test(NAME ${NAME}_test COMMAND ${VALGRIND_LOG} $<TARGET_FILE:${NAME}_test>)

        add_custom_target(${NAME}_check COMMAND ${CMAKE_CTEST_COMMAND} -V DEPENDS ${NAME}_test)
        add_dependencies(avs_commons_check ${NAME}_check)
    endmacro()

    add_module_with_include_dirs(unit MODULE_INCLUDE_DIRS)
    add_subdirectory(tests)
else()
    macro(add_avs_test NAME)
    endmacro()
endif()

if(WITH_AVS_ALGORITHM)
    add_module_with_include_dirs(algorithm MODULE_INCLUDE_DIRS)
endif()

if(WITH_AVS_BUFFER)
    add_module_with_include_dirs(buffer MODULE_INCLUDE_DIRS)
endif()

if(WITH_AVS_LIST)
    add_module_with_include_dirs(list MODULE_INCLUDE_DIRS)
endif()

if(WITH_AVS_VECTOR)
    add_module_with_include_dirs(vector MODULE_INCLUDE_DIRS)
endif()

if(WITH_AVS_NET)
    if(WITH_OPENSSL)
        if(NOT OPENSSL_FOUND)
            message(FATAL_ERROR "OpenSSL libraries not found")
        endif()

        option(OPENSSL_CUSTOM_CIPHERS_ENABLED "Customize OpenSSL cipher suite list" ON)
        if(OPENSSL_CUSTOM_CIPHERS_ENABLED)
            set(WITH_OPENSSL_CUSTOM_CIPHERS "DEFAULT:!ECDSA" CACHE STRING "Customized OpenSSL cipher suite list")
        endif()
    elseif(WITH_MBEDTLS)
        if(NOT MBEDTLS_FOUND)
            message(FATAL_ERROR "mbed TLS libraries not found")
        endif()
        separate_arguments(MBEDTLS_INCLUDE_DIRS)
        if(MBEDTLS_INCLUDE_DIRS)
            include_directories(${MBEDTLS_INCLUDE_DIRS})
        endif()
    elseif(WITH_TINYDTLS)
        if (NOT TINYDTLS_FOUND)
            message(FATAL_ERROR "tinyDTLS libraries not found")
        endif()
    endif()

    add_module_with_include_dirs(net MODULE_INCLUDE_DIRS)
endif()

if(WITH_AVS_STREAM)
    add_module_with_include_dirs(stream MODULE_INCLUDE_DIRS)
endif()

if(WITH_AVS_LOG)
    add_module_with_include_dirs(log MODULE_INCLUDE_DIRS)
endif()

if (WITH_AVS_RBTREE)
    add_module_with_include_dirs(rbtree MODULE_INCLUDE_DIRS)
endif()

# Export general avs_commons includes as well as module level includes.
if(NOT ${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    set(avs_commons_INCLUDE_DIRS ${INCLUDE_DIRS} ${MODULE_INCLUDE_DIRS} PARENT_SCOPE)
endif()

configure_file("config/config.h.in" "config/config.h")
configure_file("include_public/avsystem/commons/defs.h.in"
               "include_public/avsystem/commons/defs.h")

configure_file(avs_commons-config.cmake.in avs_commons-config.cmake @ONLY)
configure_file(avs_commons-version.cmake.in avs_commons-version.cmake @ONLY)
install(EXPORT avs_commons-targets DESTINATION ${LIB_INSTALL_DIR}/avs_commons)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/avs_commons-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/avs_commons-version.cmake
        DESTINATION ${LIB_INSTALL_DIR}/avs_commons)
install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindMbedTLS.cmake
        DESTINATION ${LIB_INSTALL_DIR}/avs_commons/cmake)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include_public/"
        DESTINATION ${INCLUDE_INSTALL_DIR}
        FILES_MATCHING REGEX "[.]h$")
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include_public/"
        DESTINATION ${INCLUDE_INSTALL_DIR}
        FILES_MATCHING REGEX "[.]h$")
