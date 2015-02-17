/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2013/12/09
* 		- File created.
*
* Description:
*	The hardware communication base class
* 
***********************************************************************************/

#ifndef HARDWARE_COMM_HPP
#define HARDWARE_COMM_HPP

#include <iostream>
#include <stdio.h>

#include <ros/ros.h>

#include "thread_safe_stl_containers/thread_safe_deque.h"

using namespace std;

class HardwareComm
{
  public:
	HardwareComm();
	~HardwareComm(); 

	virtual bool 	connect();
	virtual bool 	disconnect();  
	bool 			isConnected();
	string 			get_type();
  	void 			set_type(string type);

	virtual bool   	write(const char byte) = 0;
	virtual	bool   	writeBlock(const char *block, uint32_t write_len) = 0; 
	virtual bool  	writeBlockSlow(const char *block, uint32_t write_len) = 0; 

	virtual	bool   	read(char *byte) = 0;
	virtual	int    	readBlock(char *block, uint32_t max_read_len) = 0;                                                         

	virtual bool 	fetchBuffer(thread_safe::deque<char>* buffer) = 0;

  protected:
  	bool 			set_connected(bool connection_status);

  private:
	string 			type_;
	bool			connected_;
};	

#endif // HARDWARE_COMM_HPP
