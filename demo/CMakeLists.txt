###################################################################################################
#
# \file        CMakeLists.txt
# \brief       开心 C SDK 示例程序 CMakeLists。
#
# \version     0.1
# \date        2020-7-28
#
# \author      Roy QIU <karoyqiu@gmail.com>
# \copyright   © 2020 开心网络。
#
###################################################################################################

# 添加项目
set(target kaixin-demo)
add_executable(${target}
    main.cpp appkey.h
)
target_include_directories(${target} PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_BINARY_DIR}/src"
)
target_link_libraries(${target} PRIVATE
    kaixin
)
