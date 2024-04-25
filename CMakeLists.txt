cmake_minimum_required(VERSION 3.20)

find_package(wxWidgets REQUIRED core base)

project (6502 C CXX)

include(${wxWidgets_USE_FILE})

include_directories(
    include/
    include/crystal
)

file (GLOB_RECURSE SRCS src/*.cpp)

list(APPEND SOURCES ${SRCS})

add_definitions(-D_AFXDLL)
set(CMAKE_MFC_FLAG 2)
add_executable(6502 ${SRCS})

target_link_libraries(6502 ${wxWidgets_LIBRARIES})
