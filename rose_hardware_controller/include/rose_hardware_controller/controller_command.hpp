/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2013/12/11
* 		- File created.
*
* Description:
*	A class holding the data of a command for the lift controller
* 
***********************************************************************************/

#ifndef LIFT_CONTROLLER_COMMAND_HPP
#define LIFT_CONTROLLER_COMMAND_HPP

#include <iostream>
#include <stdio.h>

#include <list>

#include "rose_hardware_controller/controller_data.hpp"
#include "rose_hardware_controller/controller_response.hpp"

/**
 * ControllerCommand class
 */
class ControllerCommand
{
	public:

		/**
		 * Constructor of the ControllerCommand class.
		 * @param[in] string command, the command string.
		 */
		ControllerCommand(const std::string& command);

		/**
		 * Constructor of the ControllerCommand class.
		 * @param[in] string command, the command string.
		 * @param[in] ControllerResponse expected_response, the resonse expected to be returned after sending this command.
		 */		
		ControllerCommand(const std::string& command, ControllerResponse expected_response);

		/**
		 * Deconstructor of the ControllerCommand class.
		 */	
		~ControllerCommand();

		/**
		 * Gets the string to be send to a controller
		 * @return A string containing the message to be send to the serialized port.
		 */	
		std::string 			getSerialMessage();

		/**
		* Adds a ControllerData instance to the list of data items.
		* @return true, if the dataitem has succesfully been added
		* @return false, if the dataitem has snot been added
		*/	
		bool					addDataItem(ControllerData data_item);

		/**
		*  Creates a ControllerData instance from an integer and adds it to the list of data items.
		* @return true, if the dataitem has succesfully been added
		* @return false, if the dataitem has snot been added
		*/	
		bool					addDataItem(const int& data_item);

		/**
		* Creates a ControllerData instance from string and adds it to the list of data items.
		* @return true, if the dataitem has succesfully been added
		* @return false, if the dataitem has snot been added
		*/	
		bool					addDataItem(const std::string& data_item);

		/**
		 * @return A string representation of the data items contained in this command.
		 */
		std::string 			getSerialDataString();

		/**
		 * @return A pretty string of the controller command for printing purposes.
		 */
		std::string 			getPrettyString();

		/**
		 * @return A pretty string of the dataitems for printing purposes.
		 */
		std::string 			getPrettyData();

		/**
		 * Set the expected response, the response that is expected after this command is send.
		 * This will be automatically tested in the hardwareController
		 */
		void 					setExpectedResponse(ControllerResponse expected_response);

		/**
		 * @return The command string.
		 */
		std::string 			getCommand();

		/**
		 * @return The expected response.
		 */
		ControllerResponse 		getExpectedResponse();

		/**
		 * Gets the list of data items.
		 * @return A list<ControllerData> containing the data items.
		 */	
		list<ControllerData>* 	getDataItems();

	private:

		std::string 			command_;
		ControllerResponse 		expected_response_;
		list<ControllerData>	data_;
};

#endif // LIFT_CONTROLLER_COMMAND_HPP
