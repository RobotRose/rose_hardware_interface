cmake_minimum_required (VERSION 2.4.6)
project(rose_hardware_comm)

find_package(catkin REQUIRED COMPONENTS 
    roscomm 
    roscpp
    rose_common
)

catkin_package(
    INCLUDE_DIRS 
    	include
    LIBRARIES 
    	rose_hardware_comm
    CATKIN_DEPENDS 
        roscomm 
        roscpp
        rose_common
    DEPENDS 
    	roscpp
)

set(CMAKE_CXX_FLAGS "-std=c++0x -g ") ##! todo OH: remove compiler backup flag

include_directories(include ${catkin_INCLUDE_DIRS})

add_library(rose_hardware_comm 
                SHARED 
                src/hardware_comm.cpp 
                src/serial.cpp
            )

add_dependencies( rose_hardware_comm ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)

target_link_libraries(rose_hardware_comm ${catkin_LIBRARIES})
