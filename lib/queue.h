#ifndef ZD0E437531E4E79A568A44500452B941A
#define ZD0E437531E4E79A568A44500452B941A

typedef struct queue *queue_t;

queue_t queue_new(void);
void    queue_free(queue_t queue);

int     queue_is_empty(queue_t queue);
int     queue_enqueue(queue_t queue, void *data);
void *  queue_dequeue(queue_t queue);

#endif
