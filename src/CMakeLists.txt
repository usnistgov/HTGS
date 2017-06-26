
# NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
# NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
# You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-modules")


install(DIRECTORY htgs
        DESTINATION include
        COMPONENT headers)



if (BUILD_MAIN)

#    add_definitions(-DPROFILE)
#    add_definitions(-DWS_PROFILE)
#    add_definitions(-DUSE_PRIORITY_QUEUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall -Wextra -Wno-unused-parameter")

    file(GLOB_RECURSE INC_ALL "*.hpp" "*.h")
    find_package(Threads REQUIRED)
    find_package(HTGSVisualizer QUIET)

    link_libraries(${CMAKE_THREAD_LIBS_INIT})
    include_directories(".")


    include_directories(${LIBHTGS_VISUALIZER_INCLUDE_DIRS})

    add_definitions(${HTGS_VISUALIZER_DEFINITIONS})

    add_executable(main "main.cpp" ${INC_ALL})

    target_link_libraries(main ${LIBHTGS_VISUALIZER_LIBRARIES})

endif(BUILD_MAIN)
