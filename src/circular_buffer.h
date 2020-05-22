#include "stdio.h" 
#include "sys/types.h"
#include "stdlib.h"
#include "string.h"

typedef struct circular_buffer
{
    void *buffer;     // data buffer
    void *buffer_end; // end of data buffer
    size_t capacity;  // maximum number of items in the buffer
    size_t count;     // number of items in the buffer
    size_t sz;        // size of each item in the buffer
    void *head;       // pointer to head
    void *tail;       // pointer to tail
} circular_buffer;

//initialize stack
void cb_init(circular_buffer *cb, size_t capacity, size_t sz);

//free memory of circular buffer
void cb_free(circular_buffer *cb);

//vazei sti stoiva ton arithmo
void cb_push_back(circular_buffer *cb, const void *item);

//vgazei apo tin stoiva ton arithmo
void cb_pop_front(circular_buffer *cb, void *item);
