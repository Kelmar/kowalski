cmake_minimum_required(VERSION 3.20)

project (6502 C CXX)

if (WIN32)
  include(FetchContent)

  Set (FETCHCONTENT_QUIET FALSE)

  FetchContent_Declare(wxWidgets
      GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
      GIT_TAG v3.2.4
      OVERRIDE_FIND_PACKAGE
  )

  FetchContent_GetProperties(wxWidgets)

  if (NOT wxwidgets_POPULATED)
    FetchContent_Populate(wxwidgets)
    add_subdirectory(${wxwidgets_SOURCE_DIR} ${wxwidgets_BUILD_DIR})
  endif ()

  set (wxLibs wx::base wx::core)
else ()
  find_package(wxWidgets COMPONENTS base core REQUIRED)
  include("${wxWidgets_USE_FILE}")

  set (wxLibs ${wxWidgets_LIBRARIES})
endif ()

file (GLOB_RECURSE SRCS src/*.cpp)
file (GLOB_RECURSE HDRS include/*.h)

add_executable(6502 ${SRCS} ${HDRS})

target_include_directories(6502
    PRIVATE include/
    PRIVATE include/crystal
    PRIVATE ${wxWidgets_SOURCE_DIR}/include
)

target_link_libraries(6502 PRIVATE ${wxLibs})
