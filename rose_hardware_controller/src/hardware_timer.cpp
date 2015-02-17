/***********************************************************************************
* Copyright: Rose B.V. (2014)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2014/10/07
* 		- File created.
*
* Description:
*	Hardware timer, holds state of a hardware timer
* 
***********************************************************************************/

#include "rose_hardware_controller/hardware_timer.hpp"

HardwareTimer::HardwareTimer(int set_value, int cur_value)
	: set_value(set_value)
	, cur_value(cur_value)
{}

HardwareTimer::~HardwareTimer()
{}
