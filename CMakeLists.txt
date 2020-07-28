###################################################################################################
#
# \file        CMakeLists.txt
# \brief       开心 C SDK 主 CMakeLists。
#
# \version     0.1
# \date        2020-7-28
#
# \author      Roy QIU <karoyqiu@gmail.com>
# \copyright   © 2020 开心网络。
#
###################################################################################################
cmake_minimum_required(VERSION 3.16)

# 设置全局变量
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_POLICY_DEFAULT_CMP0001 NEW)
#set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
#set(CMAKE_POLICY_DEFAULT_CMP0054 NEW)
#set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
#set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
#set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)

set(CMAKE_BUILD_TYPE Debug CACHE STRING
    "Choose the type of build, options are: Debug Release.")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_DEFAULT_TRIPLET} CACHE STRING "VCPKG Target Triplet to use")

# 开心 C SDK
project(kaixin VERSION 0.0.1)

# 检查是否支持 IPO
if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()
include(CheckIPOSupported OPTIONAL)
if(COMMAND check_ipo_supported)
    check_ipo_supported(RESULT PROJECT_IS_IPO_SUPPORTED)

    if(PROJECT_IS_IPO_SUPPORTED)
        option(PROJECT_ENABLE_IPO "Enable IPO for non-debug build." OFF)
    endif()
endif()

if(PROJECT_ENABLE_IPO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE OFF)
endif()

# 判断目标架构
include(TargetArch)
target_architecture(PROJECT_TARGET_ARCH)

# 检查编译器警告选项
include(WarningFlags)
check_warning_flags(PROJECT_WARNING_FLAGS)

# Themida
#find_package(Themida REQUIRED)

# 源代码
add_subdirectory("src")

# 打包，提前设置供安装库使用。
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-all")
set(CPACK_PACKAGE_VENDOR "karoyqiu@gmail.com")
set(CPACK_PACKAGE_CONTACT "karoyqiu@gmail.com")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CHECKSUM SHA256)
set(CPACK_VERBATIM_VARIABLES ON)
set(CPACK_STRIP_FILES ON)
set(CPACK_GENERATOR "ZIP")
include(CPack)
