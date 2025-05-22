#include "memory.hpp"
#include "internal_memory.hpp"
#include "logger.hpp"
#include "kernel_config.hpp"
#include <cmath>
#include <iomanip>

internal_memory::internal_memory(process* parent, memory* ram,
                                 memory* swap0 = nullptr, memory* swap1 = nullptr,
                                 memory* swap2 = nullptr, memory* swap3 = nullptr)
{
  this->ram  = ram;
  this->swap[0] = swap0;
  this->swap[1] = swap1;
  this->swap[2] = swap2;
  this->swap[3] = swap3;
  this->parent = parent;
  // initialize the page global directory
  pgd = new uint32_t[ram->frame_count];
  memset(pgd, 0, ram->frame_count * sizeof(uint32_t));
  // initialize the memory partitions
  partitions.push_back(memory_partition(0, 0, ram->max_size, 0));
}

memory_partition* internal_memory::get_partition(int index)
{
  for (auto iterator = partitions.begin(); iterator != partitions.end(); iterator++)
    if (iterator->id == index)
      return &*iterator;
  return nullptr;
}

memory_region internal_memory::get_free_memory_region(int partition_id, int size)
{
  memory_partition* p = get_partition(partition_id);
  int index = 0; // count index by hand
  for (auto iterator = p->free_regions.begin(); iterator != p->free_regions.end();)
  {
    if (iterator->second - iterator->first > size)
    {
      // split the region into two parts: one will be given, one will stay doing nothing.
      // shrink the region
      memory_region returning(iterator->first, iterator->first + size - 1);
      memory_region after_shrink_region(iterator->first + size, iterator->second);
      p->free_regions.erase(iterator);
      p->free_regions.insert(after_shrink_region);
      return returning;
    }
    else if (iterator->second - iterator->first == size)
    {
      memory_region returning(iterator->first, iterator->first + size - 1);
      iterator = p->free_regions.erase(iterator);
      return returning;
    }
    else
    {
      ++iterator;
      ++index;
    }
  }
  return null_region;
}

int internal_memory::alloc(int partition_id, int symbol_id, int size)
{
  int               pages_count      = ceil(size * 1.0 / ram->page_size); 
  int               size_aligned     = pages_count * ram->page_size;
  memory_region     free_area        = get_free_memory_region(partition_id, size_aligned);
  memory_partition* p                = get_partition(partition_id);
  uint32_t          starting_address = p->ptr;

  if (free_area != null_region)
  {
    memory_region free_area_mapped = map_to_ram(free_area.first, pages_count);
    if (free_area_mapped != null_region)
    {
      symbol_table[symbol_id] = free_area_mapped;
      log_alloc_and_free(true, size_aligned, pages_count, free_area_mapped.first,
                         partition_id);
      return free_area_mapped.first;
    }
  }
  
  // as a syscall
  if (extend_virtual_memory_area(partition_id, size_aligned))
  {
    log_alloc_and_free(true, size_aligned, pages_count, starting_address, partition_id);
    return starting_address;
  }
  return -1;
}

bool internal_memory::extend_virtual_memory_area(int partition_id, int new_area_size)
{
  int pages_count  = ceil(new_area_size * 1.0 / ram->page_size);
  int size_aligned = pages_count * ram->page_size;

  memory_partition* area_to_resize = get_partition(partition_id);
  memory_region*    new_chunk      = nullptr;

  // struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid,
  //                                              int size, int alignedsz)
  int new_ptr = area_to_resize->ptr + size_aligned;

  if (new_ptr <= area_to_resize->end)
  {
    new_chunk = new memory_region(area_to_resize->ptr, new_ptr - 1);
    area_to_resize->ptr = new_ptr;
  } else {
    memory_partition* next_area = get_partition(partition_id + 1);
    if (next_area->start > new_ptr)
    {
      area_to_resize->end = new_ptr;
      new_chunk           = new memory_region(area_to_resize->ptr, new_ptr - 1);
      area_to_resize->ptr = new_ptr;
    }
  }

  if (new_chunk == nullptr)
    return false;

  // Map this fraction of virtual memory to RAM to be able to use
  return (map_to_ram(new_chunk->first, pages_count) == null_region);
}

memory_region internal_memory::map_to_ram(int starting_address, int pages_count)
{
  // get [pages_count] free frame page numbers
  // i.e. performing MEMPHY_get_freefp(struct memphy_struct* mp, int *fpn) [pages_count] times
  // i.e. performing alloc_pages_range(struct pcb_t *caller, int req_pgnum,
  //                                   struct framephy_struct **frm_lst)
  std::vector<int> fpns;
  for (int i = 0; i < pages_count; i++)
    fpns.push_back(ram->get_free_frame());

  if (fpns.size() < pages_count)
  {
    logger::log(" Process " + std::to_string(parent->pid) + ": Not enough memory.\n");
    return null_region;
  }

  // int vmap_page_range(struct pcb_t *caller,
  //                     int addr,
  //                     int pgnum,
  //                     struct framephy_struct *frames,
  //                     struct vm_rg_struct *ret_rg)
  int base_pgn = EXTRACT_PGN(starting_address);
  for (int th_page = 0; th_page < pages_count; th_page++)
  {
    int fpn = fpns[th_page];
    // get current page table entry
    uint32_t* pte = &pgd[base_pgn + th_page];
    set_fpn_to_pte(pte, fpn);

    // mark that the page is in use
    pages_in_use.push_back(base_pgn + th_page);
  }

  return memory_region(starting_address, starting_address + pages_count 
                       * ram->page_size - 1);
}

void internal_memory::set_fpn_to_pte(uint32_t* pte, int fpn)
{
  *pte |= (1 << PTE_PRESENT_BIT);
  *pte &= ~(1 << PTE_ONSWAP_BIT);
  *pte = set_argument(*pte, fpn, set_bits(PTE_FPN_LOW_BIT, PTE_FPN_HIGH_BIT),
                      PTE_FPN_LOW_BIT);
}

void internal_memory::set_swap_to_pte(uint32_t* pte, int device, int offset) {
  *pte |= (1 << PTE_PRESENT_BIT);
  *pte |= (1 << PTE_ONSWAP_BIT);
  *pte = set_argument(*pte, device, set_bits(PTE_SWAP_DEVICE_LOW_BIT,
                      PTE_SWAP_DEVICE_HIGH_BIT), PTE_SWAP_DEVICE_LOW_BIT);
  *pte = set_argument(*pte, offset, set_bits(PTE_SWAP_OFFSET_HIGH_BIT,
                      PTE_SWAP_OFFSET_LOW_BIT), PTE_SWAP_OFFSET_LOW_BIT);
}

bool internal_memory::free(int partition_id, int symbol_id)
{
  if (partition_id < 0 || partition_id >= SYMBOL_TABLE_SIZE)
    return false;

  memory_partition* p = get_partition(partition_id);
  // mark the region as free
  p->free_regions.insert(symbol_table[symbol_id]);
  // unmap the region from RAM
  memory_region symbol = symbol_table[symbol_id];
  int pages_count      = ceil((symbol.second - symbol.first + 1) * 1.0 / ram->page_size);
  int base_pgn         = EXTRACT_PGN(symbol.first);

  for (int th_page = 0; th_page < pages_count; th_page++)
  {
    int pgn = base_pgn + th_page;
    // get current page table entry
    uint32_t *pte = &pgd[pgn];

    int fpn = EXTRACT_FPN(*pte);
    // return the frame to the free list
    ram->used_frames.remove(fpn);
    ram->free_frames.push_back(fpn);

    // mark the page as unavailable
    *pte &= ~(1 << PTE_PRESENT_BIT);
    *pte &= ~(1 << PTE_ONSWAP_BIT);
  }
  // remove the pages from the list of pages in use
  for (auto pgn_ptr = pages_in_use.begin(); pgn_ptr != pages_in_use.end();)
  {
    if (!(*pgn_ptr >= base_pgn && *pgn_ptr - base_pgn < pages_count))
      pgn_ptr = pages_in_use.erase(pgn_ptr);
    else
      pgn_ptr++;
  }
  // maybe I can even combine the free regions
  // but let's leave it for extend_virtual_memory_area()
  // to make this method short enough

  log_alloc_and_free(false, symbol.second - symbol.first + 1, pages_count, symbol.first, partition_id);

  return true;
}

int internal_memory::load_page_to_ram(int pgn)
{
  if (pgn < 0)
    return -1;

  // get frame number of requested page by using the PTE
  uint32_t *pte_of_requested_page = &pgd[pgn];
  // if the page is not on RAM
  if (!(*pte_of_requested_page & (1 << PTE_PRESENT_BIT)))
  {
    // if ram still have free frames -> no need to evict LRU pages
    // try getting one
    int new_frame = ram->get_free_frame();
    if (new_frame == -1)
    {
      // no more free frames -> evict LRU page
      // choose the LRU page to swap to disk
      int pgn_to_swap = pages_in_use.front();
      pages_in_use.pop_front();
      // int vicpte
      uint32_t* pte_to_swap = &pgd[pgn_to_swap];
      // get its physical frame
      int physical_frame = EXTRACT_FPN(*pte_to_swap);

      // get a free frame in swap (vicfpn)
      int swap_frame, swap_id = -1;
      for (memory* device : swap)
      {
        swap_id++;
        swap_frame = device->get_free_frame(); 
        if (swap_frame != -1)
          break;
      }
      // if there are no more frames -> fail
      if (swap_frame == -1)
        return -1;

      // back up swap memory at specified page
      char swap_temp[ram->page_size]; 
      memcpy(swap_temp,
             ram->storage + physical_frame * ram->page_size,
             ram->page_size);

      // copy content from backed up buffer to swap
      memcpy(swap[swap_id]->storage + swap_frame     * swap[swap_id]->page_size,
             ram->storage           + physical_frame * ram->page_size,
             ram->page_size);

      // copy content from swap at requested page to ram
      memcpy(ram->storage + physical_frame * ram->page_size,
             swap_temp,
             ram->page_size);

      set_swap_to_pte(pte_to_swap, swap_id, swap_frame);
      pages_in_use.remove(pgn_to_swap);

      set_fpn_to_pte(pte_of_requested_page, physical_frame);
      pages_in_use.push_back(pgn);

      return physical_frame;
    }
    else
    {
      // if there are free frames, just copy the page to RAM
      // and set the PTE to point to it
      uint32_t requested_swap_offset = EXTRACT_SWAP_OFFSET(*pte_of_requested_page);
      uint32_t requested_swap_device = EXTRACT_SWAP_DEVICE(*pte_of_requested_page);

      set_fpn_to_pte(pte_of_requested_page, new_frame);
      pages_in_use.push_back(pgn);

      // copy content from swap at requested page to ram
      // int __swap_cp_page(struct memphy_struct *mpsrc, int srcfpn,
      //                    struct memphy_struct *mpdst, int dstfpn)
      memcpy(ram->storage + requested_swap_offset,
             swap[requested_swap_device]->storage + new_frame * ram->page_size,
             ram->page_size);

      return new_frame;
    }
  }

  // if the page is already on RAM, we don't have to get it from swap
  return EXTRACT_FPN(*pte_of_requested_page);
}

bool internal_memory::read_from_physical_address(int address, char* returning)
{
  int pgn    = EXTRACT_PGN           (address);
  int offset = EXTRACT_ADDRESS_OFFSET(address);
  // get the page to RAM from swap if needed
  int fpn = load_page_to_ram(pgn);

  // invalid page access
  if (fpn == -1) return false;

  int physical_address = fpn * ram->page_size + offset;
  *returning = ram->storage[physical_address];

  return true;
}

bool internal_memory::write_to_physical_address(int address, char data)
{
  int pgn    = EXTRACT_PGN           (address);
  int offset = EXTRACT_ADDRESS_OFFSET(address);
  // get the page to RAM from swap if needed
  int fpn    = load_page_to_ram(pgn);

  // invalid page access
  if (fpn == -1) return false;

  int physical_address = fpn * ram->page_size + offset;
  ram->storage[physical_address] = data;

  return true;
}

bool internal_memory::read(int partition_id, int symbol_id, int offset, char* returning)
{
  if (symbol_id < 0 || symbol_id >= SYMBOL_TABLE_SIZE)
    return false;

  memory_region symbol = symbol_table[symbol_id];
  memory_partition* partition = get_partition(partition_id);

  if (partition == nullptr) return false;

  bool success = read_from_physical_address(symbol.first + offset, returning);
  std::stringstream ss;
  ss << " Process " << parent->pid << ": Read from region " << std::dec << symbol_id
     << ", offset " << offset
     << ": " << (int)*returning << std::endl;
  logger::log(ss.str());

  return success;
}

bool internal_memory::write(int partition_id, int symbol_id, int offset, char value)
{
  if (symbol_id < 0 || symbol_id >= SYMBOL_TABLE_SIZE)
    return -1;

  memory_region symbol = symbol_table[symbol_id];
  memory_partition* partition = get_partition(partition_id);

  if (partition == nullptr) return -1;

  std::stringstream ss;
  ss << " Process " << parent->pid << ": Write to region " << std::dec << symbol_id
     << ", offset " << offset
     << " value "   << (int)value << std::endl;
  logger::log(ss.str());

  return write_to_physical_address(symbol.first + offset, value);
}

void internal_memory::log_alloc_and_free(bool is_alloc, int size, int pages,
                                         uint32_t address, int region)
{
  std::stringstream ss;
  ss << " Process " << parent->pid << ": " << (is_alloc ? "Allocated " : "Freed ")
     << std::dec << size << " bytes ("
     << std::dec << pages << " pages) at address 0x" << std::hex << address
     << " of region " << std::dec << region << std::endl;
  logger::log(ss.str());
#ifdef PRINT_PGD
  print_page_global_directory();
#endif
}

void internal_memory::print_page_global_directory() {
  std::stringstream ss;
  ss << " Process " << parent->pid << ": \n";
  ss << " +--------- PGD ---------+\n";
  for (int entry = 0; entry < ram->frame_count; entry++) {
    // skip pages that are not present
    if ((pgd[entry] & (1 << PTE_PRESENT_BIT)) == 0) continue;
    // one PTE is 4 bytes (32 bits)
    ss << " | " << std::dec << std::setfill('0') << std::setw(8)
              << entry * 4 << ":  0x" << std::hex << 
              pgd[entry] << " |\n";
  }
  ss << " +-----------------------+\n";
  for (int entry = 0; entry < ram->frame_count; entry++) {
    if ((pgd[entry] & (1 << PTE_PRESENT_BIT)) == 0) continue;
    ss << " | Page " << std::setfill(' ') <<  std::setw(3) << std::dec
              << entry << " -> Frame " << std::setw(3) << std::dec
              << EXTRACT_FPN(pgd[entry]) << " |\n";
  }
  ss << " +-----------------------+\n";
  logger::log(ss.str());
}