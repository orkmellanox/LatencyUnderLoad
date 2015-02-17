#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstdlib>

#define MAX_QUEUED_MASSAGE 100 // Maximum number of messages that can be queued in buffer. 

/* This class represent simple circle buffer object. */
class connection_buffer_t {
public:
    connection_buffer_t(unsigned int msg_size);
    virtual ~connection_buffer_t();
	uint8_t* reserve_memory();	
	void free_memory();	
	uint8_t* get_buffer_head();	
	void increase_first_msg_offset(unsigned int offset);	
	unsigned int get_first_msg_offset();
	void increase_last_msg_offset(unsigned int offset);	
	unsigned int get_last_msg_offset();
	
protected:
  	uint8_t*		buffer; //The main buffer.
	unsigned int	first_element; // Index of first element in circle buffer.
	unsigned int	last_element; // Index of last element in circle buffer.
	unsigned int 	buffer_size; // Size of main buffer.
	unsigned int 	msg_size; // Size of messages that will be filled in buffer.
	uint8_t*		last_msg_address; // Memory address of last message in buffer
	unsigned int	first_msg_offset; // offset of first message in queue
	unsigned int	last_msg_offset; // offset of last message in queue

};
