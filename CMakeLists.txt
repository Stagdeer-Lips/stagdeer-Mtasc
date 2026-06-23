cmake_minimum_required(VERSION 3.14)
project(stagdeer-Mtasclib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

if (WIN32) 
    message("[INFO] Compiling for Windows")
    set(OS_WINDOWS TRUE)
    set(PLATFORM "windows")
    set(SHARED_LIB_EXT "dll")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
    execute_process(
        OUTPUT_VARIABLE ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message("[INFO] ARCH: ${ARCH}")
        if (LINUX)
            message("[INFO] Compiling for Gnu Linux")
            execute_process(
                COMMAND uname -m
                OUTPUT_VARIABLE ARCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
    message("[INFO] ARCH: ${ARCH}")
        elseif(UNIX and APPLE)
            message("[INFO] Compiling for APPEL or UNIX")
            set(OS_MACOS TRUE)
            set(PLATFORM "macos")
            set(SHARED_LIB_EXT "dylib")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread")
            execute_process(
                COMMAND uname -m
                OUTPUT_VARIABLE ARCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
    message("[INFO] ARCH: ${ARCH}")
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "[INFO] MSVC (Visual Studio)")
    set(COMPILER_MSVC TRUE)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "[INFO] GCC (GNU)")
        set(COMPILER_GCC TRUE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            message(STATUS "[INFO] Clang (LLVM)")
            set(COMPILER_CLANG TRUE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
endif()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "[INFO] ARCH:64 (x86_64)")
    set(ARCH_64 TRUE)
else()
    message(STATUS "[INFO] ARCH:32 (x86)")
    set(ARCH_32 TRUE)
endif()

set(HEADER_DIR
    include/
)

set(LIB_DIR
    lib/parser_lib/urlparser_lib.cc
    lib/M_maker_httpv1_template.cc
)

add_library(mtasc ${LIB_DIR})
add_executable(mtasc_test test/test.cc)
target_include_directories(mtasc_test PRIVATE ${HEADER_DIR})
target_link_libraries(mtasc_test mtasc)
