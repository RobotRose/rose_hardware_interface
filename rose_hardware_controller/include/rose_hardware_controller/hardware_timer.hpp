/***********************************************************************************
* Copyright: Rose B.V. (2014)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2014/10/07
* 		- File created.
*
* Description:
*	Holds the state of a timer of a low level controller
* 
***********************************************************************************/

#ifndef HARDWARE_TIMER_HPP
#define HARDWARE_TIMER_HPP

class HardwareTimer
{
public:
	HardwareTimer(int set_value = -2, int cur_value = -2);
	~HardwareTimer();

	int set_value;
	int cur_value;
};

#endif // HARDWARE_TIMER_HPP
