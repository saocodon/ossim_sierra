#include "timer.hpp"
#include "logger.hpp"
#include <iostream>
#include <sstream>

uint64_t timer::time = 0;

struct timer_args {
  timer* self;
  void*  args;
};

timer::timer() {
  started = 0; done = 0; fsh = 0; time = 0;
  pthread_cond_init(&event_condition, NULL);
  pthread_mutex_init(&event_lock, NULL);
  pthread_cond_init(&timer_condition, NULL);
  pthread_mutex_init(&timer_lock, NULL);
  set();
}

timer::~timer() {
  pthread_cond_destroy(&event_condition);
  pthread_mutex_destroy(&event_lock);
  pthread_cond_destroy(&timer_condition);
  pthread_mutex_destroy(&timer_lock);
}

void timer::start(void* raw_args) {
  started = true;
  pthread_create(&thread, NULL, timer::thread_entry, new timer_args { this, raw_args });
}

void timer::stop() {
  started = false;

  // Fire up the last CPU burst so the timer can exit the deadlock
  // when the CPU shuts down
  for (auto& t : timer_manager::get_list()) {
    pthread_mutex_lock(&t->event_lock);
    pthread_cond_signal(&t->event_condition);
    pthread_mutex_unlock(&t->event_lock);
  }

  pthread_join(thread, NULL);
}

void timer::set() {
  if (started) return;
  timer_manager::get_list().push_front(this);
}

void timer::unset() {
  pthread_mutex_lock(&event_lock);
  fsh = 1;
  pthread_cond_signal(&event_condition);
  pthread_mutex_unlock(&event_lock);
}

void* timer::routine(void* args) {
  while (started) {
    std::stringstream ss;
    ss << "Time slot " << time << "\n";
    logger::log(ss.str());
    // count the number of finished jobs and events
    int fsh = 0, event = 0;
    for (auto t : timer_manager::get_list()) {
      pthread_mutex_lock(&t->event_lock);
      // sleep the event thread, release event_lock, allowing timer thread to work until event_condition rings
      while (!t->done && !t->fsh) {
        // when the CPU shuts down, the event thread will never be woken up
        // causing a deadlock
        if (!started) break;
        pthread_cond_wait(&t->event_condition, &t->event_lock);
      }
      if (t->fsh) fsh++;
      event++;
      pthread_mutex_unlock(&t->event_lock);
    }
    time++;
    for (auto &t : timer_manager::get_list()) {
      pthread_mutex_lock(&t->timer_lock);
      t->done = 0;
      pthread_cond_signal(&t->timer_condition);
      pthread_mutex_unlock(&t->timer_lock);
    }
    if (fsh == event) break;
  }
  logger::log("Timer " + std::to_string(id) + ": All events finished.\n");
  pthread_exit(args);
}

void timer::tick() {
  pthread_mutex_lock(&event_lock);
  done = 1;
  pthread_cond_signal(&event_condition);
  pthread_mutex_unlock(&event_lock);

  pthread_mutex_lock(&timer_lock);
  while (done) {
    pthread_cond_wait(&timer_condition, &timer_lock);
  }
  pthread_mutex_unlock(&timer_lock);
}

void* timer::thread_entry(void* args) {
  timer_args* t_args   = static_cast<timer_args*>(args);
  timer*      self     = t_args->self;
  void*       raw_args = t_args->args;
  delete t_args;
  return self->routine(raw_args);
}