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
    add_library(IXWebSocket INTERFACE)
    target_include_directories(IXWebSocket SYSTEM INTERFACE "${IXWebSocket_INCLUDE_DIR}")
    target_link_libraries(IXWebSocket INTERFACE "${IXWebSocket_LIBRARY}" ZLIB::ZLIB)

    find_package(MbedTLS)
    
    if(MbedTLS_FOUND)
        target_link_libraries(IXWebSocket INTERFACE MbedTLS::TLS MbedTLS::CRYPTO MbedTLS::X509)
    else()
        find_package(OpenSSL)
        
        if(OpenSSL_FOUND)
            target_link_libraries(IXWebSocket INTERFACE OpenSSL::SSL OpenSSL::Crypto)
        else()
            message(WARNING "No SSL library found.")
        endif()
    endif()

endif()
