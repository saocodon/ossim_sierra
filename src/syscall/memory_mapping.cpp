#include "syscall.hpp"
#include "common.hpp"
#include "memory.hpp"
#include <algorithm>

int __memory_mapping(process* caller, registers_set* regs)
{
  memory_operation operation = static_cast<memory_operation>(regs->a1);
  internal_memory* mem       = caller->memory_system;
  switch (operation) {
    case memory_operation::MAP:   /* reserved */ return 0;                                    break; 
    case memory_operation::INC:   return mem->extend_virtual_memory_area(regs->a2, regs->a3); break;
    case memory_operation::SWP: {
      int swap_count = sizeof(mem->swap) / sizeof(memory*);
      for (int i = 0; i < swap_count; i++) {
        int swap_frame = regs->a3 / mem->swap[i]->page_size;
        auto result = std::find(mem->swap[i]->used_frames.begin(),
                                mem->swap[i]->used_frames.end(), swap_frame);
        if (result == mem->swap[i]->used_frames.end()) {
          memcpy(mem->ram + regs->a2 * mem->ram->page_size,
                mem->swap[0] + regs->a3 * mem->swap[0]->page_size,
                mem->ram->page_size);
          return 0;
        }
      }
      break;
    }
    case memory_operation::READ:  regs->a3 = mem->ram->storage[regs->a2];    return regs->a3; break;
    case memory_operation::WRITE: mem->ram->storage[regs->a2] = regs->a3;           return 0; break;
    default:
      std::cout << caller->path << " [PID: " << caller->pid << "] __memory_mapping: Invalid operation code: " << regs->a1 << std::endl;
      return -1;
      break;
  }
  return -1;
}