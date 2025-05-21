#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <stdint.h>
#include <string.h>
#include <iostream>

#include <list>
#include <set>
#include <vector>
#include <queue>

#include "common.hpp"

const int SYMBOL_TABLE_SIZE = 30;

const memory_region null_region = memory_region(0, 0);

class memory_partition {
  // --------------------------------------------------------------
  // |                      vm_area_struct                        |
  // --------------------------------------------------------------
public:
  unsigned long                   id;                  // unsigned long vm_id
  unsigned long                   start;               // unsigned long vm_start
  unsigned long                   end;                 // unsigned long vm_end
                                                       // struct mm_struct *vm_mm
                                                       // struct vm_area_struct *vm_next
  unsigned long                   ptr;                 // unsigned long sbrk
  std::set<memory_region>         free_regions;        // struct vm_rg_struct *vm_freerg_list

  memory_partition(unsigned long id, unsigned long start, unsigned long end, unsigned long ptr) {
    this->id = id;
    this->start = start;
    this->end = end;
    this->ptr = ptr;
    // initialize whole partition as a free block
    free_regions.insert(memory_region(start, end));
  }
};

class memory {
  // --------------------------------------------------------------
  // |                   memphy_struct                            |
  // --------------------------------------------------------------
  friend class internal_memory;
private:
  // false is unused
  std::list<int> free_frames; // struct framephy_struct *free_fp_list
  std::list<int> used_frames; // struct framephy_struct *used_fp_list
  int       max_size;         // int maxsz
  int       page_size;
  int       frame_count;
public:
  char*     storage;          // BYTE* storage
                              // int rdmflg
                              // int cursor

  // int init_memphy(struct memphy_struct *mp, int max_size, int randomflg)
  // int MEMPHY_format(struct memphy_struct *mp, int pagesz)
  memory(int max_size, int page_size);
  ~memory() { 
    delete[] storage;
  }
  // int alloc_pages_range(struct pcb_t *caller, int req_pgnum, struct framephy_struct **frm_lst)
  int get_free_frame();

  // system calls
  friend int __memory_mapping(process*, registers_set*);
};

class internal_memory {
  // --------------------------------------------------------------
  // |                      mm_struct                             |
  // --------------------------------------------------------------
private:
  uint32_t*                   pgd;                                // page global directory (as a map table)
  std::list<int>              pages_in_use;                       // struct pgn_t *fifo_pgn
                                                                  // (used to track page replacement activities)

  process*                    parent;

  // int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
  bool                        read_from_physical_address(int address, char* returning);
  // int pg_setval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
  bool                        write_to_physical_address (int address, char data);

public:
  // this will be used for referencing RAM and swap
  memory*                     ram;
  memory*                     swap;

  memory_region               symbol_table[SYMBOL_TABLE_SIZE];    // struct vm_rg_struct symrgtbl[PAGING_MAX_SYMTBL_SIZE]
  std::list<memory_partition> partitions;                         // struct vm_area_struct *mmap

  internal_memory(memory* ram, memory* swap, process* parent);

  // struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
  memory_partition*           get_partition(int index);
  // int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
  memory_region               get_free_memory_region(int partition_id, int size);
  // int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
  int                         alloc(int partition_id, int symbol_id, int size);
  // int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
  bool                        extend_virtual_memory_area(int partition_id, int new_size);
  // int vm_map_ram(struct pcb_t *caller, int astart, int aend, int mapstart, int incpgnum, struct vm_rg_struct *ret_rg)
  memory_region               map_to_ram(int starting_address, int pages_count);

  // int pte_set_swap(uint32_t *pte, int swptyp, int swpoff)
  // is used to mark a page is in swap
  void                        set_swap_to_pte(uint32_t* pte, int device, int offset);
  // int pte_set_fpn(uint32_t *pte, int fpn)
  // is used to mark a page is in RAM
  void                        set_fpn_to_pte(uint32_t* pte, int fpn);

  // int __free(struct pcb_t *caller, int vmaid, int rgid)
  bool                        free(int partition_id, int symbol_id);
  
  // int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
  bool                        read(int partition_id, int symbol_id, int offset, char* returning);
  // int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
  bool                        write(int partition_id, int symbol_id, int offset, char value);

  // int find_victim_page(struct mm_struct *mm, int *retpgn)
  // int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
  int                         load_page_to_ram(int pgn);

  void                        log_alloc_and_free(bool is_alloc, int size, int pages, uint32_t address, int region);
  // int print_pgtbl(struct pcb_t *caller, uint32_t start, uint32_t end)
  void                        print_page_global_directory();
};

#endif
