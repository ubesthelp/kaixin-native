set(target kaixin)
add_library(${target} SHARED
    kaixin.h kaixin.cpp
)

find_package(RapidJSON CONFIG REQUIRED)

set_target_properties(${target} PROPERTIES
    DEFINE_SYMBOL "KAIXIN_EXPORTS"
)
target_compile_definitions(${target} PRIVATE "RAPIDJSON_HAS_STDSTRING=1")
target_include_directories(${target}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${RAPIDJSON_INCLUDE_DIRS}
)
target_precompile_headers(${target} PRIVATE "<rapidjson/document.h>")
#target_link_libraries(${target} PUBLIC Qt5::Widgets Qt5::Network Qt5::WebSockets crypto QNtp QtXlsx Sentrypad Themida crypt32.lib
#    PRIVATE Qt5::GuiPrivate
#)

if(MSVC)
    target_compile_definitions(${target} PRIVATE "WIN32_LEAN_AND_MEAN")
    target_compile_options(${target} PRIVATE ${PROJECT_WARNING_FLAGS} "/utf-8")
endif()
