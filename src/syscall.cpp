#include "syscall.hpp"
#include "common.hpp"

#define __SYSCALL(nr, sym) extern int __##sym(process* caller, registers_set* regs);
#include "../syscalltbl.lst"
#undef __SYSCALL

#define __SYSCALL(nr, sym) #nr "-" #sym,
const std::vector<std::string> syscall_table = {
  #include "../syscalltbl.lst"
};
#undef  __SYSCALL

int __sys_null_syscall(process* caller, registers_set* regs) {
  // do nothing
  return 0;
}

// using macro to include from `syscalltbl.lst`
#define __SYSCALL(nr, sym) case nr: return __##sym(caller, regs);
int syscall(process* caller, uint32_t nr, registers_set* regs) {
  switch (nr) {
    #include "../syscalltbl.lst"
    default: return __sys_null_syscall(caller, regs);
  }
}