/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2013/12/12
* 		- File created.
*
* Description:
*	A class holding the data of a command for the lift controller
* 
***********************************************************************************/

#include "rose_hardware_controller/controller_command.hpp"

using namespace std;

ControllerCommand::ControllerCommand(string command)
	: command_(command)
	, expected_response_("")
{}

ControllerCommand::ControllerCommand(string command, ControllerResponse expected_response)
	: command_(command)
	, expected_response_(expected_response)
{}

ControllerCommand::~ControllerCommand()
{}

string ControllerCommand::getSerialMessage()
{
	return ("$" + command_ + "," + getSerialDataString() + "\r");
}
  
bool ControllerCommand::addDataItem(ControllerData data_item)
{
	data_.push_back(data_item);
	return true;
}

bool ControllerCommand::addDataItem(const int& data_item)
{
	data_.push_back(ControllerData(rose_conversions::intToString(data_item)));
	return true;
}

bool ControllerCommand::addDataItem(const std::string& data_item)
{
	data_.push_back(ControllerData(data_item));
	return true;
}

string ControllerCommand::getSerialDataString()
{
	string data_string = "";
	for(auto it = data_.begin(); it != data_.end(); it++)
		data_string += it->getData() + ",";
	return data_string;
}

string ControllerCommand::getPrettyString()
{
	string pretty_string = "Command: " + command_ + " | Data: " + getPrettyData();
	return pretty_string;
}

string ControllerCommand::getPrettyData()
{
	string pretty_data 		= "(" + rose_conversions::intToString(data_.size()) +"):";
	for(auto it = data_.begin(); it != data_.end(); it++)
		pretty_data += it->getPrettyString();

	return pretty_data;
}

void ControllerCommand::setExpectedResponse(ControllerResponse expected_response)
{
	expected_response_ = expected_response;
}


string 	ControllerCommand::getCommand()
{
	return command_;
}

ControllerResponse ControllerCommand::getExpectedResponse()
{
	return expected_response_;
}

list<ControllerData>* ControllerCommand::getDataItems()
{
	return &data_;
}
