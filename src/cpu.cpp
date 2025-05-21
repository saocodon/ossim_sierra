#include <sstream>
#include "cpu.hpp"
#include "memory.hpp"
#include "syscall.hpp"
#include "logger.hpp"
#include "loader.hpp"
#include <iostream>

cpu::cpu(int id, int quantum_time) {
  this->id           = id;
  this->quantum_time = quantum_time;
  done               = false;
  t.id = id;
  t.set();
  t.start(NULL);
  pthread_create(&thread, NULL, thread_entry, this);
}

void* cpu::routine() {
  process* proc = nullptr;
  int time_left = 0;
  while (true) {
    if (proc == nullptr) {
      proc = scheduler::get();
      if (proc == nullptr) {
        if (done || !loader::good) {
          logger::log(" CPU " + std::to_string(id) + ": stopped.\n");
          break;
        }
        // do nothing
        t.tick();
        continue;
      }
      logger::log(" CPU " + std::to_string(id) + ": Process " + std::to_string(proc->pid) + " is being run.\n");
      time_left = quantum_time; 
    }
    if (proc->pc == proc->program_size) {
      // process is finished
      logger::log(" CPU " + std::to_string(id) + ": Finished running process " + std::to_string(proc->pid) + ".\n");
      delete proc;
      proc = nullptr;

      // Check if there is any job, or set done = true.
      if (scheduler::empty())
        done = true;
      continue;
    }
    
    execute_instruction(proc);
    time_left--;
    t.tick();

    if (time_left == 0 && proc != nullptr) {
      // process done in current time slot
      std::stringstream ss;
      logger::log(" CPU " + std::to_string(id) + ": Time quantum expired for process " + std::to_string(proc->pid) + ".\n");
      scheduler::push(proc);
      proc = nullptr;
    }
  }
  t.unset();
  pthread_exit(NULL);
}

int cpu::execute_instruction(process* proc) {
  if (proc->pc >= proc->program_size) return -100;
  
  instruction ins = proc->program[proc->pc];
  proc->pc++;
  
  int result;

  switch (ins.opcode) {
    case instruction_opcode::CALC:
      result = calc(proc);
      logger::log(" CPU " + std::to_string(id) + ": CALC instruction executed on process " + std::to_string(proc->pid) + ".\n");
      break; 
    case instruction_opcode::ALLOC:
      result = proc->memory_system->alloc(0, ins.arg1, ins.arg0);
      if (result >= 0)
        proc->registers[ins.arg1] = result;
      break;
    case instruction_opcode::FREE: 
      result = proc->memory_system->free(0, ins.arg0);
      break;
    case instruction_opcode::READ:
      char byte;
      result = proc->memory_system->read(0, ins.arg0, ins.arg1, &byte);
      proc->registers[ins.arg2] = byte;
      break;
    case instruction_opcode::WRITE:
      result = proc->memory_system->write(0, ins.arg1, ins.arg2, ins.arg0);
      break;
    case instruction_opcode::SYSCALL:
      registers_set regs;
      regs.syscall = ins.arg0;
      regs.a1      = ins.arg1;
      regs.a2      = ins.arg2;
      regs.a3      = ins.arg3;
      result       = syscall(proc, regs.syscall, &regs);
      break;
    default:    result = 1; break;
  }
  return result;
}

void* cpu::thread_entry(void* args) {
  return static_cast<cpu*>(args)->routine();
}

void cpu::halt() {
  logger::log("Shutting down timer " + std::to_string(id) + "...\n");
  t.unset();
  t.stop();
  // do not need pthread_join here, as it is done in cpu::run()
  // thread of cpu has ended using pthread_exit in the routine
}
