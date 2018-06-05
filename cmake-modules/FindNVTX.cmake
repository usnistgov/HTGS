# Locates the NVTX library and include directory.

include(FindPackageHandleStandardArgs)
unset(NVTX_FOUND)

# typical root dirs of installations, exactly one of them is used



SET(NVTX_POSSIBLE_ROOT_DIRS
        "${NVTX_ROOT_DIR}"
        "$ENV{NVTX_ROOT_DIR}"
        "/usr/local"
        "/usr"
        "/opt/local"
        "/usr/local/cuda"
        )

FIND_PATH(NVTX_ROOT_DIR
        NAMES
        nvToolsExt.h #nvtx3/nvToolsExt.h # linux /opt/net
        PATHS ${NVTX_POSSIBLE_ROOT_DIRS})

message(STATUS "Found NVTX ROOT DIR ${NVTX_ROOT_DIR}")

FIND_PATH(NVTX_INCLUDE_DIR
        NAMES
        "nvToolsExt.h"
        PATHS ${NVTX_ROOT_DIR}
        PATH_SUFFIXES include
        )

find_library(NVTX_LIBRARY
        NAMES nvToolsExt
        PATHS ${NVTX_ROOT_DIR}
        HINTS
        /usr/lib
        /usr/local/lib
        /usr/local/cuda/lib64)


# set NVTX_FOUND
find_package_handle_standard_args(NVTX DEFAULT_MSG NVTX_INCLUDE_DIR NVTX_LIBRARY)
#find_package_handle_standard_args(NVTX DEFAULT_MSG NVTX_INCLUDE_DIR)

# set external variables for usage in CMakeLists.txt
if(NVTX_FOUND)
    set(NVTX_FOUND true)
    set(NVTX_LIBRARY ${NVTX_LIBRARY})
    set(NVTX_INCLUDE_DIR ${NVTX_INCLUDE_DIR})
#    set(NVTX_LIBRARY "-pthread -ldl -lrt")
    MESSAGE(STATUS "NVTX found. Includes: ${NVTX_INCLUDE_DIR}, Libs: ${NVTX_LIBRARY}")
#    MESSAGE(STATUS "NVTX found. Includes: ${NVTX_INCLUDE_DIR}")
else()
    MESSAGE(STATUS "NVTX not found. Includes: ${NVTX_INCLUDE_DIR}")
#    MESSAGE(STATUS "NVTX not found. Includes: ${NVTX_INCLUDE_DIR}, Libs: ${NVTX_LIBRARY}")
endif()

# hide locals from GUI
#mark_as_advanced(NVTX_INCLUDE_DIR NVTX_LIBRARY)
mark_as_advanced(NVTX_INCLUDE_DIR)
