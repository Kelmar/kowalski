cmake_minimum_required(VERSION 3.20)

include (fontawesome.cmake)
include(FetchContent)

SET (CMAKE_CXX_STANDARD 20)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_EXTENSIONS OFF)

project (6502 C CXX)

SET (FETCHCONTENT_QUIET FALSE)

##############
# fmt package
##############

FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(fmt)

##################
# Sigslot package
##################

FetchContent_Declare(
    sigslot
    GIT_REPOSITORY https://github.com/palacaze/sigslot
    GIT_TAG v1.2.2
)

FetchContent_MakeAvailable(sigslot)

####################
# wxWidgets package
####################

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

if (WIN32)
  SET (execType WIN32)
endif()

file (GLOB_RECURSE SRCS src/*.cpp)
file (GLOB_RECURSE HDRS include/*.h)

download_font_awesome()

add_executable(6502 ${execType} README.md ${SRCS} ${HDRS})

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

target_link_libraries(6502 PRIVATE ${wxLibs}
    fmt::fmt
    Pal::Sigslot)

if (MSVC)
  target_precompile_headers(6502 PRIVATE include/StdAfx.h)
endif ()

