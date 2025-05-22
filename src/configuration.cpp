#include "configuration.hpp"
#include <filesystem>
#include <fstream>

std::vector<process_info> configuration::processes       = {};
int                       configuration::processes_count = 0;
int                       configuration::free_pid        = 1;

int                       configuration::simulation_time = 0;
int                       configuration::cores           = 0;
int                       configuration::quantum_time    = 1;
int                       configuration::ram_size        = 1048576;
int                       configuration::swap_size[4]    = { 16777216, 0, 0, 0 };

bool configuration::load(std::string path)
{
  if (!std::filesystem::exists(path)) return false;

  std::ifstream config(path);
  config >> simulation_time >> cores >> processes_count;
#ifndef USE_DEFAULT_MEMORY_CONFIGURATION
  config >> ram_size >> swap_size[0] >> swap_size[1] >> swap_size[2] >> swap_size[3];
#endif
  for (int i = 0; i < processes_count; i++) {
    uint64_t arrival_time;
    int priority;
    std::string path;

    config >> arrival_time >> path >> priority;
    processes.push_back({arrival_time, path, priority}); 
  }
  return true;
}