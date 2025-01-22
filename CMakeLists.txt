##########################################################################
#
# Copyright (c) 2024 - Bryce Simonds
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
##########################################################################
# Basic Setup
##########################################################################

cmake_minimum_required(VERSION 3.20)

SET (default_build_type "Debug")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}', as none was specified.")
  SET (CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Coose the type of build." FORCE)
  SET_PROPERTY (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
else()
  message(STATUS "Build type set to '${CMAKE_BUILD_TYPE}'")
endif()

##########################################################################
# Dependencies
##########################################################################

include (fontawesome.cmake)
include(FetchContent)

SET (CMAKE_CXX_STANDARD 20)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_EXTENSIONS OFF)

project (6502 C CXX)

SET (FETCHCONTENT_QUIET FALSE)

##########################################################################
# fmt package
##########################################################################

FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(fmt)

##########################################################################
# Sigslot package
##########################################################################

FetchContent_Declare(
    sigslot
    GIT_REPOSITORY https://github.com/palacaze/sigslot
    GIT_TAG v1.2.2
)

FetchContent_MakeAvailable(sigslot)

##########################################################################
# wxWidgets package
##########################################################################

if (WIN32)
  SET (wxBUILD_SHARED OFF  CACHE BOOL "Build wx libraries as shared libs")

  FetchContent_Declare(wxWidgets
      GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
      GIT_TAG v3.2.6
      GIT_SHALLOW ON
      OVERRIDE_FIND_PACKAGE
  )

  FetchContent_GetProperties(wxWidgets)

  if (NOT wxwidgets_POPULATED)
    FetchContent_Populate(wxwidgets)
    add_subdirectory(${wxwidgets_SOURCE_DIR} ${wxwidgets_BUILD_DIR})
  endif ()

  SET (wxLibs wx::base wx::core wx::richtext wx::adv wx::stc wx::aui wx::html wx::xml wx::xrc)
else()
  find_package(wxWidgets 3.2
    COMPONENTS base core richtext adv stc aui html xml xrc
    REQUIRED)

  include ("${wxWidgets_USE_FILE}")

  SET (wxLibs ${wxWidgets_LIBRARIES})
endif()

##########################################################################
# Font Awesome
##########################################################################

download_font_awesome()

##########################################################################
# Project
##########################################################################

if (WIN32)
  SET (execType WIN32)
endif()

add_subdirectory(HexEdit)

file (GLOB_RECURSE SRCS src/*.cpp)
file (GLOB_RECURSE HDRS include/*.h)

add_executable(6502 ${execType} README.md ${SRCS} ${HDRS})

add_dependencies(6502 HexEdit)

configure_file(res/res6502.xrc res6502.xrc COPYONLY)

set_property (TARGET 6502 PROPERTY CXX_STANDARD 20)

target_compile_options(6502 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_include_directories(6502
    PRIVATE include/
    PRIVATE include/crystal
    PRIVATE include/assembler
    PRIVATE ${wxWidgets_SOURCE_DIR}/include
)

target_link_libraries(6502 PRIVATE
    ${wxLibs}
    fmt::fmt
    Pal::Sigslot
    HexEdit)

if (MSVC)
  target_precompile_headers(6502 PRIVATE include/StdAfx.h)
endif ()

