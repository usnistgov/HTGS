Using Nsight Systems with HTGS {#using-nsight-systems}
=======

The HTGS API supports visualizing a task graph using the Nsight Systems visualization tool. [Nsight Systems](https://developer.nvidia.com/nsight-systems)
is a powerful tool to show a timeline of the execution of a process and visualize how HTGS tasks and graphs interact.

## Dependencies: ## 

1. The NVTX3 header-only API
2. Lib dl 

## Compilation Instructions: ##

To enable NVTX profiling add the following directive during compilation (for graphs with less than 25 tasks):
~~~~~~~~~~~~~~~~~~~~~
-DUSE_NVTX
~~~~~~~~~~~~~~~~~~~~~

To profile graphs with more than 24 tasks add the following directives during compilation:
~~~~~~~~~~~~~~~~~~~~~
-DUSE_NVTX
-DUSE_MINIMAL_NVTX
~~~~~~~~~~~~~~~~~~~~~

For convenience, we have included the [FindNVTX.cmake](https://github.com/usnistgov/HTGS/blob/master/cmake-modules/FindNVTX.cmake) cmake module example usage:
~~~~~~~~~~~~~~~~~~~~~
    find_package(NVTX QUIET)

    if(NVTX_FOUND)
        message(STATUS "FOUND NVTX!")
        add_definitions(-DUSE_NVTX)
        include_directories(${NVTX_INCLUDE_DIR})
        link_libraries(${NVTX_LIBRARIES})
    endif()
~~~~~~~~~~~~~~~~~~~~~

With the FindNVXT.cmake script, you can specify the NVTX_INCLUDE_DIR when reloading cmake.


## How to run with Nsight Systems ##

Within Nsight Systems there are two tools:
1. The visualization program **nsight-sys**
2. The profiling execution program **nsys** or **quadd_d**

After compiling HTGS with NVTX enabled, you can profile your application with the following command:
~~~~~~~~~~~~~~~~~~~~~
nsys profile -t nvtx -s cpu -d <time in seconds to profile> -w <true/false to show output> </path/to/executable>
 
nsys profile -t nvtx -s cpu -d 5 -w true /path/to/executable
~~~~~~~~~~~~~~~~~~~~~
 
By default, this will generate a report file in $HOME$/nvidia_nsight_systems/report#.qdsrtm.
 
This file can then be imported using **nsight-sys** to visualize.