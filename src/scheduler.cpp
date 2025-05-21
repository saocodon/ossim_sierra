#include "scheduler.hpp"
#include <iostream>

pthread_mutex_t     scheduler::lock                  = PTHREAD_MUTEX_INITIALIZER;
int                 scheduler::last_visited_queue    = -1;
int                 scheduler::priority_budget[140]  = {};
std::list<process*> scheduler::ready_queue[140];
int                 scheduler::last_remaining_budget = 0;

void scheduler::init(void) {
  pthread_mutex_init(&lock, NULL);
  for (int i = 0; i < MAX_PRIORITY; i++)
    priority_budget[i] = MAX_PRIORITY - i;
}

bool scheduler::empty(void) {
  for (int i = 0; i < MAX_PRIORITY; i++)
    if (!ready_queue[i].empty())
      return false;
  return true;
}

process* scheduler::get(void) {
  process* p = nullptr;
  pthread_mutex_lock(&lock);
  for (int i = 0; i < MAX_PRIORITY; i++) {
    int priority = (last_visited_queue + 1 + i) % MAX_PRIORITY;
    int remaining_budget = last_remaining_budget;
    if (!ready_queue[priority].empty()) {
      p = ready_queue[priority].front();
      ready_queue[priority].pop_front();
      remaining_budget--;
      if (remaining_budget <= 0) {
        priority = (priority + 1) % MAX_PRIORITY;
        remaining_budget = priority_budget[priority];
      }
      // store the queue status
      last_remaining_budget = remaining_budget;
      last_visited_queue = priority;
      break;
    }
  }
  pthread_mutex_unlock(&lock);

  // if (p != nullptr) {
  //   std::cout << "[scheduler::get] returning PID=" << p->pid << "\n";
  // } else {
  //   std::cout << "[scheduler::get] no process returned\n";
  // }

  return p;
}

void scheduler::push(process* proc) {
  pthread_mutex_lock(&lock);
  ready_queue[proc->priority].push_back(proc);
  pthread_mutex_unlock(&lock);
}