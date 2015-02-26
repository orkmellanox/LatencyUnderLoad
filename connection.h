#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstdlib>
#include <chrono>

#include "cbuffer.h"
#include "squeue.h"

class connection_t {
public:
    connection_t(int fd, int index);
    virtual ~connection_t() {};

	int get_fd();
	int get_index();
	queue_t<double>* get_msgs_rtt();
	connection_buffer_t* get_read_buffer();
	connection_buffer_t* get_write_buffer();
	bool is_valid();
	
	void add_unaknowledged_msg();
	void record_msg_rtt();
	void remove_connection();

	
protected:
	int fd, index;
	connection_buffer_t* write_buffer;
	connection_buffer_t* read_buffer;
	queue_t<std::chrono::high_resolution_clock::time_point>* unaknowledged_msgs;// related to client only
	queue_t<double>* msgs_rtt; // related to client only
	bool valid;
	
		
	//Following are variable that used for temporary storage.
	std::chrono::high_resolution_clock::time_point receive_time;
	double rtt;	
};

inline int connection_t::get_fd(){
	return fd;
}

inline int connection_t::get_index(){
	return index;
}

inline queue_t<double>* connection_t::get_msgs_rtt(){
	return msgs_rtt;
}

inline connection_buffer_t* connection_t::get_read_buffer(){
	return write_buffer;
}

inline connection_buffer_t* connection_t::get_write_buffer(){
	return read_buffer;
}

inline bool connection_t::is_valid(){
	return valid;
}

inline void connection_t::add_unaknowledged_msg() {

	unaknowledged_msgs->push(std::chrono::high_resolution_clock::now());
}

inline void connection_t::record_msg_rtt() {
	 
	receive_time = std::chrono::high_resolution_clock::now(); 
	rtt = std::chrono::duration_cast<std::chrono::nanoseconds>(receive_time - unaknowledged_msgs->front()).count();
	msgs_rtt->push(rtt);
	unaknowledged_msgs->pop();
}
