/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2013/12/10
* 		- File created.
*
* Description:
*	A serial over usvb implementation of the hardware_communication
* 
***********************************************************************************/

#ifndef SERIAL_OVER_USB_HPP
#define SERIAL_OVER_USB_HPP

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include <thread> 
#include <mutex> 
#include <deque>

#include <ros/ros.h>

#include "thread_safe_stl_containers/thread_safe_deque.h"
#include "rose_hardware_comm/hardware_comm.hpp"
#include "ros_name/ros_name.hpp"

#define ROS_NAME_SERIAL 					(ROS_NAME + "|SERIAL")

#define SERIAL_SLOW_BLOCK_WRITE_DELAY		400 // [us]

using namespace std;

class Serial : public HardwareComm 
{
	public:
		Serial();
		Serial(string parent_name, string port, uint baudrate);
		~Serial();

		bool   				connect();
		bool   				disconnect();

		bool   				read(char *byte);
		int    				readBlock(char *block, uint32_t max_read_len);

		bool   				write(const char byte);
		bool   				writeBlock(const char *block, uint32_t write_len);
		bool   				writeBlockSlow(const char *block, uint32_t write_len);
	
		bool 				is_ok();

		bool 				fetchBuffer(thread_safe::deque<char>* buffer);

	protected:
		bool 				spawnReadloop();
		void 				stopReadloop();
		void 				readLoop();		

		string 				port_;
		uint 				baudrate_;
		int 				file_descriptor_;
		bool 				happy_;

		boost::shared_ptr<mutex>	buffer_mutex_;
		boost::shared_ptr<thread>	read_thread_;
		thread_safe::deque<char>	read_buffer_;
		bool 						read_thread_spawned_;
		
		bool 		 				stop_read_loop_;
		boost::shared_ptr<mutex>	stop_read_loop_mutex_;

		ros::Time 					start_time_;
		ros::Time 					end_time_;


};

#endif  // SERIAL_OVER_USB_HPP