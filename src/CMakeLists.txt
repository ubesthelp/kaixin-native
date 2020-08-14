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
add_library(${target} SHARED
    kaixin.h kaixin.cpp
    kaixin_api.h kaixin_api.cpp
    rapidjsonhelpers.h
    utils.h utils.cpp
    jwt.h jwt.cpp
    fingerprint.h fingerprint.cpp
)

if(WIN32)
    target_sources(${target} PRIVATE wmi_client.h wmi_client.cpp)
endif()

# 查找依赖库
# RapidJSON
find_package(RapidJSON CONFIG REQUIRED)

# ixwebsocket
find_package(IXWebSocket REQUIRED)

# cppcodec
find_package(CPPCODEC REQUIRED)

# 设置编译选项，链接依赖库
set_target_properties(${target} PROPERTIES
    DEFINE_SYMBOL "KAIXIN_EXPORTS"
)
target_compile_features(${target} PRIVATE cxx_std_17)
target_compile_definitions(${target} PRIVATE "RAPIDJSON_HAS_STDSTRING=1")
target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${RapidJSON_INCLUDE_DIRS} ${IXWEBSOCKET_INCLUDE_DIR}
)
target_precompile_headers(${target} PRIVATE
    "<map>" "<string>" "<vector>"
    "<ixwebsocket/IXHttpClient.h>" "<ixwebsocket/IXWebSocket.h>"
    "<rapidjson/document.h>"
)
target_link_libraries(${target} PRIVATE
    IXWebSocket
    CPPCODEC
)

option(KAIXIN_BUILD_INTERNAL_API "Build the internal APIs." OFF)

if(KAIXIN_BUILD_INTERNAL_API)
    target_sources(${target} PRIVATE kaixin_internal.h kaixin_internal.cpp)
    target_compile_definitions(${target} PUBLIC "KAIXIN_BUILD_INTERNAL_API")
endif()

common_config(${target})
