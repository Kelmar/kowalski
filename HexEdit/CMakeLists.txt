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

##########################################################################

project (HexEdit C CXX)

SET (SRCS
    "src/HexEdit.cpp"
    "src/DrawMetrics.cpp"
    "src/OffsetColumns.cpp"
    "src/BaseAddressRows.cpp"
    "src/HexArea.cpp")

SET (HDRS
    "../include/HexEdit.h"
    "include/he_priv.h"
    "include/DrawMetrics.h"
    "include/SubControl.h"
    "include/OffsetColumns.h"
    "include/BaseAddressRows.h"
    "include/HexArea.h")

add_library(HexEdit ${SRCS} ${HDRS} "include/he_priv.h" "include/HexArea.h" "src/HexArea.cpp")

set_property (TARGET HexEdit PROPERTY CXX_STANDARD 20)

target_compile_options(HexEdit PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_include_directories(HexEdit
    PRIVATE include/
    PRIVATE ../include/
    PRIVATE ${wxWidgets_SOURCE_DIR}/include
)

target_link_libraries(HexEdit PRIVATE ${wxLibs}
    fmt::fmt
    Pal::Sigslot)

##########################################################################
