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
    rapidjsonhelpers.h
)

# 查找依赖库
# RapidJSON
find_package(RapidJSON CONFIG REQUIRED)

# ixwebsocket
find_path(IXWEBSOCKET_INCLUDE_DIR ixwebsocket/IXWebSocket.h)
find_library(IXWEBSOCKET_LIBRARY ixwebsocket)

# 设置编译选项，链接依赖库
set_target_properties(${target} PROPERTIES
    DEFINE_SYMBOL "KAIXIN_EXPORTS"
)
target_compile_features(${target} PRIVATE cxx_std_17)
target_compile_definitions(${target} PRIVATE "RAPIDJSON_HAS_STDSTRING=1")
target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${RAPIDJSON_INCLUDE_DIRS} ${IXWEBSOCKET_INCLUDE_DIR}
)
target_precompile_headers(${target} PRIVATE
    "<ixwebsocket/IXHttpClient.h>"
    "<ixwebsocket/IXWebSocket.h>"
    "<rapidjson/document.h>"
)
target_link_libraries(${target} PRIVATE
    ${IXWEBSOCKET_LIBRARY}
)

# MSVC 特殊设置
if(MSVC)
    target_compile_definitions(${target} PRIVATE "WIN32_LEAN_AND_MEAN")
    target_compile_options(${target} PRIVATE ${PROJECT_WARNING_FLAGS} "/utf-8")
endif()
