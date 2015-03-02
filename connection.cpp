#include "common.h"

connection_t::connection_t(int fd, int index) {
		
	this->fd = fd;
	this->index = index;
	valid = true;
	write_buffer = new connection_buffer_t(config.message_size);
	read_buffer = new connection_buffer_t(config.message_size);
	if (!config.server) { 
		unaknowledged_msgs = new queue_t<TicksTime>(config.period * config.mps);
		msgs_rtt = new queue_t<double>(config.period * config.mps);
	}
}

void connection_t::remove_connection() {

	if (write_buffer)
		delete write_buffer;
	if (read_buffer)
		delete read_buffer;
	valid = false;
}
