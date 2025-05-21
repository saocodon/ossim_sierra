#include "syscall.hpp"
#include "common.hpp"
#include "memory.hpp"

int __memory_mapping(process* caller, registers_set* regs) {
  memory_operation operation = static_cast<memory_operation>(regs->a1);
  switch (operation) {
    case memory_operation::MAP:   /* reserved */ return 0;                                                                break;
    case memory_operation::INC:   return caller->memory_system->extend_virtual_memory_area(regs->a2, regs->a3);           break; 
    case memory_operation::SWP:   memcpy(caller->memory_system->ram + regs->a2 * caller->memory_system->ram->page_size,
                                         caller->memory_system->swap + regs->a3 * caller->memory_system->swap->page_size,
                                         caller->memory_system->ram->page_size);                                return 0; break;
    case memory_operation::READ:  regs->a3 = caller->memory_system->ram->storage[regs->a2];              return regs->a3; break;
    case memory_operation::WRITE: caller->memory_system->ram->storage[regs->a2] = regs->a3;                     return 0; break;
    default:
      std::cout << caller->path << " [PID: " << caller->pid << "] __memory_mapping: Invalid operation code: " << regs->a1 << std::endl;
      return -1;
      break;
  }
}