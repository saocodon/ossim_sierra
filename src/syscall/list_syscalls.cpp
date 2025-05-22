#include "syscall.hpp"
#include "common.hpp"
#include "logger.hpp"

int __list_syscalls(process* caller, registers_set* regs)
{
  for (int i = 0; i < syscall_table.size(); ++i)
    logger::log(syscall_table[i] + '\n');
  return 0;
}