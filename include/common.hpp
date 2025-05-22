#ifndef COMMON_HPP
#define COMMON_HPP

#include <string>
#include "configuration.hpp"

const int ADDRESS_SIZE = 20; // 20 bits for address
const int OFFSET_SIZE  = 10; // 10 bits for offset
const int FLPT_SIZE    = 5;  // 5 bits for address in the first level of page table
const int SLPT_SIZE    = 5;  // 5 bits for address in the second level of page table

enum class instruction_opcode
{
  CALC,
  ALLOC,
  FREE,
  READ,
  WRITE,
  SYSCALL
};

typedef std::pair<unsigned long, unsigned long> memory_region;
class internal_memory;
class memory;

struct instruction
{
  instruction_opcode opcode;
  uint32_t           arg0;
  uint32_t           arg1;
  uint32_t           arg2;
  uint32_t           arg3;
};

struct registers_set
{
  uint32_t a1;
  uint32_t a2;
  uint32_t a3;
  uint32_t a4;
  uint32_t a5;
  uint32_t a6;

  uint32_t syscall; // used to store the system call number
                    // if there are errors, this will contain error code.
                    // or even device interrupts
  int32_t  flags;
};

enum class memory_operation
{
  MAP,
  INC,
  SWP,
  READ,
  WRITE
};

class process
{
  // --------------------------------------------------------------
  // |                          pcb_t                             |
  // --------------------------------------------------------------
  friend class internal_memory;
  friend class scheduler;
  friend class cpu;
  friend class loader;

private:
  bool                good;
  uint32_t            pid;
  uint32_t            priority;
  std::string         path;
  instruction*        program;
  uint32_t            program_size;
  // at most 10 local variables can be declared
  uint32_t            symbol_table    [10]; 
  uint32_t            registers       [10];
  uint32_t            pc;
  // break pointer
  uint32_t            bp;
  // a map which maps level 1 table -> level 2 table in the page table
  memory_region       page_table      [1 << FLPT_SIZE]; 
public:
  internal_memory*    memory_system;

  process(std::string path, memory* ram, memory* swap0,
          memory* swap1, memory* swap2,
          memory* swap3, int dynamic_priority);

  // system calls
  friend int __memory_mapping   (process*, registers_set*);
  friend int __kill_process_tree(process*, registers_set*);
};

#endif
