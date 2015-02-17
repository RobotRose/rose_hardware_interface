/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*   Author: Okke Hendriks
*   Date  : 2013/12/10
*       - File created.
*
* Description:
*     Hardware controller, base class for for example the lift_controller and the drive
*   controller.
*   Code needs to be in header due to template!
* 
***********************************************************************************/

#ifndef HARDWARE_CONTROLLER_NODE_HPP
#define HARDWARE_CONTROLLER_NODE_HPP

#include <iostream>
#include <stdio.h>

#include <ros/ros.h>

#include "thread_safe_stl_containers/thread_safe_queue.h"

#include "rose_hardware_controller/controller_data.hpp"
#include "rose_hardware_controller/controller_command.hpp"
#include "rose_hardware_controller/controller_response.hpp"
#include "rose_hardware_controller/hardware_timer.hpp"
#include "hardware_comm/hardware_comm.hpp"
#include "serial/serial.hpp"

namespace rose21_platform
{

#define ROS_NAME_HC                         (ROS_NAME + "|HC")

// Commands
#define HARDWARE_CONTROL_ID                         "100"   
#define HARDWARE_CONTROL_VERSION                    "101"
#define HARDWARE_CONTROL_UNKOWN_COMMAND             "102"   
#define HARDWARE_CONTROL_WATCHDOG                   "111"
#define HARDWARE_CONTROL_SET_WATCHDOG_TRESHOLD      "112"
#define HARDWARE_CONTROL_GET_WATCHDOG_TRESHOLD      "113"
#define HARDWARE_CONTROL_GET_NR_OF_TIMERS           "114"
#define HARDWARE_CONTROL_GET_TIMERS                 "115"

// Timeouts
#define HARDWARE_CONTROL_TIMEOUT                    1       // [s]
#define HARDWARE_CONTROL_RESET_COMM_TIMEOUT         2       // [s]

#define HARDWARE_CONTROL_DEBUG                      true    // Turn debug messages of hardware controller on and off

// Default parameters
#define HARDWARE_CONTROL_DEFAULT_WATCHDOG_TIMEOUT   1000    // [ms]
#define HARDWARE_CONTROL_WATCHDOG_RATE              10      // [hz]

using namespace std;
using namespace rose21_platform;

/**
 * The HardwareController class is a templated class, it gets templated with an interface type which defines 
 * the communication protocol. 
 */
template <class InterfaceType> class HardwareController
{
  public:
    HardwareController()
        : enabled_(false)
        , responses_read_thread_spawned_(false)
        , watchdog_thread_spawned_(false)
        , stop_watchdog_(false)
        , stop_read_loop_(false) 
        , received_controller_id_(-1)
        , received_firmware_major_version_(-1)
        , received_firmware_minor_version_(-1)
        , n_p_(ros::NodeHandle("~"))
    {
        set_name("NONAME");
        executing_command_mutex_ = boost::shared_ptr<mutex>(new mutex());   
    }

    HardwareController(string name, InterfaceType communication_interface)
        : enabled_(false)
        , responses_read_thread_spawned_(false)
        , watchdog_thread_spawned_(false)
        , stop_watchdog_(false)
        , stop_read_loop_(false)
        , received_controller_id_(-1)
        , received_firmware_major_version_(-1)
        , received_firmware_minor_version_(-1)
        , n_p_(ros::NodeHandle("~"))
    {
        set_name("NONAME");
        set_communication_interface(communication_interface);
        executing_command_mutex_ = boost::shared_ptr<mutex>(new mutex());
    }

    ~HardwareController()
    {
        stopReadloop();
    }

    bool set_name(string name) 
    {
      name_ = name;
      return true;
    }

    string get_name()
    {
      return name_;
    }

    bool set_comm_interface(InterfaceType comm_interface) 
    {
      comm_interface_ = comm_interface;
      return true;
    }

    InterfaceType* get_comm_interface()
    {
      return &comm_interface_;
    }

    bool checkControllerID(int expected_controller_id)
    {
        ControllerResponse*     response;
        ControllerCommand*      command;
        
        // Get firmware id
        response = new ControllerResponse(HARDWARE_CONTROL_ID, HARDWARE_CONTROL_TIMEOUT);
        response->addExpectedDataItem(ControllerData(expected_controller_id, received_controller_id_));
        command  = new ControllerCommand(HARDWARE_CONTROL_ID, *response);

        if(!executeCommand(*command))
        {
            ROS_ERROR_NAMED(ROS_NAME, "Invalid firmware ID detected: %d, expected: %d", received_controller_id_, expected_controller_id);
            return false;
        }

        return true;
    }

    bool checkFirmwareVersion(int expected_firmware_major_version, int expected_firmware_minor_version)
    {
        ControllerResponse*     response;
        ControllerCommand*      command;

        // Get firmware version
        response = new ControllerResponse(HARDWARE_CONTROL_VERSION, HARDWARE_CONTROL_TIMEOUT);
        response->addExpectedDataItem(ControllerData(expected_firmware_major_version, received_firmware_major_version_));
        response->addExpectedDataItem(ControllerData(expected_firmware_minor_version, received_firmware_minor_version_));
        command  = new ControllerCommand(HARDWARE_CONTROL_VERSION, *response);

        if(!executeCommand(*command))
        {
            ROS_ERROR_NAMED(ROS_NAME, "Invalid firmware version detected: %d.%d, expected: %d.%d",  received_firmware_major_version_, 
                                                                                                    received_firmware_minor_version_, 
                                                                                                    expected_firmware_major_version, 
                                                                                                    expected_firmware_minor_version);
            return false;
        }
        
        return true;
    }

    bool simpleCommand(string command_string, int timeout)
    {
        ControllerResponse response(command_string, timeout);
        ControllerCommand  command(command_string, response);

        return executeCommand(command);
    }

    bool simpleCommand(string command_string, int timeout, int data)
    {
        ControllerResponse response(command_string, timeout);
        ControllerCommand  command(command_string, response);
        command.addDataItem(data);

        return executeCommand(command);
    }

    bool setValue(string command_string, int timeout, int send_value)
    {
        ControllerResponse response(command_string, timeout);
        response.addExpectedDataItem(ControllerData(send_value, "Setting value unsuccessfull."));
        ControllerCommand  command(command_string, response);
        command.addDataItem(send_value);

        return executeCommand(command);
    }

    bool setValue(string command_string, int timeout, int send_value, int& receive_value)
    {
        ControllerResponse response(command_string, timeout);
        response.addExpectedDataItem(ControllerData(send_value, receive_value, "Setting value unsuccessfull."));
        ControllerCommand  command(command_string, response);
        command.addDataItem(send_value);

        return executeCommand(command);
    }

    bool getValue(string command_string, int timeout, int& receive_value)
    {
        ControllerResponse response(command_string, timeout);
        response.addExpectedDataItem(ControllerData(receive_value));
        ControllerCommand  command(command_string, response);

        return executeCommand(command);
    }

    bool getValue(string command_string, int timeout, const int& send_value, int& receive_value)
    {
        ControllerResponse response(command_string, timeout);
        response.addExpectedDataItem(ControllerData(receive_value));
        ControllerCommand  command(command_string, response);
        command.addDataItem(send_value);

        return executeCommand(command);
    }

    //! @todo OH: Update this function and ControllerCommand etc. to use (const) references etc.
    bool executeCommand(ControllerCommand command)
    {
        // Blocking lock such that only one thread can execute a command at a time
        // This is neccessary because the watchdog is running in a seperate thread
        executing_command_mutex_->lock();

        ROS_DEBUG_NAMED(ROS_NAME_HC, "Executing command [%s]", command.getSerialMessage().c_str());
        
        // Check if the communication interface is connected
        if(!get_comm_interface()->connect())
        {
            ROS_WARN_NAMED(ROS_NAME_HC, "Serial not connected, when trying to write command [%s]", command.getSerialMessage().c_str());
            executing_command_mutex_->unlock();
            return false;
        }
        ROS_DEBUG_NAMED(ROS_NAME_HC,  "Writing: %s", command.getSerialMessage().c_str());

        // Write to the platform
        if(!get_comm_interface()->writeBlock(command.getSerialMessage().c_str(), command.getSerialMessage().length()))
        {
            ROS_DEBUG_NAMED(ROS_NAME_HC,  "Write of command [%s] failed", command.getSerialMessage().c_str());
            executing_command_mutex_->unlock();
            return false;
        }

        // Wait for command response
        if(!waitForResponse(command))
        {
            executing_command_mutex_->unlock();
            return false;     
        }

        executing_command_mutex_->unlock();

        return true;
    }

    // You can do custom stuff in this function
    virtual bool handleResponse(ControllerResponse response)
    {
        return true;
    }

    bool checkResponse(ControllerCommand command, ControllerResponse response)
    {
        //  Check for unkown command response
        if(command.getCommand() == HARDWARE_CONTROL_UNKOWN_COMMAND)
        {
            ROS_WARN_NAMED(ROS_NAME, "Unkown command '%s' received at low-level controller.", command.getCommand().c_str());
            return false;
        }

        // Check if the received response is of the correct type (number)
        if(command.getExpectedResponse().get_type() == response.get_type()) 
        {       
            // A command could be a order or a status request, check the returned data items accordingly
            // An order will have to return the given parameters in the same sequence
            // A status request will have to set the values of the variables it is associated with.
            // Therefore a data item will have to have a pointer to this variable when doing a status request
            list<ControllerData> expected_response_data  = command.getExpectedResponse().getExpectedDataItems();
            list<ControllerData> response_data           = response.getReceivedDataItems();            
   
            // Check if the number of received and expected data items is the same
            if(expected_response_data.size() != response_data.size())
            {
                ROS_WARN_NAMED(ROS_NAME_HC,  "Received incorrect number of dataitems (%lu received, expected %lu) for command %s.", response_data.size(), expected_response_data.size(), command.getCommand().c_str());
                return false;
            }

            // Loop through the data items
            bool all_data_ok            = true;
            auto it_expected_response   = expected_response_data.begin();
            for(auto it_response = response_data.begin(); it_response != response_data.end(); it_response++)
            { 
                // If the pointer is not NULL assign the data                
                if(it_expected_response->getDataPointer() != NULL)
                {
                    if(!assignPointedValue(it_expected_response->getDataPointer(), it_response->getData()))        
                        return false;
                }

                // Check if we are expecting a specific response
                if(it_expected_response->getData() != "")
                {
                    // Check if we received this specific data
                    if(it_expected_response->getData() != it_response->getData())
                    {
                        ROS_WARN_NAMED(ROS_NAME_HC,  "Wrong data value echoed back whilst issuing an order(%s), expected: %s, received: %s", command.getCommand().c_str(), it_expected_response->getData().c_str(), it_response->getData().c_str());
                        // Display custum error message
                        if(it_expected_response->getErrorMessage() != "")
                            ROS_WARN_NAMED(ROS_NAME_HC,  "%s", it_expected_response->getErrorMessage().c_str());

                        // Do not immediatly return in order to read the other expected values if there are any
                        all_data_ok = false;
                    }
                }                
                
                // Increase iterator pointer unit
                it_expected_response++;
            } 


            return all_data_ok;

        }
        else
        {
            ROS_WARN_NAMED(ROS_NAME_HC,  "Not the correct response [%s], expected [%s].", response.getPrettyString().c_str(), command.getExpectedResponse().getPrettyString().c_str());
        }

        return false;
    }

    bool assignPointedValue(int* pointer, string value)
    {
        // Assign received value, for now always an integer
        try{
            *pointer = rose_conversions::stringToInt(value);
            ROS_DEBUG_NAMED(ROS_NAME_HC,  "Assigned integer value '%d' from a controller response: '%s'", *pointer, value.c_str());
            return true;
        }
        catch (...) {
          ROS_ERROR_NAMED(ROS_NAME_HC,  "Trying to assign integer value from a controller response but the recevied data is not a number: '%s'", value.c_str());
        }   
        return true;
    }

    bool waitForResponse(ControllerCommand command)
    {
        if(!responses_read_thread_spawned_)
        {
            ROS_ERROR_NAMED(ROS_NAME,  "Waiting for repsonse without response read loop enabled.");
            return false;
        }

        bool correct_response_received  = false;
        ros::Time begin                 = ros::Time::now();
        int sec_waiting                 = 0;
        
        do
        {
            ROS_DEBUG_THROTTLE_NAMED(0.1, ROS_NAME_HC, "Waiting for response (%s)... (%ds of %ds)", command.getExpectedResponse().getPrettyString().c_str(), sec_waiting, command.getExpectedResponse().get_timeout());    

            if(!responses_.empty())
            {                
                ControllerResponse front_response = responses_.front();

                ROS_DEBUG_NAMED(ROS_NAME_HC,  "Response received: %s", front_response.getPrettyString().c_str()); 

                if(checkResponse(command, front_response))
                {
                    ROS_DEBUG_NAMED(ROS_NAME_HC,  "Correct response received: %s", front_response.getPrettyString().c_str()); 
                    // Handle the response
                    // Break out of the while loop!
                    responses_.pop();    
                    return true;             
                }  
                else
                {
                    responses_.pop();     
                    return false;
                }
                 
            }
            else
               ros::Duration(0.002).sleep();

            // Check if timeout has expired
            sec_waiting = ros::Time::now().toSec() - begin.toSec();
            if(sec_waiting >= command.getExpectedResponse().get_timeout())
            {
                ROS_ERROR_NAMED(ROS_NAME_HC,  "TIMEOUT(%ds) while waiting for response %s", command.getExpectedResponse().get_timeout(), command.getExpectedResponse().getPrettyString().c_str());
                return false;
            }      
            
        }while(sec_waiting < command.getExpectedResponse().get_timeout());

        return false;
    }

    
    bool spawnReadloop()
    {
        if(responses_read_thread_spawned_ == true)
        {
            ROS_DEBUG_NAMED(ROS_NAME_HC,  "responsesReadloop already spawned");
            return true;
        }
        else
        {
            stop_read_loop_mutex_   = boost::shared_ptr<mutex>(new mutex());

            ROS_DEBUG_NAMED(ROS_NAME_HC,  "Spawning responsesReadloop");
            responses_read_thread_          = boost::shared_ptr<thread>(new thread(&HardwareController::responsesReadloop, this));

            responses_read_thread_spawned_  = true;
        }
    }

    void stopReadloop()
    {
        if(stop_read_loop_mutex_ == NULL || responses_read_thread_spawned_ == false)
            return;

        ROS_DEBUG_NAMED(ROS_NAME_HC,  "Stopping responsesReadloop");

        stop_read_loop_mutex_->lock();
        stop_read_loop_         = true;
        stop_read_loop_mutex_->unlock();

        responses_read_thread_->join();
        responses_read_thread_spawned_ = false;

        stop_read_loop_mutex_->lock();
        stop_read_loop_         = false;
        stop_read_loop_mutex_->unlock();
    }

    // Gets the buffer from the serial interface, and take apart into $ seperated messages
    void responsesReadloop()
    {
        deque<char>                 serial_buffer;
        ControllerResponse*         cur_response        = new ControllerResponse();
        thread_safe::deque<char>    latest_serial_data  = *new thread_safe::deque<char>();

        ros::Time start_time_;
        ros::Time end_time_;

        bool local_stop_read_loop_ = false;
        
        ROS_DEBUG_NAMED(ROS_NAME_HC,  "Started responsesReadloop");
        
        while(!local_stop_read_loop_)
        {
            start_time_ = ros::Time::now();

            stop_read_loop_mutex_->lock();
            local_stop_read_loop_ = stop_read_loop_;
            stop_read_loop_mutex_->unlock();

            if(get_comm_interface()->is_ok())
            {
                if(serial_buffer.empty())
                {
                    // Get latest data and append to the local serial_buffers                  
                    if(get_comm_interface()->fetchBuffer(&latest_serial_data))
                    {
                        serial_buffer.insert(serial_buffer.end(), latest_serial_data.begin(), latest_serial_data.end());                    
                        latest_serial_data.clear();
                    }
                    else
                        ros::Duration(0.002).sleep();                            
                }
                else
                {    
                    // Get the first character in the buffer
                    char cur_character = serial_buffer.front();
                    serial_buffer.pop_front();
                    switch(cur_character)            
                    {                
                        case '$':
                               ROS_DEBUG_NAMED(ROS_NAME_HC,  "$ received -> starting new response");
                            cur_response = new ControllerResponse();
                            break;
                        case '\n':
                        case '\r':
                               ROS_DEBUG_NAMED(ROS_NAME_HC,  "NEWLINE received -> Response received: %s", cur_response->getPrettyString().c_str());

                            responses_.push(*cur_response);

                            cur_response = new ControllerResponse();
                            break;
                        default:
                               ROS_DEBUG_NAMED(ROS_NAME_HC,  "Adding character '%c' to response", cur_character);
                            cur_response->addCharacter(cur_character);
                            break;
                    }; 
                }   
            }

            end_time_ = ros::Time::now();
            ros::Duration d = end_time_ - start_time_; 
         //   ROS_INFO("responsesReadloop time: %.5f rate: %.2f", d.toSec(), 1.0/d.toSec());    
        }   

        // Cleanup
        do
        {
            if(!responses_.empty())
                responses_.pop();
        } while(!responses_.empty());
    }

    bool setWatchdogTreshold(int treshold)
    {
        return setValue(HARDWARE_CONTROL_SET_WATCHDOG_TRESHOLD, HARDWARE_CONTROL_TIMEOUT, treshold, watchdog_treshold_);
    }

    bool getWatchdogTreshold()
    {
        return getValue(HARDWARE_CONTROL_GET_WATCHDOG_TRESHOLD, HARDWARE_CONTROL_TIMEOUT, watchdog_treshold_);
    }

    bool updateTimers()
    {
        int nr_timers = 0;

        if( not getValue(HARDWARE_CONTROL_GET_NR_OF_TIMERS, HARDWARE_CONTROL_TIMEOUT, nr_timers))
        {
            ROS_ERROR_NAMED(ROS_NAME, "Could not retreive number of hardware timers.");
            return false;
        }

        if(nr_timers < 0 or nr_timers > 1000)
        {
            ROS_ERROR_NAMED(ROS_NAME, "Received nr of timer is unreasonable (%d).", nr_timers);
            return false;
        }

        timers.resize(nr_timers);


        // Get firmware version
        ControllerResponse response = ControllerResponse(HARDWARE_CONTROL_GET_TIMERS, HARDWARE_CONTROL_TIMEOUT);
        int i = 0;
        for(auto& timer : timers)
        {
            response.addExpectedDataItem(ControllerData(timers[i].set_value));
            response.addExpectedDataItem(ControllerData(timers[i++].cur_value));
        }

        ControllerCommand command  = ControllerCommand(HARDWARE_CONTROL_GET_TIMERS, response);

        if( not executeCommand(command))
            return false;

        return true;
    }

    string getTimersString()
    {
        string s = "";
        s += "---\n|";
        // for(auto& timer : timers)
        // {
        //     s += rose_conversions::intToString(i) + "\t|\t";
        //     i++;
        // }
        // s += "\n";
        for(auto& timer : timers)
        {
            s += rose_conversions::intToString(timer.set_value) + "\t|\t";
        }
        s += "\n|";
        for(auto& timer : timers)
        {
            s += rose_conversions::intToString(timer.cur_value) + "\t|\t";
        }
        return s;
    }

    bool spawnWatchdog()
    {
        if(watchdog_thread_spawned_ == true)
        {
            ROS_DEBUG_NAMED(ROS_NAME_HC,  "watchdog already spawned");
            return true;
        }
        else if(get_comm_interface()->isConnected() && setWatchdogTreshold(HARDWARE_CONTROL_DEFAULT_WATCHDOG_TIMEOUT))
        {
            watchdog_ok_mutex_     = boost::shared_ptr<mutex>(new mutex());
            stop_watchdog_mutex_   = boost::shared_ptr<mutex>(new mutex());

            watchdog_ok_mutex_->lock();
            watchdog_ok_ = true;
            watchdog_ok_mutex_->unlock();
            
            watchdog_thread_          = boost::shared_ptr<thread>(new thread(&HardwareController::watchdog, this));
            watchdog_thread_spawned_  = true;
            
            return true;
        }

        return false;
    }

    void stopWatchdog()
    {
        if(stop_watchdog_mutex_ == NULL || watchdog_thread_spawned_ == false)
            return;

        ROS_DEBUG_NAMED(ROS_NAME_HC,  "Stopping watchdog");

        stop_watchdog_mutex_->lock();
        stop_watchdog_         = true;
        stop_watchdog_mutex_->unlock();

        watchdog_thread_->join();
        watchdog_thread_spawned_ = false;

        stop_watchdog_mutex_->lock();
        stop_watchdog_          = false;
        stop_watchdog_mutex_->unlock();

        ROS_DEBUG_NAMED(ROS_NAME_HC,  "Watchdog stopped");
    }

    bool checkWatchdog()
    {
        if(watchdog_ok_mutex_ == NULL || watchdog_thread_spawned_ == false)
        {
            ROS_WARN_NAMED(ROS_NAME_HC,  "Checking watchdog while not started.");
            return true;
        }

        bool local_watchdog_ok = false;
        watchdog_ok_mutex_->lock();
        local_watchdog_ok = watchdog_ok_;
        watchdog_ok_mutex_->unlock();
        return local_watchdog_ok;
    }

    void watchdog()
    {
        bool local_stop_watchdog_   = false;
        ros::Rate r(HARDWARE_CONTROL_WATCHDOG_RATE);          // Watchdog check rate

        // Initialize watchdog
        watchdog_           = 0;
        expected_watchdog_  = 0;
        received_watchdog_  = -1;
        int received_watchdog_cnt_ = 0;

        watchdog_ok_mutex_->lock();
        watchdog_ok_ = true;
        watchdog_ok_mutex_->unlock();

        ROS_DEBUG_NAMED(ROS_NAME_HC, "Watchdog started");

        while(!local_stop_watchdog_)
        {
            stop_watchdog_mutex_->lock();
            local_stop_watchdog_ = stop_watchdog_;
            stop_watchdog_mutex_->unlock();

            if(get_comm_interface()->is_ok() && watchdog_ok_)
            {   
                ControllerResponse*     response;
                ControllerCommand*      command;
      
                // Drive motor
                response = new ControllerResponse(HARDWARE_CONTROL_WATCHDOG, HARDWARE_CONTROL_TIMEOUT);
                response->addExpectedDataItem(ControllerData(received_watchdog_));
                response->addExpectedDataItem(ControllerData(received_watchdog_cnt_));
                response->addExpectedDataItem(ControllerData());
                response->addExpectedDataItem(ControllerData());
                command  = new ControllerCommand(HARDWARE_CONTROL_WATCHDOG, *response);
                command->addDataItem(watchdog_);

                if(!executeCommand(*command) || received_watchdog_ != expected_watchdog_)
                {
                    if(received_watchdog_ != expected_watchdog_)
                        ROS_ERROR_NAMED(ROS_NAME_HC,  "Watchdog error(%d), wrong response received (received: %d, expected: %d).", received_watchdog_cnt_, received_watchdog_, expected_watchdog_);
                    else
                        ROS_ERROR_NAMED(ROS_NAME_HC,  "Watchdog error(%d), could not communicate with platform.", received_watchdog_cnt_);
                    
                    watchdog_ok_mutex_->lock(); 
                    watchdog_ok_ = false;
                    watchdog_ok_mutex_->unlock();
                }

                ROS_DEBUG_NAMED(ROS_NAME_HC,  "Lowlevel watchdog count: %d", received_watchdog_cnt_);
                
                if(watchdog_ == 1)
                    watchdog_ = 0;
                else
                    watchdog_ = 1; 

                if(expected_watchdog_ == 1)
                    expected_watchdog_ = 0;
                else
                    expected_watchdog_ = 1;      
            }

            // Watchdog interval
            r.sleep();
        }
    }


  protected:
    string                                  name_;
    InterfaceType                           comm_interface_;
    ros::NodeHandle                         n_;
    ros::NodeHandle                         n_p_;

    boost::shared_ptr<mutex>                executing_command_mutex_;

    bool                                    responses_empty_;
    boost::shared_ptr<thread>               responses_read_thread_;
    thread_safe::queue<ControllerResponse>  responses_; 
    bool                                    responses_read_thread_spawned_;
    bool                                    stop_read_loop_;
    boost::shared_ptr<mutex>                stop_read_loop_mutex_;

    boost::shared_ptr<thread>               watchdog_thread_;
    bool                                    watchdog_thread_spawned_;
    bool                                    stop_watchdog_;
    boost::shared_ptr<mutex>                stop_watchdog_mutex_;
    boost::shared_ptr<mutex>                watchdog_ok_mutex_;

    int     watchdog_;
    int     expected_watchdog_;
    int     received_watchdog_;
    bool    watchdog_ok_;
    int     watchdog_treshold_;

    int     received_controller_id_;
    int     received_firmware_major_version_;
    int     received_firmware_minor_version_;

    int    enabled_;

    std::vector<HardwareTimer> timers;
};

}

#endif // HARDWARE_CONTROLLER_NODE_HPP
