#
# Original idea and part of the code originates from https://github.com/mlverse/torch after some mail exchange
# with Daniel Falbel (Copyright).
# Original licensed with MIT. https://github.com/mlverse/torch/blob/main/LICENSE.md
#
cmake_minimum_required(VERSION 3.18)

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

# The archive name is stamped with the LibTorch version. Without this, a build tree
# that already holds `libtorch.zip` from a previous version would silently skip the
# download and build against the wrong LibTorch -- the archive contents are not
# otherwise distinguishable by name. The extraction guard likewise checks an absolute
# path in the binary dir; the previous relative "${name}" was resolved against the
# wrong directory and so effectively never matched.
function(retrieve_lib url name)
    set(archive "${CMAKE_CURRENT_BINARY_DIR}/${name}-${LIBTORCH_VERSION}.zip")
    download_file("${url}" "${archive}")

    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${name}")
        message("-- Extracting ${name}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf "${archive}"
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
endfunction()


############################################################
# Torch
############################################################

set(CMAKE_VERBOSE_MAKEFILE ON)

# LibTorch download URLs, keyed by version. These must stay in step with
# `install_config` in R/install.R: this file downloads LibTorch to *compile*
# libmistnet, while install.R downloads LibTorch again at run time on the user's
# machine. A mismatch between the two produces a successful build followed by a
# load-time failure, so the workflow publishes version-suffixed artifacts to keep
# the pairing honest.
#
# Not every version covers every platform, hence a table rather than string
# substitution into a single template. An empty URL means "this version has no build
# for this platform" and is a hard error below.
set(LIBTORCH_VERSION "1.12.1" CACHE STRING "LibTorch version to build against")

if(LIBTORCH_VERSION STREQUAL "1.12.1")
    set(LIBTORCH_URL_MACOS_X86_64 "https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.12.1.zip")
    # pytorch.org shipped no arm64 macOS build at this version; this is a locally
    # patched rebuild. Retired from 2.14.0 onwards, where an official build exists.
    set(LIBTORCH_URL_MACOS_ARM64  "https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/main/latest/libtorch-macos-arm64-volbird-1.12.1.zip")
    set(LIBTORCH_URL_WINDOWS      "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-1.12.1%2Bcpu.zip")
    set(LIBTORCH_URL_LINUX        "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.12.1%2Bcpu.zip")
elseif(LIBTORCH_VERSION STREQUAL "2.1.2")
    # Intel macOS only: 2.1.2 is the last PyTorch release shipping an x86_64 macOS
    # build. Every other platform is served by 2.14.0, so those URLs are left empty
    # deliberately rather than pointing at a version nobody should be using.
    set(LIBTORCH_URL_MACOS_X86_64 "https://download.pytorch.org/libtorch/cpu/libtorch-macos-2.1.2.zip")
    set(LIBTORCH_URL_MACOS_ARM64  "")
    set(LIBTORCH_URL_WINDOWS      "")
    set(LIBTORCH_URL_LINUX        "")
elseif(LIBTORCH_VERSION STREQUAL "2.14.0")
    # No Intel macOS build exists in this series -- see the 2.1.2 branch above.
    set(LIBTORCH_URL_MACOS_X86_64 "")
    set(LIBTORCH_URL_MACOS_ARM64  "https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-2.14.0.zip")
    set(LIBTORCH_URL_WINDOWS      "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-2.14.0%2Bcpu.zip")
    # 2.x dropped the `cxx11-abi` infix from the Linux archive name.
    set(LIBTORCH_URL_LINUX        "https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-2.14.0%2Bcpu.zip")
else()
    message(FATAL_ERROR "Unsupported LIBTORCH_VERSION '${LIBTORCH_VERSION}'. Supported: 1.12.1, 2.1.2, 2.14.0.")
endif()

# Fail at configure time on a platform/version pair that does not exist, rather than
# letting an empty URL turn into a confusing download or link error later.
function(retrieve_libtorch url platform)
    if("${url}" STREQUAL "")
        message(FATAL_ERROR
            "LibTorch ${LIBTORCH_VERSION} has no ${platform} build. "
            "Choose a version that supports this platform.")
    endif()
    retrieve_lib("${url}" "libtorch")
endfunction()

if(APPLE)
    if ('${CMAKE_HOST_SYSTEM_PROCESSOR}' STREQUAL 'x86_64')
      retrieve_libtorch("${LIBTORCH_URL_MACOS_X86_64}" "macOS x86_64")
    elseif ('${CMAKE_HOST_SYSTEM_PROCESSOR}' STREQUAL 'arm64')
      retrieve_libtorch("${LIBTORCH_URL_MACOS_ARM64}" "macOS arm64")
    else()
      message(FATAL_ERROR "Unknown Apple arch.")
    endif()
elseif(WIN32)
    retrieve_libtorch("${LIBTORCH_URL_WINDOWS}" "Windows")
elseif(UNIX)
    retrieve_libtorch("${LIBTORCH_URL_LINUX}" "Linux")
else()
    message(FATAL_ERROR "OS not supported.")
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

