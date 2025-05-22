#include "syscall.hpp"
#include "common.hpp"
#include "logger.hpp"
#include "scheduler.hpp"
#include "memory.hpp"
#include <sstream>

int __kill_process_tree(process* caller, registers_set* regs)
{
  // according to the assignment, we need to kill the process tree
  // by the target name set in the assembly program, not by path
  std::string target_name = "";
  // get the target name requested by the caller
  // read every single byte from the symbol requested by the caller
  memory_region symbol = caller->memory_system->symbol_table[regs->a1];
  for (int i = 0; i <= symbol.second - symbol.first; i++)
  {
    char c = 0;
    caller->memory_system->read(0, regs->a1, i, &c);
    if (c == -1) break;
    target_name += c;
  }

  std::stringstream ss;
  ss << " Process " << caller->pid << ": Killing processes with target name "
     << target_name << std::endl;
  logger::log(ss.str());

  // loop through the queue and pop the processes which ->path = target_name
  for (int i = 0; i < MAX_PRIORITY; i++)
  {
    scheduler::ready_queue[i].remove_if(
      [&target_name](process* p) {
        return p->path == target_name;
    });
  }

  return 0;
}