#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstdlib>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include <deque>
#include <queue>
#include <map>
#include <chrono>

#include "cbuffer.h"

#define CHECK_EQUAL(verb, act_val, exp_val, cmd) if((exp_val) != (act_val)){ \
    printf("Error in %s, expected value %d, actual value %d\n",(verb), (exp_val), (act_val));			\
	if (errno != 0) printf("\"%s\"\n", strerror(errno));\
    cmd;                                                                \
  }

#define CHECK_NOT_EQUAL(verb, act_val, exp_val, cmd) if((exp_val) == (act_val)){ \
    printf("Error in %s (errno = %s)\n", (verb), strerror(errno));						\
	if (errno != 0) printf("\"%s\"\n", strerror(errno));\
    cmd;                                                                \
  }
  
#define WARM_UP_PERIOD 2 // Warm Up period in seconds
#define NUM_EXTRA_CONNECTION 40 // Warm Up period in seconds
#define SND_RCV_BUFFER_SIZE 4000 // Send and receive buffer size
#define NANO_SEC 1e9 // Number of nanosecond in second

enum extra_t {OPEN, CLOSE, NON};

struct config_t {
  	int	server; /* Is server */
  	char server_ip[20]; /* Server's IP */
  	int	server_port; /* Server's port */
  	int	message_size; /* Message Size */
  	int	mps; /* Message Rate Per Second */
	int period; /* period to run test */
	int	number_connection; /* number of connection */
	int	warm; /* warm up period */
	int	tcp_nodelay; /* TCP_NODELAY option */
	enum extra_t extra_op; /* Use extra connection for testing close, open or non. */ 
};

struct msg_t {
	int		fd;
	double rtt;
};

extern 	struct config_t config; // Test configuration
extern	bool is_finished; // flag to indicate that test finished.

class runner_t {
public:
        runner_t(){};
        virtual ~runner_t(){};
        virtual void run() = 0; // The scenario for running (server - client) side.
		virtual void print_result() = 0; // Print result of Test.
		virtual void init_socket() = 0; // Initialize socket depending on its side (server -client).
		virtual void handle_socket_write() = 0; // Handle available socket write (send) request that is filled in queue(FIFO).
		virtual void handle_socket_read() = 0; // Handle available read (receive) request that is filled in queue(FIFO).
		virtual void handle_connection_read() = 0; // Handle previously received messages that is filled in queue(FIFO).

		void	init_epoll();  
		int 	set_nonblocking_socket(int fd); 
		void 	add_new_connection(int fd); 
		void 	remove_connection(int fd); 
		void 	set_socket_option(int fd);
		
protected:
        unsigned int msg_count;
		int efd;
		std::queue<int> writable_socket_list; // List of socket file descriptor that need to be write on. 		
		std::queue<int> readable_socket_list; // List of socket file descriptor that need to be read from.
		std::queue<int> readable_connection_list; // List of read socket file descriptor that need to be handled.
		std::map<int, connection_buffer_t*> write_buffer_list; // Map for each socket file descriptor and it write buffer.
		std::map<int, connection_buffer_t*> read_buffer_list; // Map for each socket file descriptor and it read buffer.
		
		//Following are variable that used for temporary storage.
		int fd, rc, data_size;
		unsigned int* msg_id;
		uint8_t* start_address;
		std::map<int, connection_buffer_t*>::iterator iter;
};

class server_t : public runner_t{
public:
        server_t();
        virtual ~server_t();
        void run();
		void print_result();
		void init_socket();
		void handle_socket_write();
		void handle_socket_read();
		void handle_connection_read();
		
protected:
		int lfd; // listen socket FD.

		//Following are variable that used for temporary storage.
		struct sockaddr_in cli_addr;
		socklen_t cli_len;
};

class client_t : public runner_t {
public:
        client_t();
        virtual ~client_t();
        void run();
		void print_result();
		void init_socket();
		void handle_socket_write();
		void handle_socket_read();
		void handle_connection_read();
		void open_new_socket();
		void connect_new_socket(int sock_fd);

protected:
		std::deque<int> socket_list; // List of open socket file descriptor.
		std::map<unsigned int, std::chrono::high_resolution_clock::time_point> unaknowledged_packets;// Map for each send and not received message with its send time.
		std::queue<struct msg_t> msgs; // List of RTT for each generated message.
		std::map<unsigned int, std::chrono::high_resolution_clock::time_point>::iterator iter_send_time;
		std::map<int, int> fd_id;
		unsigned int tick_counter;
		bool connection_is_established;
		unsigned int sent_message;
		double send_period;
		
		//Following are variable that used for temporary storage.
		std::chrono::high_resolution_clock::time_point receive_time;
		struct msg_t msg;
};

