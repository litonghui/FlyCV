include(ExternalProject)

set(ZLIB_NAME zlib)
set(ZLIB_WORK_DIR ${PROJECT_SOURCE_DIR}/third_party)
set(ZLIB_BUILD_DIR ${PROJECT_BINARY_DIR}/third_party/${ZLIB_NAME})
set(ZLIB_INSTALL_DIR ${ZLIB_BUILD_DIR}/output)

set(ZLIB_CMAKE_ARGS)
list(APPEND ZLIB_CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${ZLIB_INSTALL_DIR}
        -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release)

if(NOT APPLE)
    list(APPEND LIBJPEG_CMAKE_ARGS
        -DCMAKE_CXX_FLAGS:STRING="-fPIC"
        -DCMAKE_CXX_FLAGS:STRING="-w"
        -DCMAKE_C_FLAGS:STRING="-fPIC"
        -DCMAKE_C_FLAGS:STRING="-w")
endif()

if(ANDROID)
    list(APPEND ZLIB_CMAKE_ARGS
            -DANDROID_ABI=${ANDROID_ABI}
            -DANDROID_PLATFORM=${ANDROID_PLATFORM}
            -DANDROID_ARM_NEON=${ANDROID_ARM_NEON})
elseif(APPLE)
    list(APPEND ZLIB_CMAKE_ARGS -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES})
elseif(UNIX)
    list(APPEND ZLIB_CMAKE_ARGS -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR})
endif()

fcv_download_dependency(
    "https://github.com/madler/zlib.git"
    v1.2.9
    ${ZLIB_NAME}
    ${ZLIB_WORK_DIR}
    )

ExternalProject_Add(
    ${ZLIB_NAME}
    PREFIX ${ZLIB_WORK_DIR}/${ZLIB_NAME}
    CMAKE_ARGS ${ZLIB_CMAKE_ARGS}
    SOURCE_DIR ${ZLIB_WORK_DIR}/${ZLIB_NAME}
    TMP_DIR ${ZLIB_BUILD_DIR}/tmp
    BINARY_DIR ${ZLIB_BUILD_DIR}
    STAMP_DIR ${ZLIB_BUILD_DIR}/stamp
)

add_library(fcv_zlib STATIC IMPORTED)

if(WIN32)
    set(ZLIB_LIB_NAME "zlibstatic.lib")
else()
    set(ZLIB_LIB_NAME "libz.a")
endif()

set_property(TARGET fcv_zlib
        PROPERTY IMPORTED_LOCATION
        ${ZLIB_INSTALL_DIR}/lib/${ZLIB_LIB_NAME})

include_directories(${ZLIB_INSTALL_DIR}/include)
list(APPEND FCV_LINK_DEPS fcv_zlib)

if(NOT BUILD_SHARED_LIBS)
    list(APPEND FCV_EXPORT_LIBS ${ZLIB_INSTALL_DIR}/lib/${ZLIB_LIB_NAME})
endif()

list(APPEND FCV_EXTERNAL_DEPS ${ZLIB_NAME})