#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>

struct process_info
{
  uint64_t arrival_time;
  std::string path;
  int priority;
};

class configuration
{
private:
  static int                       free_pid;                        // static int avail_pid

public:
  static int                       quantum_time;

  static int                       simulation_time;                 // time_slot
  static int                       cores;                           // num_cpus
  static int                       ram_size;
  static int                       swap_size[4];
  const static int                 page_size = 256;                 // 256 bytes
  static int                       processes_count;                 // num_processes
  static std::vector<process_info> processes;

  static bool                      load(std::string path);          // static void read_config(const char * path)
  inline static int                get_pid() { return free_pid++; }
};

#endif