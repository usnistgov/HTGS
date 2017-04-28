Getting Started {#install-instructions}
=======

The HTGS API is a header-only library. To compile using HTGS use the following compilation flags:
~~~~~~~~~~~~~~~~~~~~~
-std=c++11 -pthread -I$HTGS_INSTALLATION_DIR$
~~~~~~~~~~~~~~~~~~~~~

To enable CUDA compilation use the directive (must have CUDA installed):
~~~~~~~~~~~~~~~~~~~~~
-DUSE_CUDA
~~~~~~~~~~~~~~~~~~~~~

Installation and documentation for CUDA can be found [here](http://docs.nvidia.com/cuda/index.html#axzz4eWVEar2N)

## Dependencies: ##
1. cmake v2.7+
2. gcc/g++ v4.8.x (or higher, C++11)
3. pthreads
4. (Optional) [Doxygen](http://www.stack.nl/~dimitri/doxygen/)
5. (Optional) [Graphviz](http://www.graphviz.org/)

## CMake Options: ##
 CMAKE_INSTALL_PREFIX - Where to install HTGS (and documentation)

 BUILD_DOXYGEN - Creates doxygen documentation (view online at: Documentation)

 RUN_GTEST - Compiles and runs google unit tests for HTGS ('make run-test' to re-run)

~~~~~~~~~~~~~~~~~~~~~
    :$ git clone https://github.com/usnistgov/HTGS.git
    :$ cd <HTGS_Directory>
    :<HTGS_Directory>$ mkdir build && cd build
    :<HTGS_Directory>/build$ ccmake ../         (or cmake-gui)

    'Configure' and setup cmake parameters
    'Configure' and 'Build'

    :<HTGS_Directory>/build$ make
    :<HTGS_Directory>/build$ make install
~~~~~~~~~~~~~~~~~~~~~
