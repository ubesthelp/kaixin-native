# 查询 IXWebSocket 库
find_path(IXWebSocket_INCLUDE_DIR "ixwebsocket/IXWebSocket.h")
find_library(IXWebSocket_LIBRARY "ixwebsocket")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IXWebSocket DEFAULT_MSG
    IXWebSocket_LIBRARY
    IXWebSocket_INCLUDE_DIR
)
mark_as_advanced(
    IXWebSocket_LIBRARY
    IXWebSocket_INCLUDE_DIR
)

if(IXWebSocket_FOUND AND NOT TARGET IXWebSocket)
    find_package(ZLIB REQUIRED)
    find_package(OpenSSL REQUIRED)
    add_library(IXWebSocket UNKNOWN IMPORTED)
    set_target_properties(IXWebSocket PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${IXWebSocket_INCLUDE_DIR}"
        IMPORTED_LOCATION "${IXWebSocket_LIBRARY}"
        INTERFACE_LINK_LIBRARIES "ZLIB::ZLIB;OpenSSL::SSL;OpenSSL::Crypto"
    )

    if(WIN32)
        set_property(TARGET IXWebSocket APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES "crypt32.lib;shlwapi.lib;ws2_32.lib")
    endif()
endif()
