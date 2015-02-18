cmake_minimum_required(VERSION 2.8.3)
project(rose_hardware_controller)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
		roscpp
		rose_common
		rose_conversions
		rose_hardware_comm
		rose_common
	)


catkin_package(
 INCLUDE_DIRS 
 	include
 LIBRARIES 

 CATKIN_DEPENDS
	roscpp
	rose_common
	rose_conversions
	rose_hardware_comm
	rose_common
 DEPENDS 

)

set(CMAKE_CXX_FLAGS "-std=c++0x -g") #! @todo OH [IMPR]: Remove -g flag in release?

include_directories( include ${catkin_INCLUDE_DIRS} )

add_library(rose_hardware_controller 
								src/controller_data.cpp
								src/controller_command.cpp
								src/controller_response.cpp
								src/hardware_timer.cpp
								src/hardware_controller.cpp)

add_dependencies( rose_hardware_controller ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)

target_link_libraries(rose_hardware_controller ${catkin_LIBRARIES})
