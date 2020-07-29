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

target_link_libraries(${target} PRIVATE
    kaixin
)
common_config(${target})
