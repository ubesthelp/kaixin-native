# 查询 CPPCODEC 库
find_path(CPPCODEC_INCLUDE_DIR "cppcodec/base32_crockford.hpp")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CPPCODEC DEFAULT_MSG
    CPPCODEC_INCLUDE_DIR
)
mark_as_advanced(
    CPPCODEC_INCLUDE_DIR
)

if(CPPCODEC_FOUND AND NOT TARGET CPPCODEC)
    add_library(CPPCODEC INTERFACE IMPORTED)
    set_target_properties(IXWebSocket PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CPPCODEC_INCLUDE_DIR}"
    )
endif()
