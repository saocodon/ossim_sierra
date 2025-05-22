#ifndef INTERNAL_MEMORY_HPP
#define INTERNAL_MEMORY_HPP

int set_bits(int l, int r)
{
  // initialize [r+1] bits to 1
  int result = (1 << (r + 1)) - 1;
  // initialize [1...l] bitmask
  int off_mask = (1 << l) - 1;
  return result & ~off_mask;
}

int set_argument(int current_argument_value, int new_argument_value,
                 int argument_mask, int argument_offset)
{
  int entry_with_argument_cleared = (current_argument_value & ~argument_mask);
  int entry_with_only_argument    = ((new_argument_value << argument_offset)
                                     & argument_mask);
  int final_entry                 = (entry_with_argument_cleared | entry_with_only_argument);
  return final_entry;
}

// ===========================================================================
// ||                   PAGE TABLE ENTRY (PTE) STRUCTURE                    ||
// ==========================================================================
// if this entry is present in the table
const int PTE_PRESENT_BIT          = 31; 
// if this page is on RAM or on swap
const int PTE_ONSWAP_BIT           = 30; 

// <------------------------| if the page is on RAM | ----------------------->
const int PTE_FPN_HIGH_BIT         = 12;
const int PTE_FPN_LOW_BIT          =  0;

// bits 0-12 is used for FPN
#define EXTRACT_FPN(entry)             (entry & set_bits(PTE_FPN_LOW_BIT, PTE_FPN_HIGH_BIT) >> PTE_FPN_LOW_BIT)

// <------------------------| if the page is on swap | ---------------------->
const int PTE_SWAP_OFFSET_HIGH_BIT = 25;
const int PTE_SWAP_OFFSET_LOW_BIT  =  5;
const int PTE_SWAP_DEVICE_HIGH_BIT =  4;
const int PTE_SWAP_DEVICE_LOW_BIT  =  0;

#define EXTRACT_SWAP_OFFSET(entry)    (entry & set_bits(PTE_SWAP_OFFSET_LOW_BIT, PTE_SWAP_OFFSET_HIGH_BIT) >> PTE_SWAP_OFFSET_LOW_BIT)
#define EXTRACT_SWAP_DEVICE(entry)    (entry & set_bits(PTE_SWAP_DEVICE_LOW_BIT, PTE_SWAP_DEVICE_HIGH_BIT) >> PTE_SWAP_DEVICE_LOW_BIT)

// ===========================================================================
// ||                       ADDRESS STRUCTURE                               ||
// ==========================================================================
const int ADDRESS_PGN_HIGH_BIT    = 21;
const int ADDRESS_PGN_LOW_BIT     =  8;
const int ADDRESS_OFFSET_HIGH_BIT =  7;
const int ADDRESS_OFFSET_LOW_BIT  =  0;

// bits 8-21 is used for PGN
#define EXTRACT_PGN(entry)            ((entry & set_bits(ADDRESS_PGN_LOW_BIT   , ADDRESS_PGN_HIGH_BIT   )) >> ADDRESS_PGN_LOW_BIT   )
// bits 0-7  is used for offset
#define EXTRACT_ADDRESS_OFFSET(entry) ((entry & set_bits(ADDRESS_OFFSET_LOW_BIT, ADDRESS_OFFSET_HIGH_BIT)) >> ADDRESS_OFFSET_LOW_BIT)

#endif