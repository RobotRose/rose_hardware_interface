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

#include "rose_hardware_comm/hardware_comm.hpp"

using namespace std;

HardwareComm::HardwareComm()
	: type_("")
	, connected_(false)
{}

HardwareComm::~HardwareComm()
{}

void HardwareComm::set_type(string type)
{
	type_ = type;
}

string HardwareComm::get_type()
{
	return type_;
}

// Dummy
bool HardwareComm::connect()
{
	return false;
}

// Dummy
bool HardwareComm::disconnect()
{
	return false;
}

bool HardwareComm::isConnected()
{
	return connected_;
}

bool HardwareComm::set_connected(bool connection_status)
{
	connected_ 	= connection_status;
	return true;
}
