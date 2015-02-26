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

/* 
* Reserve memory from circle buffer equal to message size defined in constructor.
* Return pointer to the reserved memory if there enough free memory, NULL otherwise.
*/
inline uint8_t* connection_buffer_t::reserve_memory(){
	uint8_t* reserved_buffer_head;
	
	reserved_buffer_head = NULL;

	if ((last_msg_offset > 0) && (last_msg_offset < msg_size)) {
		reserved_buffer_head = last_msg_address + last_msg_offset;
	}
	else {
		if (first_element == last_element){
			first_element 	= 0;
			last_element 	= 0;
			reserved_buffer_head = buffer;
		}
		else if (first_element < last_element){
			if (msg_size <= (buffer_size - last_element)) {
				reserved_buffer_head = buffer + last_element;
				last_element += msg_size;
			}
			else if (msg_size < first_element) {
				reserved_buffer_head = buffer;
				last_element = msg_size;
			}
		}
		else if ((first_element > last_element) && (msg_size < (first_element - last_element))){
			reserved_buffer_head = buffer + last_element;
			last_element += msg_size;
		}
		
		last_msg_address = reserved_buffer_head;
	}
	
	return reserved_buffer_head;
}

/* 
* Free memory from header of circle buffer with size equal to message size defined in constructor.
*/	
inline void connection_buffer_t::free_memory(){

	if (first_element == last_element){
		first_element 	= 0;
		last_element 	= 0;
	}
	else if (first_element < last_element){
		first_element += msg_size;
	}
	else if (first_element > last_element){
		first_element += msg_size;
		if ((buffer_size - first_element) < msg_size){
			first_element = 0;
		}
	}
}	

/* 
* Get pointer to the header of circle buffer.
*/	
inline uint8_t* connection_buffer_t::get_buffer_head(){

	return buffer + first_element + first_msg_offset;
}

/* 
* Increase first message offset with offset amount and back to zero in reach default msg_size.
*/	
inline void connection_buffer_t::increase_first_msg_offset(unsigned int offset){
	
	first_msg_offset += offset;
	if (first_msg_offset == msg_size)
		first_msg_offset = 0;
}

/* 
* Return offset of the first message in buffer.
*/	
inline unsigned int connection_buffer_t::get_first_msg_offset(){
	
	return first_msg_offset;
}
	
/* 
* Increase last message offset with offset amount and back to zero in reach default msg_size.
*/	
inline void connection_buffer_t::increase_last_msg_offset(unsigned int offset){
	
	last_msg_offset += offset;
	if (last_msg_offset == msg_size)
		last_msg_offset = 0;
}

/* 
* Return offset of the last message in buffer.
*/	
inline unsigned int connection_buffer_t::get_last_msg_offset(){
	
	return last_msg_offset;
}