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

cmake_minimum_required(VERSION 3.20)

##########################################################################

project (core C CXX)

SET (SRCS
    "src/core.cpp"
)

SET (HDRS
    "publish/core_sig.h"
    "publish/deferred.h"
)

##########################################################################

add_library(core ${SRCS} ${HDRS})

set_property (TARGET core PROPERTY CXX_STANDARD 20)

target_compile_options(core PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_include_directories(core
    PRIVATE "include/"
    PRIVATE "../include"
    PUBLIC "publish/"
)

target_link_libraries(core PRIVATE fmt::fmt)

##########################################################################
