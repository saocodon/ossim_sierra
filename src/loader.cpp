#include "loader.hpp"
#include "logger.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

bool                 loader::good      = true;
memory*              loader::ram       = nullptr;
memory*              loader::swap      = nullptr;
pthread_t            loader::thread;
timer                loader::t;

void loader::init(memory* ram, memory* swap) {
  loader::ram    = ram;
  loader::swap   = swap;

  pthread_create(&loader::thread, NULL, routine, NULL);
}

void* loader::routine(void* args) {
  for (int i = 0; i < configuration::processes_count; i++) {
    process* proc = new process(ram, swap, configuration::processes[i].path, configuration::processes[i].priority);
    if (!proc->good) {
      std::stringstream ss;
      ss << "Executable " << proc->path << " not found.\n";
      logger::log(ss.str());
      good = false;
      break;
    }

    while (t.get_time() < configuration::processes[i].arrival_time)
      t.tick();
    logger::log(" Loaded process " + std::to_string(proc->pid) + " with priority " + std::to_string(proc->priority) + ".\n");
    scheduler::push(proc);
    t.tick();
  }
  t.unset();
  pthread_exit(NULL);
}