#ifndef CPU_HPP
#define CPU_HPP

#include "timer.hpp"
#include "scheduler.hpp"

class cpu {
private:
  timer      t;
  int        id;
  int        quantum_time;
  pthread_t  thread;
  bool       done;

public:
  cpu(int id, int quantum_time);

  // instructions
  inline int   calc(process* proc)                 { return ((unsigned long)proc & 0UL); }

  void*        routine();                          // static void * cpu_routine(void * args)
  int          execute_instruction(process* proc);
  static void* thread_entry(void* args);

  void         run()                               { pthread_join(thread, NULL); }
  void         halt();
};

#endif