#include <iostream>
#include "producer_consumer.h"

  

int main(int argc, char** argv) {
  int cons_n = std::atoi(argv[1]);
  int max_sleep = std::atoi(argv[2]);
  std::cout << run_threads(cons_n, max_sleep) << std::endl;
  return 0;
}
