/***********************************************************************************
* Copyright: Rose B.V. (2013)
*
* Revision History:
*	Author: Okke Hendriks
*	Date  : 2013/12/10
* 		- File created.
*
* Description:
*	A simple serial implementation of the hardware_communication based on a lib from
* 	stanford university. Scroll down fot their disclaimer.
* 
***********************************************************************************/

#include "rose_hardware_comm/serial.hpp"

using namespace std;

Serial::Serial()
{}

Serial::Serial(string parent_name, string port, uint baudrate) 
	: HardwareComm()
	, port_(port)
	, baudrate_(baudrate)
	, read_thread_spawned_(false)
	, happy_(false)
{
	ROS_DEBUG_NAMED(ROS_NAME_SERIAL, "Serial communication object constructed");
	set_type(parent_name + "_serial_controller");	
}

Serial::~Serial()
{
	disconnect();
	ROS_DEBUG_NAMED(ROS_NAME_SERIAL, "Serial communication object destructed");	
}

bool Serial::is_ok() 
{ 
	return happy_; 
}

bool Serial::connect()
{
	// Return true immediatly if already opened and happy
	if(isConnected())
		return true;

	// Check if our state was connected but we became unhappy
	// Then close the port and try to reopen
	if(!happy_)
	{
		ROS_DEBUG_NAMED(ROS_NAME_SERIAL, "Serial connection [%s:%d] is not Happy, disconnecting in order to force reconnect.", port_.c_str(), baudrate_);
		disconnect();
	}

	// Start opening the port
	file_descriptor_ = open(port_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (file_descriptor_ < 0)
	{
		ROS_ERROR_NAMED(ROS_NAME_SERIAL, "Could not open serial connection [%s:%d]: %s. Is the interface connected?", port_.c_str(), baudrate_, strerror(errno));
		return false;
	}
	else
		ROS_INFO_NAMED(ROS_NAME_SERIAL, "Opened serial connection [%s:%d].", port_.c_str(), baudrate_);
  
    // Put the port_ in nonblocking mode
	struct termios oldtio, newtio;
	if (tcgetattr(file_descriptor_, &oldtio) < 0)
	{
		ROS_ERROR_NAMED(ROS_NAME_SERIAL, "Could not get attributes of serial connection [%s:%d].", port_.c_str(), baudrate_);
		return false;
	}
	bzero(&newtio, sizeof(newtio));
	newtio.c_iflag = IGNPAR | INPCK;
	newtio.c_oflag = 0;
	newtio.c_cflag = CS8 | CLOCAL | CREAD;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0; // poll
	cfsetspeed(&newtio, baudrate_);
	tcflush(file_descriptor_, TCIOFLUSH);
	if (tcsetattr(file_descriptor_, TCSANOW, &newtio) < 0)
	{
		ROS_ERROR_NAMED(ROS_NAME_SERIAL, "Setting attributes of serial connection [%s:%d] failed.", port_.c_str(), baudrate_);
		return false;
	}

	// Flush the buffer of the serial device
	char byte;
	while (this->read(&byte) > 0) { }

	happy_ = true;
  	set_connected(true);
  	ROS_DEBUG_NAMED(ROS_NAME_SERIAL,"Attibutes of serial connection [%s:%d] set.", port_.c_str(), baudrate_);

  	spawnReadloop();

	return true;
}

bool Serial::disconnect()
{
	if(!this->isConnected())
		return true;
	
	this->set_connected(false);
	
	// Stop the readthread
	stopReadloop();

	if(file_descriptor_ > 0)
	{
    	close(file_descriptor_);
    	ROS_INFO_NAMED(ROS_NAME_SERIAL, "Closed serial connection [%s:%d]", port_.c_str(), baudrate_);
	}
  	file_descriptor_ = 0; 

	return true;	
}


bool Serial::read(char *byte)
{
  	if(!isConnected())
		return false;

	long nread;
	nread = ::read(file_descriptor_, byte, 1);
	if (nread < 0)
	{
		ROS_WARN_NAMED(ROS_NAME_SERIAL, "Read of serial connection [%s:%d] failed.", port_.c_str(), baudrate_);
		happy_ = false;
		disconnect();
		return false;
	}
	return (nread == 1);
}

int Serial::readBlock(char* block, uint32_t max_read_len)
{
	if(!isConnected())
    	return false;

  	long nread = ::read(file_descriptor_, block, (size_t)max_read_len);
  	if (nread < 0)
	{
		ROS_WARN_NAMED(ROS_NAME_SERIAL, "Block read serial connection [%s:%d] failed.", port_.c_str(), baudrate_);
		happy_ = false;
		disconnect();
		return false;
	}

  	return (nread < 0 ? 0 : nread);
}

bool Serial::write(const char byte)
{
	if(!isConnected())
		return false;

	if (file_descriptor_ >= 0 && ::write(file_descriptor_, &byte, 1) < 0)
	{
		ROS_WARN_NAMED(ROS_NAME_SERIAL, "Byte write on serial connection [%s:%d] failed.", port_.c_str(), baudrate_);
		happy_ = false;
		disconnect();
		return false;
	}

	return true;
}

bool Serial::writeBlock(const char *block, uint32_t block_len)
{
	
	if(!isConnected())
		return false;

	if (file_descriptor_ >= 0 && ::write(file_descriptor_, block, block_len) < 0)
	{
		ROS_WARN_NAMED(ROS_NAME_SERIAL, "Block write on serial connection [%s:%d] failed.", port_.c_str(), baudrate_);
		happy_ = false;
		disconnect();
		return false;
  	}
  	tcflush(file_descriptor_, TCOFLUSH);
	return true;
}


bool Serial::writeBlockSlow(const char *block, uint32_t block_len)
{
	if(!isConnected())
		return false;

	uint32_t j = 0;
	for(j = 0;j < block_len; j++)
	{
		//usleep(SERIAL_SLOW_BLOCK_WRITE_DELAY);
		if(this->write(block[j]) == false) 
		{
			ROS_WARN_NAMED(ROS_NAME_SERIAL, "Slow block write on serial connection [%s:%d] failed.\n", port_.c_str(), baudrate_);
			happy_ = false;
			disconnect();
			return false;
		}
		tcdrain(file_descriptor_); 
	}

	return true;
}

bool Serial::fetchBuffer(thread_safe::deque<char>* buffer)
{
	if(!read_buffer_.empty())
	{
		// Copy, return the copy and clear the thread_safe::read_buffer
		buffer_mutex_->lock(); 
		*buffer = read_buffer_;
		read_buffer_.clear();
		buffer_mutex_->unlock(); 

		string buffer_string = "";
		for(auto it = buffer->begin(); it != buffer->end(); it++)
			buffer_string += *it;
		
		ROS_DEBUG_NAMED(ROS_NAME_SERIAL, "Fetching serial read buffer %s", buffer_string.c_str());
		return true;
	}	
	return false;
}



bool Serial::spawnReadloop()
{
	if(read_thread_spawned_ == true)
	{
		ROS_DEBUG_NAMED(ROS_NAME_SERIAL, "Serial read loop already spawned.");
		return true;
	}
	else	
	{
		ROS_DEBUG_NAMED(ROS_NAME_SERIAL, "Spawning readLoop");
		buffer_mutex_ 			= boost::shared_ptr<mutex>(new mutex());
		stop_read_loop_mutex_	= boost::shared_ptr<mutex>(new mutex());

		stop_read_loop_mutex_->lock();
		stop_read_loop_ 		= false;
		stop_read_loop_mutex_->unlock();

		read_thread_ 			= boost::shared_ptr<thread>(new thread(&Serial::readLoop, this));
		read_thread_spawned_ 	= true;
	}

	return true;
}

void Serial::stopReadloop()
{
	if(stop_read_loop_mutex_ == NULL)
		return;

	stop_read_loop_mutex_->lock();
	stop_read_loop_ 		= true;
	stop_read_loop_mutex_->unlock();

	read_thread_->join();
	read_thread_spawned_ = false;

	stop_read_loop_mutex_->lock();
	stop_read_loop_ 		= false;
	stop_read_loop_mutex_->unlock();
}

// Call this with a separate thread
void Serial::readLoop()
{
	buffer_mutex_->lock();
	read_buffer_.clear();
	buffer_mutex_->unlock();

	char character;
	char character_buffer[100];
	bool local_stop_read_loop_ = false;
	while(isConnected() && !local_stop_read_loop_)		
	{
		stop_read_loop_mutex_->lock();
		local_stop_read_loop_ = stop_read_loop_;
		stop_read_loop_mutex_->unlock();

		int n_read = readBlock(&character_buffer[0], 100);
		if(n_read >= 0)
		{
			ROS_DEBUG_NAMED(ROS_NAME_SERIAL, "%d char's received", n_read);

			buffer_mutex_->lock();
			for(int i = 0; i < n_read; i++)
			{
				ROS_DEBUG_NAMED(ROS_NAME_SERIAL, " %c", character_buffer[i]);
				read_buffer_.push_back(character_buffer[i]); 
			}
			buffer_mutex_->unlock();
		}
		usleep(100);
	}
	ROS_DEBUG_NAMED(ROS_NAME_SERIAL, "Stopping serial readloop");
}


///////////////////////////////////////////////////////////////////////////////
// The serial_port_ package has been used to create this code 
// provides small, simple static libraries to access serial devices.
//
// Copyright (C) 2008, Morgan Quigley
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright 
//     notice, this list of conditions and the following disclaimer in the 
//     documentation and/or other materials provided with the distribution.
//   * Neither the name of Stanford University nor the names of its 
//     contributors may be used to endorse or promote products derived from 
//     this software without specific prior written permission.
//   
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
