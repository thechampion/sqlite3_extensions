cmake_minimum_required(VERSION 3.4)

project(sqlite3_extensions LANGUAGES C)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(OPENSSL openssl)
pkg_check_modules(LIBURIPARSER liburiparser)


set(BUILD_HASHLIB ${OPENSSL_FOUND})
set(BUILD_URIPARSE ${LIBURIPARSER_FOUND})

if(${BUILD_HASHLIB})
    add_library(hashlib MODULE hashlib.c)
    set_target_properties(hashlib PROPERTIES PREFIX "" OUTPUT_NAME hashlib)
    target_link_libraries(hashlib ${OPENSSL_LIBRARIES})
else()
    message(WARNING "openssl is not found, won't build hashlib extension")
endif()

if(${LIBURIPARSER_FOUND})
    add_library(uriparse MODULE uriparse.c)
    set_target_properties(uriparse PROPERTIES PREFIX "" OUTPUT_NAME uriparse)
    target_link_libraries(uriparse ${LIBURIPARSER_LIBRARIES})
else()
    message(WARNING "uriparse is not found, won't build uriparse extension")
endif()
