#include "common.h"

connection_t::connection_t(int fd, int index) {

	this->fd = fd;
	this->index = index;
	valid = true;
	write_buffer = new connection_buffer_t(config.message_size);
	read_buffer = new connection_buffer_t(config.message_size);
	// 0.2 is overhead of test period because alarm is not accurate
	int queue_size = (int )( (config.period+0.2) * config.mps );
	if (!config.server) {
		unaknowledged_msgs = new queue_t<TicksTime>(queue_size);
		msgs_rtt = new queue_t<double>(queue_size);
	}
}

void connection_t::remove_connection() {

	if (write_buffer)
		delete write_buffer;
	if (read_buffer)
		delete read_buffer;
	valid = false;
}
