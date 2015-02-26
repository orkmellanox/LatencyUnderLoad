#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstdlib>

/* This class represent simple queue. */
template <typename queue_element_t>
class queue_t {
public:
    queue_t(int max_queue_size);
    virtual ~queue_t();
	void push(queue_element_t element);
	void pop();
	queue_element_t front();
	unsigned int size();
	
protected:
	int max_queue_size; // Maximum number of elements that can be queued.
  	queue_element_t* element_list;
	int first_element_index;
	int last_element_index;
	unsigned int queue_size;
};

template <typename queue_element_t>
queue_t<queue_element_t>::queue_t(int max_queue_size) {
	this->max_queue_size = max_queue_size;
	element_list = (queue_element_t*) malloc(max_queue_size * sizeof(queue_element_t));
	first_element_index = 0;
	last_element_index = 0;
	queue_size = 0;
}

template <typename queue_element_t>
queue_t<queue_element_t>::~queue_t() {
	free(element_list);
}

template <typename queue_element_t>
inline void queue_t<queue_element_t>::push(queue_element_t element) {
	if (queue_size < max_queue_size) { 
		element_list[last_element_index] = element;
		last_element_index++;		
		if (last_element_index >= max_queue_size) {
			last_element_index = 0;
		}
		queue_size++;
	}
	else {
		printf("No available space in queue\n");
	}
}

template <typename queue_element_t>
inline void queue_t<queue_element_t>::pop() {
	if (queue_size > 0) { 
		first_element_index++;
		if (first_element_index >= max_queue_size) {
			first_element_index = 0;
		}
		queue_size--;
	}
	else {
		printf("No elements in queue\n");
	}	
}

template <typename queue_element_t>
inline queue_element_t queue_t<queue_element_t>::front() {

	if (queue_size <= 0) {
		printf("No elements in queue\n");
	}
	
	return element_list[first_element_index];
}

template <typename queue_element_t>
inline unsigned int queue_t<queue_element_t>::size() {
	return queue_size;
}