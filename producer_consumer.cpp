#include "producer_consumer.h"

#include <pthread.h>
#include <iostream>

#include <unistd.h>
#include <cstdlib>
#include <experimental/iterator>
#include <sstream>
#include <string>

struct storage {
  int value;
  bool valid;
  bool finished;
  pthread_cond_t writing_finished;
  pthread_cond_t reading_finished;
  pthread_mutex_t mutex;
};

struct thread_data {
  storage* storage_obj;
  int* my_tid_ptr;
};

unsigned int sleep_limit;
bool debug_flag;
std::istream* in_stream;

pthread_t* consumers;
thread_local int* tid_ptr;

int get_tid() {
  // 1 to 3+N thread ID
  return *tid_ptr;
}

void* producer_routine(void* arg) {
  thread_data* data = (thread_data*)arg;
  storage* place = data->storage_obj;
  tid_ptr = data->my_tid_ptr;
  // read data, loop through each value and update the value, notify consumer,
  // wait for consumer to process

  std::string line, token;
  std::getline(*in_stream, line);
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
  while (place->valid) {
    pthread_cond_wait(&place->reading_finished, &place->mutex);
  }
  place->finished = true;
  pthread_cond_broadcast(&place->writing_finished);
  pthread_mutex_unlock(&place->mutex);

  return nullptr;
}

void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  thread_data* data = (thread_data*)arg;
  storage* place = data->storage_obj;
  tid_ptr = data->my_tid_ptr;
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)

  size_t res = 0;
  while (1) {
    pthread_mutex_lock(&place->mutex);
    if (place->finished) break;
    while (!place->valid && !place->finished) {
      pthread_cond_wait(&place->writing_finished, &place->mutex);
    }
    if (place->valid) {
      res += place->value;
      if (debug_flag) std::cout << get_tid() << ": " << res << std::endl;
      place->valid = false;
      pthread_cond_signal(&place->reading_finished);
    }
    pthread_mutex_unlock(&place->mutex);
    if (sleep_limit > 0) usleep(rand() % sleep_limit);
  }

  pthread_mutex_unlock(&place->mutex);
  pthread_exit((void*)res);
}

void* consumer_interruptor_routine(void* arg) {
  thread_data* data = (thread_data*)arg;
  storage* place = data->storage_obj;
  tid_ptr = data->my_tid_ptr;
  // interrupt random consumer while producer is running
  while (1) {
    pthread_mutex_lock(&place->mutex);
    if (place->finished) break;
    pthread_cancel(
        consumers[rand() % (int(sizeof(consumers) / sizeof(consumers[0])))]);
    pthread_mutex_unlock(&place->mutex);
  }
  pthread_mutex_unlock(&place->mutex);
  return nullptr;
}

int run_threads(int cons_n, unsigned int max_sleep, bool debug,
                std::istream* input_stream) {
  // start N threads and wait until they're done
  // return aggregated sum of values
  if (cons_n == 0) return 0;
  int i;
  sleep_limit = max_sleep;
  debug_flag = debug;
  in_stream = input_stream;

  storage place;
  place.finished = false;
  place.valid = false;

  pthread_mutex_init(&place.mutex, NULL);
  pthread_cond_init(&place.writing_finished, NULL);
  pthread_cond_init(&place.reading_finished, NULL);

  consumers = (pthread_t*)malloc(cons_n * sizeof(pthread_t));
  pthread_t producer;
  pthread_t interruptor;

  thread_data* data = (thread_data*)malloc((cons_n + 2) * sizeof(thread_data));
  int* tids = (int*)malloc((cons_n + 2) * sizeof(int));
  for (i = 0; i < cons_n + 2; i++) tids[i] = i + 2;

  data[0] = {&place, tids};
  for (i = 0; i < cons_n; i++) data[i + 2] = {&place, tids + i + 2};
  data[1] = {&place, tids + 1};

  pthread_create(&producer, NULL, producer_routine, &data[0]);
  for (i = 0; i < cons_n; i++)
    pthread_create(&consumers[i], NULL, consumer_routine, &data[i + 2]);
  pthread_create(&interruptor, NULL, consumer_interruptor_routine, &data[1]);

  void** consumer_results = (void**)malloc(cons_n * sizeof(void*));
  size_t res = 0;

  pthread_join(producer, NULL);
  for (i = 0; i < cons_n; i++) {
    pthread_join(consumers[i], &consumer_results[i]);
    res += (size_t)consumer_results[i];
  }
  pthread_join(interruptor, NULL);

  free(consumers);
  free(consumer_results);
  free(data);
  free(tids);

  return res;
}
