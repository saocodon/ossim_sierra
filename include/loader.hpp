#ifndef LOADER_HPP
#define LOADER_HPP

#include <pthread.h>
#include "common.hpp"
#include "memory.hpp"
#include "configuration.hpp"
#include "timer.hpp"
#include "scheduler.hpp"

class loader {
private:
  static pthread_t             thread;          // pthread_t ld
  static timer                 t;

  static memory*               ram;
  static memory*               swap;
public:
  static bool                  good;

  static void  init(memory* ram, memory* swap);
  static void* routine(void* args);             // static void * ld_routine(void * args)
};

#endif