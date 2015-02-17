
#include "cbuffer.h"

connection_buffer_t::connection_buffer_t(unsigned int msg_size){
	this->msg_size = msg_size;
	buffer_size = this->msg_size * MAX_QUEUED_MASSAGE;
	first_element 		= 0;
	last_element 		= 0;
	last_msg_offset		= 0;
	first_msg_offset	= 0;
	buffer = (uint8_t*) malloc(buffer_size);
	bzero(buffer, buffer_size);
}

connection_buffer_t::~connection_buffer_t(){
	free(buffer);
}

/* 
* Reserve memory from circle buffer equal to message size defined in constructor.
* Return pointer to the reserved memory if there enough free memory, NULL otherwise.
*/
uint8_t* connection_buffer_t::reserve_memory(){
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
void connection_buffer_t::free_memory(){

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
uint8_t* connection_buffer_t::get_buffer_head(){

	return buffer + first_element + first_msg_offset;
}

/* 
* Increase first message offset with offset amount and back to zero in reach default msg_size.
*/	
void connection_buffer_t::increase_first_msg_offset(unsigned int offset){
	
	first_msg_offset += offset;
	if (first_msg_offset == msg_size)
		first_msg_offset = 0;
}

/* 
* Return offset of the first message in buffer.
*/	
unsigned int connection_buffer_t::get_first_msg_offset(){
	
	return first_msg_offset;
}
	
/* 
* Increase last message offset with offset amount and back to zero in reach default msg_size.
*/	
void connection_buffer_t::increase_last_msg_offset(unsigned int offset){
	
	last_msg_offset += offset;
	if (last_msg_offset == msg_size)
		last_msg_offset = 0;
}

/* 
* Return offset of the last message in buffer.
*/	
unsigned int connection_buffer_t::get_last_msg_offset(){
	
	return last_msg_offset;
}
			
