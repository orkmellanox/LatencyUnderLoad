#include <signal.h>

#include "common.h"

/* 
* Handle SIGINT signal. (End test on server side)
*/
void handle_interupt_signal(int sig_num)
{
	is_finished = true;
} 

server_t::server_t(){

	lfd = -1;
	init_epoll();
	init_socket();
	signal(SIGINT, handle_interupt_signal);
}

server_t::~server_t(){
	int rc;
	//remove listen fd from epoll
	rc = epoll_ctl(efd, EPOLL_CTL_DEL, lfd, NULL);
	CHECK_NOT_EQUAL("epoll_ctl", rc, -1, exit(-1));
	//close listen fd
	rc = close(lfd); // Close listen FD.	
	CHECK_NOT_EQUAL("close", rc, -1, exit(-1));	
	//close epoll fd
	rc = close(efd); // Close epoll FD.		
	CHECK_NOT_EQUAL("close", rc, -1, exit(-1));
}

void server_t::init_socket() {

	int rc;
	int num_socket = 0;
	size_t min_receive_bytes, snd_rcv_buf;
	struct epoll_event ev;
	struct sockaddr_in server_addr;

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family		=	AF_INET;
	server_addr.sin_addr.s_addr	=	inet_addr(config.server_ip);
	server_addr.sin_port		=	htons(config.server_port);

	min_receive_bytes = config.message_size;
	snd_rcv_buf = SND_RCV_BUFFER_SIZE;

	lfd = socket(AF_INET,SOCK_STREAM,0);
	CHECK_NOT_EQUAL("socket", lfd, -1, exit(-1));
	
	rc = bind(lfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	CHECK_NOT_EQUAL("bind", rc, -1, exit(-1));
	
	set_socket_option(lfd);
	
	rc = listen(lfd, SOMAXCONN);
	CHECK_NOT_EQUAL("listen", rc, -1, exit(-1));
	
	ev.events = EPOLLIN;
	ev.data.fd = lfd;
	rc = epoll_ctl(efd, EPOLL_CTL_ADD, lfd, &ev); 
	CHECK_NOT_EQUAL("epoll_ctl", rc, -1, exit(-1));
}

void server_t::run(){

	int rc;
	struct epoll_event events[MAX_QUEUED_MASSAGE*config.number_connection];

	
	while (!is_finished) {

		// Handle available read (receive) request that is filled in queue(FIFO).
		int list_size = readable_socket_list.size();
		for (int i = 0; i < list_size; i++)
			handle_socket_read();
		
		// Handle read buffer by writing it to circle buffer and push socket fd to write waiting list.
		list_size = readable_connection_list.size();
		for (int i = 0; i < list_size; i++)
			handle_connection_read();

                // Handle available socket write (send) request that is filled in queue(FIFO).
		list_size = writable_socket_list.size();
		for (int i = 0; i < list_size; i++)
                        handle_socket_write();

		// check ready socket that received enough data (>= message size) and push them in read queue 
		rc = epoll_wait(efd, events, MAX_QUEUED_MASSAGE*config.number_connection, 0);
		CHECK_NOT_EQUAL("epoll_wait", rc, -1, exit(-1));
		
		for (int index = 0; index < rc; index++)
			readable_socket_list.push(events[index].data.fd);
	}
}


void server_t::handle_socket_write(){
	
	fd = writable_socket_list.front();
	writable_socket_list.pop();

	iter = write_buffer_list.find(fd);
	if (iter != write_buffer_list.end()) { 
		start_address = iter->second->get_buffer_head();
		data_size = config.message_size - iter->second->get_first_msg_offset();
		
		rc = send(fd, start_address, data_size, 0);
		if (rc >= 0) {
			//Full message is sent
			if (rc == data_size){
				msg_count++;
				iter->second->free_memory();
			}
			//Message is not sent completely => reschedule				
			else {
				writable_socket_list.push(fd);	
			}
			iter->second->increase_first_msg_offset(rc);
		}
		else {
			if (errno == EAGAIN) {
				writable_socket_list.push(fd);	
			}
			else if (errno == ECONNRESET) {
				remove_connection(fd);
			}	
			else if (errno != EBADF) {
				printf("Error in send() \"%s\"\n", strerror(errno));
				exit(-1);
			}		
		}
	}
}

void server_t::handle_socket_read(){
	
	fd = readable_socket_list.front();
	readable_socket_list.pop();

	iter = read_buffer_list.find(fd);
	//New connection is requested
	if (fd == lfd) {
		bzero(&cli_addr, sizeof(cli_addr));
		cli_len = sizeof(cli_addr);
		fd = accept(lfd, (struct sockaddr *)&cli_addr, &cli_len);
		if (fd == -1) {
			if (errno != EAGAIN){
				printf("Error in accept() \"%s\"\n", strerror(errno)); 
				exit(-1);
			}
		}
		else {
			set_socket_option(fd);
			add_new_connection(fd);
		}
	}
	// data is received
	else if (iter != read_buffer_list.end()) { 
		while(true) {
			start_address = iter->second->reserve_memory();
			if (start_address != NULL){
				data_size = config.message_size - iter->second->get_last_msg_offset();
				rc = recv(fd, start_address, data_size, 0);
				if (rc > 0) {
					iter->second->increase_last_msg_offset(rc);
					//Full message is received
					if (rc == data_size) {
						readable_connection_list.push(fd);
					}
				}
				else if ((rc == 0) || ((rc == -1) && (errno == ECONNRESET))) {
					remove_connection(fd);
					break;
				}
				else {
					if ((errno == EAGAIN) || (errno == EBADF)) {
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
				readable_socket_list.push(fd);
				break;
			}
		}
	}
}

void server_t::handle_connection_read(){

	fd = readable_connection_list.front();
	readable_connection_list.pop();

	iter = read_buffer_list.find(fd);
	if (iter != read_buffer_list.end()) { 
		msg_id = (unsigned int*)iter->second->get_buffer_head();
		start_address = write_buffer_list[fd]->reserve_memory();
		if (start_address != NULL){
			memcpy(start_address, msg_id, sizeof(*msg_id));
			writable_socket_list.push(fd);	
			iter->second->free_memory();
		}
		//No enough buffer to send message => reschedule	
		else{
			readable_connection_list.push(fd);
		}
	}
}		

void server_t::print_result(){
	
	unsigned int ignored_messages;

	ignored_messages = config.warm * config.mps;

	if (msg_count > ignored_messages)
		printf("Received %u messages\n", (msg_count - ignored_messages));
	
}
