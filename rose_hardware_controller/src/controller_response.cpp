/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2013/12/12
* 		- File created.
*
* Description:
*	A class holding the data of a serial response fot the lift_controller
* 
***********************************************************************************/

#include "rose_hardware_controller/controller_response.hpp"

using namespace std;
using namespace rose21_platform;

ControllerResponse::ControllerResponse()
	: response_("")
	, timeout_(0)
{}

ControllerResponse::ControllerResponse(string response)
	: response_(response)
	, timeout_(DEFAULT_TIMEOUT)
{}

ControllerResponse::ControllerResponse(string response, int timeout)
	: response_(response)
	, timeout_(timeout)
{}

ControllerResponse::~ControllerResponse()
{}

void ControllerResponse::addCharacter(char character)
{
	response_ = response_ + character;
}

string ControllerResponse::get_response()
{
	return response_;
}

bool ControllerResponse::set_response(string response)
{
	response_ = response;
	return true;
}

int ControllerResponse::get_timeout()
{
	return timeout_;
}

string ControllerResponse::get_type()
{
	// Get the part of the string after the first comma and before the second
    int comma_pos = response_.find(",");
    
    if(comma_pos != string::npos)
        return response_.substr(0, comma_pos);
    else
        return response_;
}



string ControllerResponse::getRawData()
{
	// Get the part of the string behind the first comma if there is one
    int comma_pos = response_.find(",");
    
    if(comma_pos != string::npos)
        return response_.substr(comma_pos + 1);
    else
        return "";

}

string ControllerResponse::getPrettyReceivedData()
{
	list<ControllerData> data_items = getReceivedDataItems();
	string pretty_data 		= "(" + rose_conversions::intToString(data_items.size()) +"):";	
	
	for(auto it = data_items.begin(); it != data_items.end(); it++)
		pretty_data += it->getPrettyString();

	return pretty_data;
}

string ControllerResponse::getPrettyExpectedData()
{
	list<ControllerData> data_items = getExpectedDataItems();
	string pretty_data 		= "(" + rose_conversions::intToString(data_items.size()) +"):";	
	
	for(auto it = data_items.begin(); it != data_items.end(); it++)
		pretty_data += it->getPrettyString();

	return pretty_data;
}

string ControllerResponse::getPrettyString()
{
	string pretty_string = "Type: " + get_type() + " | Expected Data: " + getPrettyExpectedData() + " | Received Data: " + getPrettyReceivedData();
	return pretty_string;
}

bool ControllerResponse::hasData()
{
	if(getRawData() == "")
		return false;
	else
		return true;
}

bool ControllerResponse::addExpectedDataItem(ControllerData data_item)
{
	expected_data_.push_back(data_item);
	return true;
}

list<ControllerData> ControllerResponse::getExpectedDataItems()
{
	return expected_data_;
}

list<ControllerData> ControllerResponse::getReceivedDataItems()
{
	list<ControllerData> dataItems;					//! @todo OH list<ControllerData>
	if(hasData())
	{
		string raw_data 	= getRawData();
		while(raw_data != "")
		{
			int comma_pos = raw_data.find(",");			
			if(comma_pos == string::npos)
				break;

			dataItems.push_back(ControllerData(raw_data.substr(0, comma_pos)));
			raw_data = raw_data.substr(comma_pos + 1, raw_data.length() - comma_pos - 1);
		}	
	}
	return dataItems;
}

