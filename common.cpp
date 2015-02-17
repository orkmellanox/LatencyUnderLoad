#include <netinet/tcp.h>
#include "common.h"

/*
* Initialize EPOLL.
*/
void runner_t::init_epoll() {
	efd = epoll_create(MAX_QUEUED_MASSAGE*config.number_connection);
	CHECK_NOT_EQUAL("epoll_create", efd, -1, exit (-1));
}

/*
* Change socket mode to non-blocking for given socket FD. 
*/
int runner_t::set_nonblocking_socket(int fd)
{
	int flags, ret;
	flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		printf("ERROR: fcntl(F_GETFL)\n");
		return -1;
	}
	flags |=  O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret < 0) {
		printf("ERROR: fcntl(F_SETFL)\n");
		return -1;
	}
	return 0;
}

/*
* Reserve memory and register event related to specific socket (register in epoll and reserve send and receive buffer).
*/
void runner_t::add_new_connection(int fd) {
	int rc;
	struct epoll_event ev;

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = fd;
	rc = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev); 
	CHECK_NOT_EQUAL("epoll_ctl EPOLL_CTL_ADD", rc, -1, exit(-1));
	
	// Initialize read and write buffer for given socket file descriptor
	write_buffer_list[fd] 	= new connection_buffer_t(config.message_size);
	read_buffer_list[fd] 	= new connection_buffer_t(config.message_size);
}

/*
* Remove reserved memory and event related to specific socket (remove from epoll, free reserved send and receive buffer and close fd).
*/
void runner_t::remove_connection(int fd){
	std::map<int, connection_buffer_t*>::iterator iter;
	
	write_buffer_list.erase(write_buffer_list.find(fd));
	read_buffer_list.erase(read_buffer_list.find(fd));			  
	
	epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);	
}

/*
* Set following option for given socket FD (send buffer size, receive buffer size, change socket mode to non_blocking and set TCP_NODELAY option if request).
*/
void runner_t::set_socket_option(int fd){

	int rc;
	size_t snd_rcv_buf;

	snd_rcv_buf = SND_RCV_BUFFER_SIZE;
	
	rc = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &snd_rcv_buf, sizeof(snd_rcv_buf));
	CHECK_NOT_EQUAL("setsockopt", rc, -1, exit(-1));
	
	rc = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &snd_rcv_buf, sizeof(snd_rcv_buf));
	CHECK_NOT_EQUAL("setsockopt", rc, -1, exit(-1));
	
	if (config.tcp_nodelay) {
		size_t is_enabled;

		is_enabled = 1;
		rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &is_enabled, sizeof(is_enabled));
		CHECK_NOT_EQUAL("setsockopt", rc, -1, exit(-1));
	}
	
	rc = set_nonblocking_socket(fd);
	CHECK_NOT_EQUAL("set_nonblocking_socket", rc, -1, exit(-1));
}