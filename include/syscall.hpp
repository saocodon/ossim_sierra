#ifndef SYSCALL_HPP
#define SYSCALL_HPP

#include <stdint.h>
#include "common.hpp"

// has been calculated in syscall.cpp
extern const std::vector<std::string> syscall_table;

int syscall(process* caller, uint32_t nr, registers_set* regs);
int __sys_null_syscall(process* caller, registers_set* regs);

#endif