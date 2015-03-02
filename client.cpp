#include <signal.h>

#include "common.h"
  
/* 
* Handle time_out interrupt. (End test on client side)
*/
void handle_test_timeout(int sig_num)
{
	is_finished = true;
} 

client_t::client_t() : socket_list(config.number_connection + NUM_EXTRA_CONNECTION)
{	
	tick_counter = 0;
	sent_message = 0;
	send_period = NSEC_IN_SEC / (config.mps * config.number_connection); // period in nano second between succeed send iteration
	connection_is_established = false;

	init_epoll();
	init_socket();
	signal(SIGALRM, handle_test_timeout); // Set alarm handler.
}

client_t::~client_t(){
	
	int rc, fd;
		
	alarm(0); 
	rc = close(efd);	
	CHECK_NOT_EQUAL("close", rc, -1, exit(-1));
}

void client_t::init_socket() {
	int num_socket;

	config.number_connection = (config.extra_op != CLOSE) ? config.number_connection : (config.number_connection + NUM_EXTRA_CONNECTION);

	// Initialize num_socket number of socket equal to one given in configuration.
	for (num_socket = 0; num_socket < config.number_connection; num_socket++) {
		open_new_socket();
	}
}

void client_t::open_new_socket() {
	int rc, sock_fd;
	struct epoll_event ev;
	struct sockaddr_in server_addr;
	
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family		=	AF_INET;
	server_addr.sin_addr.s_addr	=	inet_addr(config.server_ip);
	server_addr.sin_port		=	htons(config.server_port);
	
	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	CHECK_NOT_EQUAL("connect", sock_fd, -1, exit(-1));
	
	set_socket_option(sock_fd);

	rc = connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if ((rc == -1) && (errno != EINPROGRESS)) {
		printf("Error in connect() \n"); 
		exit(-1);
	}
	else {
		ev.events = EPOLLOUT | EPOLLET;
		ev.data.fd = sock_fd;
		rc = epoll_ctl(efd, EPOLL_CTL_ADD, sock_fd, &ev); 
		CHECK_NOT_EQUAL("epoll_ctl", rc, -1, exit(-1));	 
	}
}

void client_t::connect_new_socket(int sock_fd) {

	epoll_ctl(efd, EPOLL_CTL_DEL, sock_fd, NULL);
	
	socket_list.push(sock_fd);

	send_period = NSEC_IN_SEC / (config.mps * socket_list.size());
	
	add_new_connection(sock_fd);	
	
}

void client_t::run(){
	
	double new_connection_period;
	TicksTime last_send, last_conn, current;
	struct epoll_event events[MAX_QUEUED_MASSAGE*config.number_connection];

	new_connection_period = (NSEC_IN_SEC * config.period) / 2; // period in nano second between succeed new socket opening
	
	// Will finish when time-out event is handled from timer that we set previously.  
	while (!is_finished) {
		
		// Handle available read (receive) request that is filled in queue(FIFO).
		int list_size = readable_socket_list.size();
                for (int i = 0; i < list_size; i++)
                        handle_socket_read();

		// Handle read buffer and calculate rtt from received time and saved time in unaknowledged_packets.
                list_size = readable_connection_list.size();
		for (int i = 0; i < list_size; i++)
                        handle_connection_read();

                if ((!connection_is_established) && (socket_list.size() >= config.number_connection)) {
                        connection_is_established = true;
						last_send.setNow();
						last_conn.setNow();
                        alarm(config.period + config.warm); // Start timer equal to period given in configuration.
                }

		// If time elapsed from last send iteration >= send_period then a new send iteration should be handled.
		if (connection_is_established) {
			current.setNow();
                        if ((config.extra_op != NON) && ((current - last_conn).toNsec() >=  new_connection_period)) {
                                for (int i = 0; i < NUM_EXTRA_CONNECTION; i++) {
                                        if (config.extra_op == OPEN) {
                                                open_new_socket();
                                        }
                                        else {
                                                remove_connection(socket_list.front());
                                                socket_list.pop();
                                        }
                                }
                                last_conn.setNow();
                                new_connection_period = NSEC_IN_SEC * config.period;
                        }
			
			if ((current - last_send).toNsec() >=  send_period){
				fd = socket_list.front();
				socket_list.push(fd);
				socket_list.pop();
				
				current_connection = connection_list[fd];
				write_buffer = current_connection->get_write_buffer();
				start_address = write_buffer->reserve_memory();
				if (start_address != NULL){ // if there is space in connection buffer then copy new message to connection buffer.
					memcpy(start_address, &tick_counter, sizeof(tick_counter));// tick_counter is used as message since its unique.
					current_connection->add_unaknowledged_msg();
					writable_socket_list.push(current_connection);
				}
				tick_counter++;
				last_send.setNow();
			}
		}
		
		// Handle available socket write (send) request that is filled in queue(FIFO).
		list_size = writable_socket_list.size();
		for (int i = 0; i < list_size; i++)
			handle_socket_write();

		// check ready socket that received enough data (>= message size) and push them in read queue 
		rc = epoll_wait(efd, events, MAX_QUEUED_MASSAGE * config.number_connection, 0);
		CHECK_NOT_EQUAL("epoll_wait", rc, -1, exit(-1));
		for (int index = 0; index < rc; index++) {
			if (events[index].events & EPOLLOUT) {
				connect_new_socket(events[index].data.fd);
			}
			else {
				current_connection = connection_list[events[index].data.fd];
				readable_socket_list.push(current_connection);
			}
		}
	}
	
	while (socket_list.size()) {
		remove_connection(socket_list.front());
		socket_list.pop();
	}	
}

void client_t::handle_socket_write(){

	current_connection = writable_socket_list.front();
	writable_socket_list.pop();

	if (current_connection->is_valid()) {
		write_buffer = current_connection->get_write_buffer();
		start_address = write_buffer->get_buffer_head();
		data_size = config.message_size - write_buffer->get_first_msg_offset();
		
		rc = send(current_connection->get_fd(), start_address, data_size, 0);
		if (rc >= 0) {
			write_buffer->increase_first_msg_offset(rc);
			//Full message is sent
			if (rc == data_size){
				write_buffer->free_memory();
				sent_message++;
			}
			//Message is not sent completely => reschedule				
			else {
				writable_socket_list.push(current_connection);	
			}
		}
		else {
			if (errno == EAGAIN) {
				writable_socket_list.push(current_connection);	
			}
			else {
				printf("Error in send() \"%s\"\n", strerror(errno));
				exit(-1);
			}
		}
	}
}

void client_t::handle_socket_read(){

	current_connection = readable_socket_list.front();
	readable_socket_list.pop();
	
	if (current_connection->is_valid()) { 
		read_buffer = current_connection->get_read_buffer();
		while(true) {
			start_address = read_buffer->reserve_memory();
			if (start_address != NULL){
				data_size = config.message_size - read_buffer->get_last_msg_offset();
				rc = recv(current_connection->get_fd(), start_address, data_size, 0);
				if (rc > 0) {
					read_buffer->increase_last_msg_offset(rc);
					//Full message is received
					if (rc == data_size) {
						readable_connection_list.push(current_connection);
					}
				}
				else if (rc <= 0) {
					if ((rc == -1) && (errno == EAGAIN)) {
						break;
					}
					else {
						printf("Error in recv() \"%s\"\n", strerror(errno)); 
						exit(-1);
					}
				}
			}	
			//No enough buffer to receive message => reschedule			
			else{
				readable_socket_list.push(current_connection);
				break;
			}
		}
	}
}

void client_t::handle_connection_read(){

	current_connection = readable_connection_list.front();
	readable_connection_list.pop();

	if (current_connection->is_valid()) { 
		current_connection->record_msg_rtt();
		read_buffer = current_connection->get_read_buffer();
		read_buffer->free_memory();
	}
}		

void client_t::print_result() {
	std::map<int, connection_t*>::iterator iter;
	queue_t<double>* msgs_rtt;
	unsigned int index, ignored_messages;
	FILE*  output_file;
	char file_name[50];
	double latency_sum;
	
	index = 0;
	latency_sum = 0;
	
	sprintf(file_name, "./data_%d.csv", getpid());
	output_file= fopen(file_name, "w");
	ignored_messages = config.warm * config.mps;

	// Save data to file.
	  for (iter=connection_list.begin(); iter!=connection_list.end(); iter++) {
		index = 0;
		msgs_rtt = iter->second->get_msgs_rtt();
		while (msgs_rtt->size()){
			index++;	
			if (index > ignored_messages) {
				fprintf(output_file, "%u,%f,%d\n", (index- ignored_messages), msgs_rtt->front(), iter->second->get_index());
				latency_sum += msgs_rtt->front();
			}
			msgs_rtt->pop();
		}	
		if (index > ignored_messages)
			msg_count += index;
	}
	
	if (msg_count > 0) {
		printf("Sent %u messages, sent message rate = %.1f mps\n", sent_message, (sent_message / ((double)config.period * config.number_connection)));
		printf("Received %u messages, received message rate = %.1f mps\n", msg_count, (msg_count / ((double)config.period * config.number_connection)));
		printf("Average latency = %f\n", (latency_sum / msg_count));
	}
}

