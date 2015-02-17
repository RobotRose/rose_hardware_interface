/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2013/12/11
* 		- File created.
*
* Description:
*	A class holding the data of a serial response fot the lift_controller
* 
***********************************************************************************/

#ifndef LIFT_CONTROLLER_RESPONSE_HPP
#define LIFT_CONTROLLER_RESPONSE_HPP

#include <iostream>
#include <stdio.h>

#include <list>

#include "rose_hardware_controller/controller_data.hpp"

#define DEFAULT_TIMEOUT			5

using namespace std;
using namespace rose21_platform;

class ControllerResponse
{
  public:
  	ControllerResponse();
	ControllerResponse(string response);
	ControllerResponse(string response, int timeout);
	~ControllerResponse();

	void 					addCharacter(char character);
	string 					get_response();
	bool  					set_response(string response);
	int 					get_timeout();
	string 					get_type();
	string 					getRawData();
	string  				getPrettyReceivedData();
	string  				getPrettyExpectedData();
	string 					getPrettyString();
	bool 					hasData();
	bool 					addExpectedDataItem(ControllerData data_item);
	list<ControllerData> 	getExpectedDataItems();
	list<ControllerData> 	getReceivedDataItems();
  
  private:
	string 					response_;
	int 					timeout_;
	list<ControllerData> 	expected_data_;
};

#endif // LIFT_CONTROLLER_RESPONSE_HPP