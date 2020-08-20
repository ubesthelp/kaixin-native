###################################################################################################
#
# \file        CMakeLists.txt
# \brief       开心 C SDK CMakeLists。
#
# \version     0.1
# \date        2020-7-28
#
# \author      Roy QIU <karoyqiu@gmail.com>
# \copyright   © 2020 开心网络。
#
###################################################################################################

# 添加项目
set(target kaixin)
add_library(${target}
    authorization_disabler.h
    fingerprint.h fingerprint.cpp
    jwt.h jwt.cpp
    kaixin.h kaixin.cpp
    kaixin_api.h kaixin_api.cpp
    noncopyable.h
    rapidjsonhelpers.h
    simple_timer.h simple_timer.cpp
    utils.h utils.cpp
    websocket_client.h websocket_client.cpp
)

if(WIN32)
    target_sources(${target} PRIVATE wmi_client.h wmi_client.cpp)
endif()

# 生成导出头文件
include(GenerateExportHeader)
generate_export_header(${target})

# 查找依赖库
# RapidJSON
find_package(RapidJSON CONFIG REQUIRED)

# ixwebsocket
find_package(IXWebSocket REQUIRED)

# cppcodec
find_package(CPPCODEC REQUIRED)

# 设置编译选项，链接依赖库
set_target_properties(${target} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_compile_features(${target} PRIVATE cxx_std_17)
target_compile_definitions(${target} PRIVATE "RAPIDJSON_HAS_STDSTRING=1")
target_compile_options(${target} PRIVATE ${PROJECT_WARNING_FLAGS})
target_include_directories(${target} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR} ${RapidJSON_INCLUDE_DIRS} ${IXWEBSOCKET_INCLUDE_DIR}
)
target_precompile_headers(${target} PRIVATE
    "<map>" "<string>" "<vector>"
    "<ixwebsocket/IXHttpClient.h>" "<ixwebsocket/IXWebSocket.h>"
    "<rapidjson/document.h>"
)
target_link_libraries(${target}
    IXWebSocket
    CPPCODEC
)

option(KAIXIN_BUILD_INTERNAL_API "Build the internal APIs." OFF)

if(KAIXIN_BUILD_INTERNAL_API)
    target_sources(${target} PRIVATE kaixin_internal.h kaixin_internal.cpp)
    target_compile_definitions(${target} PUBLIC "KAIXIN_BUILD_INTERNAL_API")
endif()

if(BUILD_SHARED_LIBS)
    if(MSVC)
        configure_file(kaixin.rc.in kaixin.rc @ONLY)
        target_sources(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/kaixin.rc")
    endif()
else()
    target_compile_definitions(${target} PUBLIC "KAIXIN_STATIC_DEFINE")
endif()

# Windows 特殊设置
if(WIN32)
    target_compile_definitions(${target} PRIVATE "WIN32_LEAN_AND_MEAN")
    target_link_libraries(${target} "crypt32" "shlwapi" "ws2_32")
endif()

# MSVC 特殊设置
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("/utf-8" HAVE_UTF_8)

if(HAVE_UTF_8)
    target_compile_options(${target} PRIVATE "/utf-8")
endif()


# 安装
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
configure_package_config_file("${target}Config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/${target}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${target}"
)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${target}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(TARGETS ${target} EXPORT ${target}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES "kaixin.h" "${CMAKE_CURRENT_BINARY_DIR}/kaixin_export.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${target}Config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/${target}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${target}"
)
install(EXPORT ${target}Targets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${target}"
    NAMESPACE "${target}::"
    FILE ${target}Targets.cmake
)
