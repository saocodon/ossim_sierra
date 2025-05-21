#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <list>
#include <pthread.h>
#include "common.hpp"

const int MAX_PRIORITY = 140;

class scheduler {
private:
  static std::list<process*> ready_queue[MAX_PRIORITY];  // static struct queue_t mlq_ready_queue[MAX_PRIO]
  static pthread_mutex_t lock;                           // static pthread_mutex_t queue_lock

  static int last_visited_queue;                         // static uint32_t curr_prio
  static int last_remaining_budget;                      // static int curr_slot
  static int priority_budget[MAX_PRIORITY];              // static int slot[MAX_PRIO]
public:
  static void init(void);                                // void init_scheduler(void)
  static bool empty(void);                               // int queue_empty(void)
  static process* get(void);                             // struct pcb_t * get_mlq_proc(void)
  static void push(process* proc);                       // void put_mlq_proc(struct pcb_t * proc)

  // system calls
  friend int __kill_process_tree(process*, registers_set*);
};

#endif