/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2013/12/20
* 		- File created.
*
* Description:
*	Controller data item, contains functions for storing data message and pointer to
* coupled variable. 
* 
***********************************************************************************/

#ifndef CONTROLLER_DATA_HPP
#define CONTROLLER_DATA_HPP

#include <iostream>
#include <stdio.h>

#include "rose_common/common.hpp"
#include "rose_conversions/conversions.hpp"
 
/**
 * The ControllerData represents one data item, a number of overridden constructors setting the datafields accordingly.
 * it contains the data value for use in a ControllerCommand and a pointer to a coupled variable for use in a ControllerResponse.
 */
class ControllerData
{
  public:

    //! Empty constructor of ControllerData
    ControllerData();

    /**
     * ControllerData constructor calling Initialize()
     * @param[in] int& data_reference
     */
    ControllerData(int& data_reference);

    /**
    * ControllerData constructor calling Initialize()
    * @param[in] std::string data_reference
    */
    ControllerData(std::string data);

    /**
    * ControllerData constructor calling Initialize()
    * @param[in] const int& data
    * @param[in] std::string error_message
    */
    ControllerData(const int& data, std::string error_message);

    /**
    * ControllerData constructor calling Initialize()
    * @param[in] std::string data
    * @param[in] std::string error_message
    */
    ControllerData(std::string data, std::string error_message);

    /**
    * ControllerData constructor calling Initialize()
    * @param[in] const int& data
    * @param[in] int& data_reference
    */
    ControllerData(const int& data, int& data_reference);

    /**
    * ControllerData constructor calling Initialize()
    * @param[in] std::string data
    * @param[in] int& data_reference
    */
    ControllerData(std::string data, int& data_reference);

    /**
    * ControllerData constructor calling Initialize()
    * @param[in] const int& data
    * @param[in] int& data_reference
    * @param[in] std::string error_message
    */
    ControllerData(const int& data, int& data_reference, std::string error_message);

    /**
    * ControllerData constructor calling Initialize()
    * @param[in] std::string data
    * @param[in] int& data_reference
    * @param[in] std::string error_message
    */
    ControllerData(std::string data, int& data_reference, std::string error_message);

    /**
    * Deconstructor of the ControllerData class
    */
    ~ControllerData();

     /**
    * Initializes the data item, this is called from all constructors
    * @param[in] std::string data
    * @param[in] std::string error_message
    */
    void     Initialize(std::string data, std::string error_message);

    /**
    * Initializes the data item, this is called from all constructors
    * @param[in] std::string data
    * @param[in] int& data_reference
    * @param[in] std::string error_message
    */
    void     Initialize(std::string data, int& data_reference, std::string error_message);

    /**
    * @return The stringalized data item
    */
  	std::string 	 getData();

    /**
    * @return Pointer to the coupled variable
    */
  	int* 	 getDataPointer();

    /**
    * @return A pretty string for printing purposes
    */
  	std::string getPrettyString();

    /**
    * @return The error message
    */
    std::string   getErrorMessage();

    /**
    * Equallity operator
    */
  	bool operator==(const ControllerData &other) const {
  		return (this->data_ == other.data_ && this->data_reference_ == other.data_reference_ && this->error_message_ == other.error_message_);
  	}

    /**
    * Not-equal operator
    */
  	bool operator!=(const ControllerData &other) const {
  		return (this->data_ != other.data_ && this->data_reference_ != other.data_reference_ && this->error_message_ != other.error_message_);
  	}

  private:
  	std::string      data_;
  	int*             data_reference_;    
  	std::string      error_message_;    
	
};

#endif // CONTROLLER_DATA_HPP 
