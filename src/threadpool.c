#include <pthread.h>
#include <stdio.h>
#include "threadpool.h"


void threadpool_init(threadpool_t* pool) {
  pool->queued = 0;
  pool->queue_front = 0;
  pool->queue_back = 0;
  pool->stop = 0;

  pthread_mutex_init(&(pool->lock), NULL);
  pthread_cond_init(&(pool->notify), NULL);

  for (int i = 0; i < THREADS; i++) {
    pthread_create(&(pool->threads[i]), NULL, (void*)example_task, pool);    
  }
}

//void threadpool_destroy(threadpool_t* pool) {
//
//}
//
//void threadpool_add_task(threadpool_t* pool, void (*function)(void*), void* arg) {
//
//}
//
void example_task(void* arg) {
  printf("hello, world!\n");
}
