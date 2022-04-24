#pragma once
#include <iostream>

// the declaration of run threads can be changed as you like
int run_threads(int cons_n, unsigned int max_sleep, bool debug,
                std::istream* input_stream);
