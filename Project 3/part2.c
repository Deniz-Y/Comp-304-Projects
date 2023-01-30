/**
 * virtmem2together.c
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define TLB_SIZE 16 // cache for processes
#define PAGES 1024
#define PHYS_PAGES 256
#define PAGE_MASK 0xFFFFF /* 1111 1111 1111 1111 1111 */

#define PAGE_SIZE 1024
#define OFFSET_BITS 10
#define OFFSET_MASK 0x3FF /* 0011 1111 1111*/

#define MEMORY_SIZE PAGES *PAGE_SIZE

// Max number of characters per line of input file to read.
#define BUFFER_SIZE 10

struct tlbentry
{
  unsigned int logical;
  unsigned int physical;
};

// TLB is kept track of as a circular array, with the oldest element being overwritten once the TLB is full.
struct tlbentry tlb[TLB_SIZE];
// number of inserts into TLB that have been completed. Use as tlbindex % TLB_SIZE for the index of the next TLB line to use.
int tlbindex = 0;

// pagetable[logical_page] is the physical page number for logical page. Value is -1 if that logical page isn't yet in the table.
int pagetable[PAGES];

signed int main_memory[MEMORY_SIZE];

// Pointer to memory mapped backing file
signed char *backing;

// keep the recently used memory pages
int LRUTable[PHYS_PAGES];

int max(int a, int b)
{
  if (a > b)
    return a;
  return b;
}

/* Returns the physical address from TLB or -1 if not present. */
int search_tlb(unsigned int logical_page)
{
  /* TODO */
  int i;
  for (i = 0; i < TLB_SIZE; i++)
  {
    if (tlb[i].logical == logical_page)
    {
      return tlb[i].physical;
    }
  }
  return -1;
}
// shifts empty cells in tlb to the end of the array
int shift_tlb()
{
  /* TODO */
  int i;
  int j;
  for (i = 0; i < TLB_SIZE; i++)
  {
    if (tlb[i].logical == -1 && tlb[i].physical == -1)
    {
      for (j = i; j < TLB_SIZE - 1; j++)
      { // shift everything starting from this empty cell to left
        tlb[j].logical = tlb[j + 1].logical;
        tlb[j].physical = tlb[j + 1].physical;
      }
      tlb[TLB_SIZE - 1].logical = -1; // last cell is emptied
      tlb[TLB_SIZE - 1].physical = -1;
    }
  }
}

/* Adds the specified mapping to the TLB, replacing the oldest mapping (FIFO replacement). */
void add_to_tlb(unsigned int logical, unsigned int physical)
{
  tlb[tlbindex % TLB_SIZE].logical = logical;
  tlb[tlbindex % TLB_SIZE].physical = physical;
  tlbindex++;
}

int main(int argc, const char *argv[])
{
  int lru = 0;
  if (argc != 5)
  {
    fprintf(stderr, "Usage ./virtmem backingstore input -p 0  (fifo) or ./virtmem backingstore input -p 1 (lru) \n");
    exit(1);
  }
  if (!strcmp(argv[4], "0"))
  {
    lru = 0;
  }
  else if (!strcmp(argv[4], "1"))
  {
    lru = 1;
  }
  else
  {
    fprintf(stderr, "Usage ./virtmem backingstore input -p 0  (fifo) or ./virtmem backingstore input -p 1 (lru) \n");
    exit(1);
  }

  const char *backing_filename = argv[1];
  int backing_fd = open(backing_filename, O_RDONLY);
  backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0);

  const char *input_filename = argv[2];
  FILE *input_fp = fopen(input_filename, "r");
  if (input_fp == NULL)
  {
    printf("INPUT FILE IS NOT AVAILABLE\n");
    return 0;
  }
  // Fill page table entries with -1 for initially empty table.
  int i;
  for (i = 0; i < TLB_SIZE; i++)
  {
    tlb[i].logical = -1;
    tlb[i].physical = -1;
  }
  for (i = 0; i < PAGES; i++)
  {
    pagetable[i] = -1;
  }
  for (i = 0; i < PHYS_PAGES; i++)
  {
    LRUTable[i] = 0;
  }
  // Character buffer for reading lines of input file.
  char buffer[BUFFER_SIZE];

  // Data we need to keep track of to compute stats at end.
  int total_addresses = 0;
  int tlb_hits = 0;
  int page_faults = 0;

  // Number of the next unallocated physical page in main memory
  unsigned int free_page = 0;

  // use arguments to change between fifo and lru
  while (fgets(buffer, BUFFER_SIZE, input_fp) != NULL)
  {
    total_addresses++;
    int logical_address = atoi(buffer);

    /* TODO
    / Calculate the page offset and logical page number from logical_address */
    int offset = logical_address & OFFSET_MASK;                      // bitmask and get last 10 bits
    int logical_page = (logical_address & PAGE_MASK) >> OFFSET_BITS; // bitmask and get 20 bits, then shift and get only first 10
    ///////

    int physical_page = search_tlb(logical_page);
    // TLB hit
    if (physical_page != -1)
    {
      LRUTable[physical_page] = total_addresses;
      tlb_hits++;
      // TLB miss
    }
    else
    {
      physical_page = pagetable[logical_page];
      if (physical_page != -1)
      {
        LRUTable[physical_page] = total_addresses;
      }

      // Page fault
      if (physical_page == -1)
      {
        if (!lru)
        {
          /* TODO FIFO*/
          physical_page = free_page % PHYS_PAGES; // USE MODULO
          for (i = 0; i < PAGE_SIZE; i++)
          {
            main_memory[physical_page * PAGE_SIZE + i] = backing[logical_page * PAGE_SIZE + i]; // do memcopy
          }
          // Erase from page table
          for (i = 0; i < PAGES; i++)
          {
            if (pagetable[i] == physical_page)
            {
              pagetable[i] = -1;
            }
          }
          // Erase from TLB
          for (i = 0; i < TLB_SIZE; i++)
          {
            if (tlb[i].physical == physical_page)
            {
              tlb[i].logical = -1;
              tlb[i].physical = -1;
              tlbindex--;
              shift_tlb();
            }
          }
          free_page++;
        }
        else
        {
          /* TODO LRU*/
          int full = 0; // this is for checking if phys.memory is full.
          if (free_page >= PHYS_PAGES)
          {
            full = 1;
            int min = 1025; // no memory page can be used more than 1024 this is the max number of logical memory pages
            int torem = 0;
            for (i = 0; i < PHYS_PAGES; i++)
            {
              if (LRUTable[i] < min) // check the least recently used
              {
                min = LRUTable[i];
                torem = i; // find the memory that is used the least and oldest
              }
            }
            LRUTable[torem] = 0; // erase memory from LRU table
            free_page = torem;   // this memory snippet will be re loaded with new info
          }
          physical_page = free_page;
          for (i = 0; i < PAGE_SIZE; i++)
          {
            main_memory[physical_page * PAGE_SIZE + i] = backing[logical_page * PAGE_SIZE + i]; // do memcopy
          }
          // Erase old memory from Page table
          for (i = 0; i < PAGES; i++)
          {
            if (pagetable[i] == physical_page)
            {
              pagetable[i] = -1;
            }
          }
          // Erase old phys. memory from TLB
          for (i = 0; i < TLB_SIZE; i++)
          {
            if (tlb[i].physical == physical_page)
            {
              tlb[i].logical = -1;
              tlb[i].physical = -1;
              tlbindex--;
              shift_tlb();
            }
          }

          LRUTable[physical_page] = total_addresses; // total addresses is used as a time indicator
          if (!full)
          {
            free_page++;
          }
          else
          {
            free_page = PHYS_PAGES; // if phys memory is full keep in mind there are no free pages, you just re used an old page
          }
        }

        pagetable[logical_page] = physical_page;

        page_faults++;
      }

      add_to_tlb(logical_page, physical_page);
    }

    int physical_address = (physical_page << OFFSET_BITS) | offset;
    signed int value = main_memory[physical_page * PAGE_SIZE + offset];

    printf("Virtual address: %d Physical address: %d Value: %d \n", logical_address, physical_address, value);
  }

  printf("Number of Translated Addresses = %d\n", total_addresses);
  printf("Page Faults = %d\n", page_faults);
  printf("Page Fault Rate = %.3f\n", page_faults / (1. * total_addresses));
  printf("TLB Hits = %d\n", tlb_hits);
  printf("TLB Hit Rate = %.3f\n", tlb_hits / (1. * total_addresses));

  return 0;
}
