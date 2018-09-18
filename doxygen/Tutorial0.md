Tutorial 0 {#tutorial0}
========
[TOC]

In this tutorial we explain how to download and use the tutorials using the command-line, Eclipse IDE, and CLion IDE.

Downloading the Tutorials {#tut0-download}
=======

## Dependencies: ##
1. HTGS (See [Getting Started](@ref install-instructions))
2. cmake v2.7+
3. gcc/g++ v4.8.x (or higher, C++11)
4. pthreads
5. (Optional) [Graphviz](http://www.graphviz.org/)
6. (Optional, except Tutorials 3b and 6) [OpenBLAS](http://www.openblas.net/)
7. (Optional, except for Tutorials 4, 5, 7, and 8) [CUDA](https://developer.nvidia.com/cuda-downloads) (Requires one or more CUDA-Enabled GPUs on system)

## CMake Options: ##
HTGS_INCLUDE_DIR - HTGS include directory, example: /usr/local/include

OpenBLAS - OpenBLAS home directory, example /usr/local
OpenBLAS_LIB - OpenBLAS lib directory
OpenBLAS_INCLUDE_DIR - OpenBLAS include directory

~~~~~~~~~~~~~~~~~~~~~
    :$ git clone https://github.com/usnistgov/HTGS-Tutorials.git
    :$ cd <HTGS_Tutorials_Directory>
    :<HTGS_Tutorials_Directory>$ mkdir build && cd build
    :<HTGS_Tutorials_Directory>/build$ ccmake ../         (or cmake-gui)

    'Configure' and setup cmake parameters
    'Configure' and 'Build'

    :<HTGS_Tutorials_Directory>/build$ make
~~~~~~~~~~~~~~~~~~~~~

Getting Started using the Command-Line {#tut0-cmd}
======
1. Clone HTGS-Tutorials as shown above and navigate to \<HTGS\_TUTORIALS\_DIRECTORY>
2. Create and navigate into **build** directory
3. Run **ccmake ../** or **cmake-gui ../**
4. Configure CMake Options and specify build type: i.e. Release or Debug
5. Run **make**
6. Tutorial executables are now available in their associated directories (tutorial1, tutorial2, etc.)

Getting Started using Eclipse CDT {#tut0-eclipse}
======
1. Download Eclipse CDT from [here](https://eclipse.org/cdt/)
2. Run Eclipse and download cmake4eclipse
  - Help -> Eclipse Marketplace
  - Find **cmake4eclipse**
  - Restart eclipse
3. Import HTGS Tutorials
  - File->Import...
  - Git->Projects from Git
  - Clone URI
  - Paste https://github.com/usnistgov/HTGS-Tutorials.git to URI
  - Next-> Select **master**
  - **Import as general project**
4. Configure as C/C++ project
  - Right click **HTGS-Tutorials** in Project Explorer
  - New->Convert to a C/C++ Project
  - Project type:->Executable
  - Finish
5. Configure CMake builder and parameters
  - Right click **HTGS-Tutorials** in Project Explorer
  - Properties
  - C/C++ Build-> Ensure **Generate Makefiles automically** is checked
  - C/C++ Build Dropdown->CMake->Symbols
    + Add CMake cache entries to define (-D)
      - Variable name: HTGS_INCLUDE_DIR
      - Type: FILEPATH
      - Value: PATH_TO_HTGS
      - Variable name: OpenBLAS
      - Type: FILEPATH
      - Value: PATH_TO_OPENBLAS_PREFIX_DIR
    + C/C++ Build Dropdown->Tool Chain Editor
      - Current builder: **CMake Make Builder**
6. You should be able to build the HTGS-Tutorials

## Getting Started on Windows with Visual Studio

1. Clone HTGS-Tutorials as shown above and store it in a known directory (see step 6 for git bash)
2. Download Microsoft Visual Studio Community from  [here](https://www.visualstudio.com/downloads/)
3. Run Visual Studio and open a new Win32 project
      - Select default options and click *Finish*
4. Add HTGS source files
      - Right-click on Solution in Solution Explorer
      - Properties > C/C++ > General > Additional Include Directories
      - Write <HTGS source directory>, which denotes where the #include statements will path from
5. To compile, go to Build > Build solution to generate the .exe file.
      - The program will be made in \<VISUAL STUDIO LOCATION\>/Projects/\<PROJECT NAME\>/Debug
6. Optional programs for Windows
      - **Windows Powershell** is useful for easily running .exe's and for navigating directories with bash. It sometimes comes installed on Windows by default or can otherwise be downloaded [here](https://msdn.microsoft.com/en-us/powershell/scripting/setup/installing-windows-powershell).
      - A command-line version of **git** can be found [here](https://git-scm.com/downloads).

### Notes:
- Ensure that #include "stdafx.h" is the very first statement in every source file
- In Visual Studio's Solution Explorer you can make "New Filter"s to categorize task, rules, etc. but this is not the same as a new *folder* and should not be pathed as such in #include statements.
- When switching from 'Debug' to 'Release' in Visual Studio, you might get some strange errors. These might be fixed by change the 'Precompiled Headers' configuration: Right-click the Solution in the Solution Explorer; Properties > C/C++ > Precompiled Headers > Precompiled Header; change from 'Use (/Yu)' to 'Not Using Precompiled Headers'.

Getting Started using CLion {#tut0-clion}
=====

