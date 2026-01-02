#/* ********************************************************************************************************************
# * Copyright 2026 Matthew Krueger <contact@matthewkrueger.com>
# * Original Source: https://github.com/Matthew-Krueger/simple-timer
# *
# * Redistribution and use in source and binary forms, with or without modification,
# * are permitted provided that the following conditions are met:
# *
# * 1. Redistributions of source code must retain the above copyright notice,
# * this list of conditions and the following disclaimer.
# *
# * 2. Redistributions in binary form must reproduce the above copyright notice,
# * this list of conditions and the following disclaimer in the documentation
# * and/or other materials provided with the distribution.
# *
# * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
# * or promote products derived from this software without specific prior written permission.
# *
# * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# * AND FITNESSFOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# *********************************************************************************************************************/

cmake_minimum_required(VERSION 3.14)
project(simple-timer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(simple-timer INTERFACE)
add_library(simple-timer::simple-timer ALIAS simple-timer)

target_include_directories(simple-timer INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(simple-timer INTERFACE cxx_std_17)

# Optional MPI support
option(SIMPLE_TIMER_ENABLE_MPI "Enable MPI support in simple-timer" OFF)
if(SIMPLE_TIMER_ENABLE_MPI)
    find_package(MPI REQUIRED)
    target_link_libraries(simple-timer INTERFACE MPI::MPI_CXX)
    target_compile_definitions(simple-timer INTERFACE BUILD_WITH_MPI)
endif()

add_executable(Timer-Demo main.cpp)
target_link_libraries(Timer-Demo simple-timer::simple-timer)