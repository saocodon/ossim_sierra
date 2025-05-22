#include "memory.hpp"
#include "common.hpp"
#include <filesystem>
#include <fstream>

process::process(std::string path, memory* ram, memory* swap0 = nullptr,
                 memory* swap1 = nullptr, memory* swap2 = nullptr,
                 memory* swap3 = nullptr, int dynamic_priority = 0) {
  this->path = path;
  good = true;
  if (!std::filesystem::exists(path))
  {
    good = false;
    return;
  }
  pid = configuration::get_pid();
  std::ifstream stream(path);
  stream >> priority >> program_size;
  this->priority = dynamic_priority;

  program = new instruction[program_size];
  for (int i = 0; i < program_size; i++)
  {
    std::string opcode;
    stream >> opcode;
    
    if (opcode == "calc")    program[i].opcode = instruction_opcode::CALC;
    if (opcode == "alloc")   program[i].opcode = instruction_opcode::ALLOC;
    if (opcode == "free")    program[i].opcode = instruction_opcode::FREE;
    if (opcode == "read")    program[i].opcode = instruction_opcode::READ;
    if (opcode == "write")   program[i].opcode = instruction_opcode::WRITE;
    if (opcode == "syscall") program[i].opcode = instruction_opcode::SYSCALL;

    switch (program[i].opcode)
    {
      case instruction_opcode::CALC:                                                                                        break;
      case instruction_opcode::ALLOC:   stream >> program[i].arg0 >> program[i].arg1;                                       break; 
      case instruction_opcode::FREE:    stream >> program[i].arg0;                                                          break;
      case instruction_opcode::READ:
      case instruction_opcode::WRITE:   stream >> program[i].arg0 >> program[i].arg1 >> program[i].arg2;                    break;
      case instruction_opcode::SYSCALL: stream >> program[i].arg0 >> program[i].arg1 >> program[i].arg2 >> program[i].arg3; break;
      default:                                                                                          break;
    }
  }

  this->pc = this->bp = 0;
  memory_system = new internal_memory(this, ram, swap0, swap1, swap2, swap3);
}