#ifndef TIMER_HPP
#define TIMER_HPP

#include <pthread.h>
#include <stdint.h>
#include <list>

class timer {
private:
  bool                   started;                    // int timer_started
                                                     // int timer_stopped

  int                    fsh;
  pthread_cond_t         event_condition;            // pthread_cond_t event_cond
  pthread_mutex_t        event_lock;
  pthread_cond_t         timer_condition;            // pthread_cond_t timer_cond
  pthread_mutex_t        timer_lock;

  pthread_t              thread;                     // pthread_t _timer
  // global timer
  static uint64_t        time;                       // static uint64_t _time

public:
  int                    id;

  bool                   done;                       // flag that the cpu has done executed current instruction

  timer();

  void                   start(void* raw_args);      // void start_timer()
  void                   stop();                     // void stop_timer()
  void                   set();                      // struct timer_id_t* attach_event()
  void                   unset();                    // void detach_event(struct timer_id_t * event)
  void*                  routine(void* raw_args);    // static void * timer_routine(void * args)
  void                   tick();                     // void next_slot(struct timer_id_t * timer_id)
  inline uint64_t        get_time() { return time; } // uint64_t current_time()

  static void*           thread_entry(void* args);

  ~timer();
};

class timer_manager {
public:
  // make the list thread-safe (ensure initialization)
  static std::list<timer*>& get_list() {
    /*
    must be a list of pointers
    because it will be used under timer::timer()
    when a timer is still under construction
    copying it into this list makes a SEGFAULT.
    */
    static std::list<timer*> list;
    return list;
  }
};

#endif