
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
			
