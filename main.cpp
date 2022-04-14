#include <iostream>
#include "producer_consumer.h"

int main(int argc, char** argv) {
  bool debug = false;
  if (argc == 4) {
    std::string arg(argv[3]);
    std::string debugstr("-debug");
    debug = arg == debugstr;
  }
  int cons_n = std::atoi(argv[1]);
  int max_sleep = std::atoi(argv[2]);
  std::cout << run_threads(cons_n, max_sleep, debug) << std::endl;
  return 0;
}
