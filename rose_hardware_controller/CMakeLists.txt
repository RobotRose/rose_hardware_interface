cmake_minimum_required(VERSION 2.8.3)
project(rose21_platform_controller)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
		roscpp
		rose_common
		rose_conversions
		rose_hardware_comm
		rose_hardware_controller
		rose_utils
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
	rose_hardware_controller
	rose_utils
 DEPENDS 

)

include_directories( include ${catkin_INCLUDE_DIRS} )

add_library(rose_hardware_controller 
								src/rose_hardware_controller/controller_data.cpp
								src/rose_hardware_controller/controller_command.cpp
								src/rose_hardware_controller/controller_response.cpp
								src/rose_hardware_controller/hardware_timer.cpp
								src/rose_hardware_controller/hardware_controller.cpp)

add_dependencies( rose_hardware_controller ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)

target_link_libraries(rose_hardware_controller ${catkin_LIBRARIES})
