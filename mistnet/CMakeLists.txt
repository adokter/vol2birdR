#
# Ideas and some code in this file originates from https://github.com/mlverse/torch after
# some mail exchange with Daniel Falbel (Copyright) about pros and cons. All credit should go to him.
# Original licensed with MIT. https://github.com/mlverse/torch/blob/main/LICENSE.md
#
cmake_minimum_required(VERSION 3.10)

project(mistnet)

############################################################
# Helpers
############################################################
function(download_file url filename)
    if(NOT EXISTS ${filename})
        message("-- Downloading ${filename}: ${url}")
        file(DOWNLOAD  ${url} ${filename}
             TIMEOUT 600
             TLS_VERIFY ON
        )
    endif()
endfunction()

function(retrieve_lib url name)
    download_file("${url}" "${CMAKE_CURRENT_BINARY_DIR}/${name}.zip")

    if(NOT EXISTS "${name}")
        message("-- Extracting ${name}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${name}.zip
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
endfunction()


############################################################
# Torch
############################################################

set(CMAKE_VERBOSE_MAKEFILE ON)

if(APPLE)
    if ('${CMAKE_HOST_SYSTEM_PROCESSOR}' STREQUAL 'x86_64')
      retrieve_lib("https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.12.1.zip" "libtorch")
    elseif ('${CMAKE_HOST_SYSTEM_PROCESSOR}' STREQUAL 'arm64')
      retrieve_lib("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/main/latest/libtorch-macos-arm64-volbird-1.12.1.zip" "libtorch")
    else()
      message(FATAL_ERROR, "Unknown Apple arch.")
    endif()
elseif(WIN32)
    retrieve_lib("https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-1.12.1%2Bcpu.zip" "libtorch")
elseif(UNIX)
    retrieve_lib("https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.12.1%2Bcpu.zip" "libtorch")
else()
    message(FATAL_ERROR, "OS not supported.")
endif()

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_BINARY_DIR}/libtorch/share/cmake/Torch")

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message("-- Using ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
else()
    message("-- ccache not found.")
endif(CCACHE_FOUND)

set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# see https://github.com/pytorch/pytorch/issues/18607#issuecomment-480665557
if(WIN32)
  add_compile_options("/permissive")
  add_compile_options("/permissive-")
endif()


############################################################
# Library
############################################################

set(MISTNET_SRC 
    src/mistnet.cpp
)


add_library(mistnet SHARED ${MISTNET_SRC})

add_library(mistnet::library ALIAS mistnet)

target_include_directories(mistnet PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(mistnet ${TORCH_LIBRARIES})
set_property(TARGET mistnet PROPERTY CXX_STANDARD 17)

