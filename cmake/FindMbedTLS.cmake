# 查询 Mbed TLS 库
find_path(MbedTLS_INCLUDE_DIR "mbedtls/ssl.h")
find_library(MbedTLS_TLS_LIBRARY "mbedtls")
find_library(MbedTLS_CRYPTO_LIBRARY "mbedcrypto")
find_library(MbedTLS_X509_LIBRARY "mbedx509")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MbedTLS DEFAULT_MSG
    MbedTLS_TLS_LIBRARY
    MbedTLS_CRYPTO_LIBRARY
    MbedTLS_X509_LIBRARY
    MbedTLS_INCLUDE_DIR
)
mark_as_advanced(
    MbedTLS_TLS_LIBRARY
    MbedTLS_CRYPTO_LIBRARY
    MbedTLS_X509_LIBRARY
    MbedTLS_INCLUDE_DIR
)

if(MbedTLS_FOUND)
    if(NOT TARGET MbedTLS::TLS)
        add_library(MbedTLS::TLS UNKNOWN IMPORTED)
        set_target_properties(MbedTLS::TLS PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${MbedTLS_INCLUDE_DIR}")
        set_target_properties(MbedTLS::TLS PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${MbedTLS_TLS_LIBRARY}")
    endif()
    
    if(NOT TARGET MbedTLS::CRYPTO)
        add_library(MbedTLS::CRYPTO UNKNOWN IMPORTED)
        set_target_properties(MbedTLS::CRYPTO PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${MbedTLS_INCLUDE_DIR}")
        set_target_properties(MbedTLS::CRYPTO PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${MbedTLS_CRYPTO_LIBRARY}")

        if(WIN32)
            set_property(TARGET MbedTLS::CRYPTO APPEND PROPERTY INTERFACE_LINK_LIBRARIES crypt32.lib)
        endif()
    endif()

    if(NOT TARGET MbedTLS::X509)
        add_library(MbedTLS::X509 UNKNOWN IMPORTED)
        set_target_properties(MbedTLS::X509 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${MbedTLS_INCLUDE_DIR}")
        set_target_properties(MbedTLS::X509 PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${MbedTLS_X509_LIBRARY}")
    endif()
endif()
