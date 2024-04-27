cmake_minimum_required(VERSION 3.20)

if (WIN32)
  include(FetchContent)

  FetchContent_Declare(
      wxWidgets
      GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
  )

  FetchContent_MakeAvailable(wxWidgets)
else ()
  find_package(wxWidgets COMPONENTS core base REQUIRED)
endif (WIN32)

project (6502 C CXX)

include_directories(
    include/
    include/crystal
)

if (WIN32)
else ()
  include("${wxWidgets_USE_FILE}")
endif (WIN32)

file (GLOB_RECURSE SRCS src/*.cpp)

list(APPEND SOURCES ${SRCS})

add_executable(6502 ${SRCS})

target_link_libraries(6502 ${wxWidgets_LIBRARIES})

