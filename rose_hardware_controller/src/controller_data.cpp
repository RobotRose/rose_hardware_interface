/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2013/12/20
* 		- File created.
*
* Description:
*	Data item that contains the data to be send and a pointer to the variable to
* 	be set when a response is received.
* 
***********************************************************************************/

#include "rose_hardware_controller/controller_data.hpp"


// TODO OH: Make templated bool int std::string?

ControllerData::ControllerData()
{
	Initialize("", "");
}

ControllerData::ControllerData(int& data_reference)
{
	Initialize("", data_reference, "");
}

ControllerData::ControllerData(std::string data)
{
	Initialize(data, "");
}

ControllerData::ControllerData(const int& data, std::string error_message)
{
	Initialize(rose_conversions::intToString(data), error_message);
}

ControllerData::ControllerData(std::string data, std::string error_message)
{
	Initialize(data, error_message);
}

ControllerData::ControllerData(const int& data, int& data_reference)
{
	Initialize(rose_conversions::intToString(data), data_reference, "");
}

ControllerData::ControllerData(std::string data, int& data_reference)
{
	Initialize(data, data_reference, "");
}

ControllerData::ControllerData(const int& data, int& data_reference, std::string error_message)
{
	Initialize(rose_conversions::intToString(data), data_reference, error_message);
}

ControllerData::ControllerData(std::string data, int& data_reference, std::string error_message)
{
	Initialize(data, data_reference, error_message);
}


void ControllerData::Initialize(std::string data, std::string error_message)
{
	data_ 				= data;
	data_reference_  	= NULL;
	error_message_ 		= error_message;	
}

void ControllerData::Initialize(std::string data, int& data_reference, std::string error_message)
{
	data_ 				= data;
	data_reference_  	= &data_reference;
	error_message_ 		= error_message;	
}



ControllerData::~ControllerData()
{}

std::string ControllerData::getData()
{
	return data_;
}

int* ControllerData::getDataPointer()
{
	return data_reference_;
}

std::string ControllerData::getPrettyString()
{
	return "[" + data_ + "]";
}

std::string ControllerData::getErrorMessage()
{
	return error_message_;
}
