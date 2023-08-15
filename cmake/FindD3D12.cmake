# DirectXShaderCompiler LICENSE
# 
# ==============================================================================
# LLVM Release License
# ==============================================================================
# University of Illinois/NCSA
# Open Source License
# 
# Copyright (c) 2003-2015 University of Illinois at Urbana-Champaign.
# All rights reserved.
# 
# Developed by:
# 
#     LLVM Team
# 
#     University of Illinois at Urbana-Champaign
# 
#     http://llvm.org
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal with
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
# 
#     * Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimers.
# 
#     * Redistributions in binary form must reproduce the above copyright notice,
#       this list of conditions and the following disclaimers in the
#       documentation and/or other materials provided with the distribution.
# 
#     * Neither the names of the LLVM Team, University of Illinois at
#       Urbana-Champaign, nor the names of its contributors may be used to
#       endorse or promote products derived from this Software without specific
#       prior written permission.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
# SOFTWARE.
#
# Find D3D12 SDK aka Windows 10 SDK.
# Set environment variable WIN10_SDK_PATH and WIN10_SDK_VERSION to customize Windows 10 SDK version.
#
# Variables:
#   D3D12_INCLUDE_DIRS
#   D3D12_LIBRARIES
#   D3D12_DXC - Path to dxc.exe
#   D3D12_FXC - Path to fxc.exe
#
 
set(WIN10_SDK_PATH)
set(WIN10_SDK_VERSION)

# Checks if the specified Windows 10 SDK exists.
function(inkCheckWin10SDK BUILD_VER)
    get_filename_component(WIN10_SDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
    if(NOT WIN10_SDK_PATH)
        return() # not found.
    endif()
    if(EXISTS "${WIN10_SDK_PATH}/Include/${BUILD_VER}.0/um")
        set(WIN10_SDK_FOUND TRUE PARENT_SCOPE)
    endif()
endfunction(inkCheckWin10SDK)

# Find the Win10 SDK path.
if("$ENV{WIN10_SDK_PATH}" STREQUAL "")
    get_filename_component(WIN10_SDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
else()
    set(WIN10_SDK_PATH $ENV{WIN10_SDK_PATH})
endif()

if("$ENV{WIN10_SDK_VERSION}" STREQUAL "")
    if(${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
        set(WIN10_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
    else()
        # Find Windows10 SDK
        set(
            WIN10_SDK_VERSION_LIST
            10.0.22000
            10.0.20348
            10.0.19041
            10.0.18362 # Win10 1903 "19H1"
            10.0.17763 # Win10 1809 "October 2018 Update"
            # Older versions are not supported.
        )

        foreach(SDKVER ${WIN10_SDK_VERSION_LIST})
            inkCheckWin10SDK(${SDKVER})
            if(WIN10_SDK_FOUND)
                set(WIN10_SDK_VERSION ${SDKVER})
                break()
            endif()
        endforeach()
    endif()
else()
    set(WIN10_SDK_VERSION $ENV{WIN10_SDK_VERSION})
endif()

# WIN10_SDK_PATH will be something like C:\Program Files (x86)\Windows Kits\10
# WIN10_SDK_VERSION will be something like 10.0.14393 or 10.0.14393.0; we need the
# one that matches the directory name.

if(IS_DIRECTORY "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}.0")
    set(WIN10_SDK_VERSION "${WIN10_SDK_VERSION}.0")
endif(IS_DIRECTORY "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}.0")

# Find the d3d12 and dxgi include path, it will typically look something like this.
# C:\Program Files (x86)\Windows Kits\10\Include\10.0.10586.0\um\d3d12.h
# C:\Program Files (x86)\Windows Kits\10\Include\10.0.10586.0\shared\dxgi1_4.h
find_path(
    D3D12_INCLUDE_DIR       # Set variable D3D12_INCLUDE_DIR
    "d3d12.h"               # Find a path with d3d12.h
    HINTS "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}/um"
    DOC "path to WIN10 SDK header files"
    HINTS
)

find_path(
    DXGI_INCLUDE_DIR        # Set variable DXGI_INCLUDE_DIR
    "dxgi1_6.h"             # Find a path with dxgi1_6.h
    HINTS "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}/shared"
    DOC "path to WIN10 SDK header files"
    HINTS
)

set(D3D12_INCLUDE_DIRS ${D3D12_INCLUDE_DIR} ${DXGI_INCLUDE_DIR})

find_path(
    WIN10_SDK_UTILS_X64_DIR # Set variable WIN10_SDK_UTILS_X64_DIR
    "dxc.exe"               # Find a path with dxc.exe
    HINTS "${WIN10_SDK_PATH}/bin/${WIN10_SDK_VERSION}/x64"
    DOC "path to WIN10 SDK utilities."
)

find_path(
    WIN10_SDK_UTILS_X86_DIR # Set variable WIN10_SDK_UTILS_X86_DIR
    "dxc.exe"               # Find a path with dxc.exe
    HINTS "${WIN10_SDK_PATH}/bin/${WIN10_SDK_VERSION}/x86"
    DOC "path to WIN10 SDK utilities."
)

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "AMD64")
    set(WIN10_SDK_UTILS_DIR ${WIN10_SDK_UTILS_X64_DIR})
else()
    set(WIN10_SDK_UTILS_DIR ${WIN10_SDK_UTILS_X86_DIR})
endif()

if(NOT WIN10_SDK_UTILS_DIR)
    message(FATAL_ERROR "Could not find WIN10 SDK utilities.")
endif()

set(D3D12_DXC "${WIN10_SDK_UTILS_DIR}/dxc.exe")
set(D3D12_FXC "${WIN10_SDK_UTILS_DIR}/fxc.exe")

# List of D3D libraries
set(D3D12_LIBRARIES "d3d12.lib" "dxgi.lib" "d3dcompiler.lib")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set D3D12_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
    D3D12 DEFAULT_MSG
    D3D12_INCLUDE_DIRS D3D12_LIBRARIES D3D12_DXC D3D12_FXC
)

mark_as_advanced(D3D12_INCLUDE_DIRS D3D12_LIBRARIES D3D12_DXC D3D12_FXC)
