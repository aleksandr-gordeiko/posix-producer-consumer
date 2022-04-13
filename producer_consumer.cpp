#include <pthread.h>
#include <iostream>

struct mutex_int {
    int value;
    pthread_mutex_t mutex;
};

thread_local int* tid_ptr;
int* thread_numbers;

int get_tid() {
  // 1 to 3+N thread ID
  return *tid_ptr;
}

void* producer_routine(void* arg) {
  (void)arg;
  // read data, loop through each value and update the value, notify consumer,
  // wait for consumer to process
  return nullptr;
}

void* consumer_routine(void* arg) {
  (void)arg;
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)
  return nullptr;
}

void* consumer_interruptor_routine(void* arg) {
  (void)arg;
  // interrupt random consumer while producer is running
  return nullptr;
}

int run_threads(int cons_n, int max_sleep) {
  // start N threads and wait until they're done
  // return aggregated sum of values
  int i = 0;
  struct mutex_int place;
  
  thread_numbers = (int*) malloc((cons_n + 2) * sizeof(int));
  pthread_mutex_init(&place.mutex, NULL);

	pthread_t* consumers = (pthread_t*) malloc(cons_n * sizeof(pthread_t));
  pthread_t producer;
  pthread_t interruptor;

  pthread_create(&producer, NULL, producer_routine, &place);
  pthread_create(&interruptor, NULL, consumer_interruptor_routine, &place);
	for(i = 0; i < cons_n; i++) {
		pthread_create(&consumers[i], NULL, consumer_routine, &place);
	}

  pthread_join(producer, NULL);
  pthread_join(interruptor, NULL);
	for(i = 0; i < cons_n; i++) {
		pthread_join(consumers[i], NULL);
	}

  return 0;
}
