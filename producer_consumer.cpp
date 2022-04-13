#include <pthread.h>
#include <iostream>

#include <string>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <experimental/iterator>

struct storage {
    int value;
    bool valid;
    bool finished;
    pthread_cond_t writing_finished;
    pthread_cond_t reading_finished;
    pthread_mutex_t mutex;
};

unsigned int sleep_limit;
pthread_t* consumers;

int get_tid() {
  // 1 to 3+N thread ID
  return 0;
}

void* producer_routine(void* arg) {
  storage* place = (storage*) arg;
  // read data, loop through each value and update the value, notify consumer,
  // wait for consumer to process

  std::string line, token;
  std::getline(std::cin, line);
  std::stringstream S(line);

  while (S >> token) {
    pthread_mutex_lock(&place->mutex);
    while (place->valid) {
      pthread_cond_wait(&place->reading_finished, &place->mutex);
    }
    place->value = std::stoi(token);
    place->valid = true;
    pthread_cond_signal(&place->writing_finished);
    pthread_mutex_unlock(&place->mutex);
  }

  pthread_mutex_lock(&place->mutex);
  place->finished = true;
  pthread_cond_broadcast(&place->writing_finished);
  pthread_mutex_unlock(&place->mutex);

  return nullptr;
}

void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  storage* place = (storage*) arg;
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)

  int res = 0;
  while (1) {
    pthread_mutex_lock(&place->mutex);
    if (place->finished) break;
    while (!place->valid && !place->finished) {
      pthread_cond_wait(&place->writing_finished, &place->mutex);
    }
    if (place->valid) {
      res += place->value;
      place->valid = false;
      pthread_cond_signal(&place->reading_finished);
    }
    pthread_mutex_unlock(&place->mutex);
    if (sleep_limit > 0) usleep(rand()%sleep_limit);
  }

  pthread_mutex_unlock(&place->mutex);
  pthread_exit((void*)res);
}

void* consumer_interruptor_routine(void* arg) {
  storage* place = (storage*) arg;
  // interrupt random consumer while producer is running
  while(1) {
    pthread_mutex_lock(&place->mutex);
    if (place->finished) break;
    pthread_cancel(consumers[rand()%(int(sizeof(consumers) / sizeof(consumers[0])))]);
    pthread_mutex_unlock(&place->mutex);
  }
  pthread_mutex_unlock(&place->mutex);
  return nullptr;
}

int run_threads(int cons_n, unsigned int max_sleep) {
  // start N threads and wait until they're done
  // return aggregated sum of values
  int i = 0;
  storage place = {NULL, false, false, NULL, NULL, NULL};
  sleep_limit = max_sleep;
  
  pthread_mutex_init(&place.mutex, NULL);
  pthread_cond_init(&place.writing_finished, NULL);
  pthread_cond_init(&place.reading_finished, NULL);

	consumers = (pthread_t*) malloc(cons_n * sizeof(pthread_t));
  pthread_t producer;
  pthread_t interruptor;

  pthread_create(&producer, NULL, producer_routine, &place);
	for(i = 0; i < cons_n; i++) {
		pthread_create(&consumers[i], NULL, consumer_routine, &place);
	}
  pthread_create(&interruptor, NULL, consumer_interruptor_routine, &place);

  void** consumer_results = (void**) malloc(cons_n * sizeof(void*));
  int res = 0;

  pthread_join(producer, NULL);
	for(i = 0; i < cons_n; i++) {
		pthread_join(consumers[i], &consumer_results[i]);
    res += (int) consumer_results[i];
	}
  pthread_join(interruptor, NULL);

  return res;
}
