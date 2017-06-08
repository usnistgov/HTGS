# - Try to find libuv
# Once done, this will define
#
#  LIBUV_FOUND - system has libuv
#  LIBUV_INCLUDE_DIRS - the libuv include directories
#  LIBUV_LIBRARIES - link these to use libuv
#
# Set the LIBUV_USE_STATIC variable to specify if static libraries should
# be preferred to shared ones.

if(NOT USE_BUNDLED_LIBUV)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(PC_LIBUV QUIET libuv)
    endif()
else()
    set(PC_LIBUV_INCLUDEDIR)
    set(PC_LIBUV_INCLUDE_DIRS)
    set(PC_LIBUV_LIBDIR)
    set(PC_LIBUV_LIBRARY_DIRS)
    set(LIMIT_SEARCH NO_DEFAULT_PATH)
endif()

find_path(LIBUV_INCLUDE_DIR uv.h
        HINTS ${PC_LIBUV_INCLUDEDIR} ${PC_LIBUV_INCLUDE_DIRS}
        ${LIMIT_SEARCH})

# If we're asked to use static linkage, add libuv.a as a preferred library name.
if(LIBUV_USE_STATIC)
    list(APPEND LIBUV_NAMES
            "${CMAKE_STATIC_LIBRARY_PREFIX}uv${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif(LIBUV_USE_STATIC)

if(MSVC)
    list(APPEND LIBUV_NAMES libuv)
else()
    list(APPEND LIBUV_NAMES uv)
endif()

find_library(LIBUV_LIBRARY NAMES ${LIBUV_NAMES}
        HINTS ${PC_LIBUV_LIBDIR} ${PC_LIBUV_LIBRARY_DIRS}
        ${LIMIT_SEARCH})

mark_as_advanced(LIBUV_INCLUDE_DIR LIBUV_LIBRARY)

if(PC_LIBUV_LIBRARIES)
    list(REMOVE_ITEM PC_LIBUV_LIBRARIES uv)
endif()

set(LIBUV_LIBRARIES ${LIBUV_LIBRARY} ${PC_LIBUV_LIBRARIES})
set(LIBUV_INCLUDE_DIRS ${LIBUV_INCLUDE_DIR})

# Deal with the fact that libuv.pc is missing important dependency information.

include(CheckLibraryExists)

check_library_exists(dl dlopen "dlfcn.h" HAVE_LIBDL)
if(HAVE_LIBDL)
    list(APPEND LIBUV_LIBRARIES dl)
endif()

check_library_exists(kstat kstat_lookup "kstat.h" HAVE_LIBKSTAT)
if(HAVE_LIBKSTAT)
    list(APPEND LIBUV_LIBRARIES kstat)
endif()

check_library_exists(kvm kvm_open "kvm.h" HAVE_LIBKVM)
if(HAVE_LIBKVM AND NOT CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    list(APPEND LIBUV_LIBRARIES kvm)
endif()

check_library_exists(nsl gethostbyname "nsl.h" HAVE_LIBNSL)
if(HAVE_LIBNSL)
    list(APPEND LIBUV_LIBRARIES nsl)
endif()

check_library_exists(perfstat perfstat_cpu "libperfstat.h" HAVE_LIBPERFSTAT)
if(HAVE_LIBPERFSTAT)
    list(APPEND LIBUV_LIBRARIES perfstat)
endif()

check_library_exists(rt clock_gettime "time.h" HAVE_LIBRT)
if(HAVE_LIBRT)
    list(APPEND LIBUV_LIBRARIES rt)
endif()

check_library_exists(sendfile sendfile "" HAVE_LIBSENDFILE)
if(HAVE_LIBSENDFILE)
    list(APPEND LIBUV_LIBRARIES sendfile)
endif()

if(WIN32)
    # check_library_exists() does not work for Win32 API calls in X86 due to name
    # mangling calling conventions
    list(APPEND LIBUV_LIBRARIES iphlpapi)
    list(APPEND LIBUV_LIBRARIES psapi)
    list(APPEND LIBUV_LIBRARIES userenv)
    list(APPEND LIBUV_LIBRARIES ws2_32)
endif()

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBUV_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibUV DEFAULT_MSG
        LIBUV_LIBRARY LIBUV_INCLUDE_DIR)

mark_as_advanced(LIBUV_INCLUDE_DIR LIBUV_LIBRARY)



# - Try to find libuv
# Once done, this will define
#
#  LIBUWS_FOUND - system has libuv
#  LIBUWS_INCLUDE_DIRS - the libuv include directories
#  LIBUWS_LIBRARIES - link these to use libuv
#
# Set the LIBUWS_USE_STATIC variable to specify if static libraries should
# be preferred to shared ones.

if(NOT USE_BUNDLED_LIBUWS)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(PC_LIBUWS QUIET libuWS)
    endif()
else()
    set(PC_LIBUWS_INCLUDEDIR)
    set(PC_LIBUWS_INCLUDE_DIRS)
    set(PC_LIBUWS_LIBDIR)
    set(PC_LIBUWS_LIBRARY_DIRS)
    set(LIMIT_SEARCH NO_DEFAULT_PATH)
endif()

find_path(LIBUWS_INCLUDE_DIR uWS.h
        HINTS ${PC_LIBUWS_INCLUDEDIR} ${PC_LIBUWS_INCLUDE_DIRS}
        ${LIMIT_SEARCH})

# If we're asked to use static linkage, add libuv.a as a preferred library name.
if(LIBUWS_USE_STATIC)
    list(APPEND LIBUWS_NAMES
            "${CMAKE_STATIC_LIBRARY_PREFIX}uWS${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif(LIBUWS_USE_STATIC)

list(APPEND LIBUWS_NAMES uWS)


find_library(LIBUWS_LIBRARY NAMES ${LIBUWS_NAMES}
        HINTS ${PC_LIBUWS_LIBDIR} ${PC_LIBUWS_LIBRARY_DIRS}
        ${LIMIT_SEARCH})

mark_as_advanced(LIBUWS_INCLUDE_DIR LIBUWS_LIBRARY)

if(PC_LIBUWS_LIBRARIES)
    list(REMOVE_ITEM PC_LIBUWS_LIBRARIES uWS)
endif()

set(LIBUWS_LIBRARIES ${LIBUWS_LIBRARY} ${PC_LIBUWS_LIBRARIES})
set(LIBUWS_INCLUDE_DIRS ${LIBUWS_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBUWS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibUWS DEFAULT_MSG
        LIBUWS_LIBRARY LIBUWS_INCLUDE_DIR)

mark_as_advanced(LIBUWS_INCLUDE_DIR LIBUWS_LIBRARY)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)



# - Try to find libuv
# Once done, this will define
#
#  LIBUWS_FOUND - system has libuv
#  LIBUWS_INCLUDE_DIRS - the libuv include directories
#  LIBUWS_LIBRARIES - link these to use libuv
#
# Set the LIBUWS_USE_STATIC variable to specify if static libraries should
# be preferred to shared ones.

if(NOT USE_BUNDLED_LIBUWS)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(PC_LIBUWS QUIET libuWS)
    endif()
else()
    set(PC_LIBUWS_INCLUDEDIR)
    set(PC_LIBUWS_INCLUDE_DIRS)
    set(PC_LIBUWS_LIBDIR)
    set(PC_LIBUWS_LIBRARY_DIRS)
    set(LIMIT_SEARCH NO_DEFAULT_PATH)
endif()

find_path(LIBUWS_INCLUDE_DIR uWS.h
        HINTS ${PC_LIBUWS_INCLUDEDIR} ${PC_LIBUWS_INCLUDE_DIRS}
        ${LIMIT_SEARCH})

# If we're asked to use static linkage, add libuv.a as a preferred library name.
if(LIBUWS_USE_STATIC)
    list(APPEND LIBUWS_NAMES
            "${CMAKE_STATIC_LIBRARY_PREFIX}uWS${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif(LIBUWS_USE_STATIC)

list(APPEND LIBUWS_NAMES uWS)


find_library(LIBUWS_LIBRARY NAMES ${LIBUWS_NAMES}
        HINTS ${PC_LIBUWS_LIBDIR} ${PC_LIBUWS_LIBRARY_DIRS}
        ${LIMIT_SEARCH})

mark_as_advanced(LIBUWS_INCLUDE_DIR LIBUWS_LIBRARY)

if(PC_LIBUWS_LIBRARIES)
    list(REMOVE_ITEM PC_LIBUWS_LIBRARIES uWS)
endif()

set(LIBUWS_LIBRARIES ${LIBUWS_LIBRARY} ${PC_LIBUWS_LIBRARIES})
set(LIBUWS_INCLUDE_DIRS ${LIBUWS_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBUWS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibUWS DEFAULT_MSG
        LIBUWS_LIBRARY LIBUWS_INCLUDE_DIR)

mark_as_advanced(LIBUWS_INCLUDE_DIR LIBUWS_LIBRARY)




# - Try to find libhtgsVisualizer
# Once done, this will define
#
#  LIBHTGS_VISUALIZER_FOUND - system has libuv
#  LIBHTGS_VISUALIZER_INCLUDE_DIRS - the libuv include directories
#  LIBHTGS_VISUALIZER_LIBRARIES - link these to use libuv
#  HTGS_VISUALIZER_DEFINITIONS - Definitions to define to enable profiling
#
# Set the LIBUWS_USE_STATIC variable to specify if static libraries should
# be preferred to shared ones.

if(NOT USE_BUNDLED_LIBHTGS_VISUALIZER)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(PC_LIBHTGS_VISUALIZER QUIET libhtgsVisualizer)
    endif()
else()
    set(PC_LIBHTGS_VISUALIZER_INCLUDEDIR)
    set(PC_LIBHTGS_VISUALIZER_INCLUDE_DIRS)
    set(PC_LIBHTGS_VISUALIZER_LIBDIR)
    set(PC_LIBHTGS_VISUALIZER_LIBRARY_DIRS)
    set(LIMIT_SEARCH NO_DEFAULT_PATH)
endif()

find_path(LIBHTGS_VISUALIZER_INCLUDE_DIR htgs/core/graph/profile/WebSocketProfiler.hpp
        HINTS ${PC_LIBHTGS_VISUALIZER_INCLUDEDIR} ${PC_LIBHTGS_VISUALIZER_INCLUDE_DIRS}
        ${LIMIT_SEARCH})

# If we're asked to use static linkage, add libuv.a as a preferred library name.
if(LIBHTGS_VISUALIZER_USE_STATIC)
    list(APPEND LIBHTGS_VISUALIZER_NAMES
            "${CMAKE_STATIC_LIBRARY_PREFIX}htgsVisualizer${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif(LIBHTGS_VISUALIZER_USE_STATIC)

list(APPEND LIBHTGS_VISUALIZER_NAMES htgsVisualizer)


find_library(LIBHTGS_VISUALIZER_LIBRARY NAMES ${LIBUWS_NAMES}
        HINTS ${PC_LIBHTGS_VISUALIZER_LIBDIR} ${PC_LIBHTGS_VISUALIZER_LIBRARY_DIRS}
        ${LIMIT_SEARCH})

mark_as_advanced(LIBHTGS_VISUALIZER_INCLUDE_DIR LIBUWS_LIBRARY)

if(PC_LIBHTGS_VISUALIZER_LIBRARIES)
    list(REMOVE_ITEM PC_LIBHTGS_VISUALIZER_LIBRARIES htgsVisualizer)
endif()

set(LIBHTGS_VISUALIZER_LIBRARIES ${LIBHTGS_VISUALIZER_LIBRARY} ${PC_LIBHTGS_VISUALIZER_LIBRARIES} ${LIBUWS_LIBRARIES} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${LIB_UV_LIBRARIES})
set(LIBHTGS_VISUALIZER_INCLUDE_DIRS ${LIBHTGS_VISUALIZER_INCLUDE_DIR} ${LIBUWS_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${LIBUV_INCLUDE_DIRS})


include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBUWS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibHTGS_VISUALIZER DEFAULT_MSG
        LIBHTGS_VISUALIZER_LIBRARY LIBHTGS_VISUALIZER_INCLUDE_DIR)

if(LIBHTGS_VISUALIZER_FOUND)
    set(HTGS_VISUALIZER_DEFINITIONS -DWS_PROFILE -DPROFILE)
endif(LIBHTGS_VISUALIZER_FOUND)

mark_as_advanced(LIBHTGS_VISUALIZER_INCLUDE_DIR LIBHTGS_VISUALIZER_LIBRARY)
