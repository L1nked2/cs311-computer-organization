
/* main.c

Page Table Entry (PTE) format:

Width: 32 bits

Bit 31 ~ 12     : 20-bit physical page number for the 2nd-level page table node or the actual physical page.
Bit 1           : Dirty bit
Bit 0           : Valid bit

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h" /* DO NOT DELETE */

/* addr_t type is the 32-bit address type defined in util.h */

#define VALID(ADDR) ADDR & 0x01
#define DIRTY(ADDR) ((ADDR & 0x03) >> 1)
#define PTN(ADDR) ((ADDR & 0xfffff000) >> 12)
#define L1PTN(ADDR) ((ADDR & 0xffc00000) >> 22 << 2)
#define L2PTN(ADDR) ((ADDR & 0x003ff000) >> 12 << 2)
#define TRUE 1
#define FALSE 0
#define VPN_DIGITS 20
#define TOTAL_DIGITS 32
// turn on debugging
#define DEBUG_A 0
#define DEBUG_B 0
int wa, wb, wc = 0;

/* struct for tlb entry */
typedef struct tlb_entry
{
  char valid;
  char dirty;
  uint32_t tag;
  addr_t vpn;
  addr_t ppn;
  uint32_t lru;
}tlb_entry_t;

/* simulation variables */
// tlb base pointer, tlb[SET][WAY]
tlb_entry_t **tlb;
// number of entries, sets and ways
int num_entries, num_sets, num_ways;
// digits of set and tag
int set_digits, tag_digits;
// statistics
int total_mem_access = 0;
int total_read_access = 0;
int total_write_access = 0;
int total_tlb_access = 0;
int total_tlb_hits = 0;
int total_tlb_misses = 0;
int total_page_walks = 0;
int total_page_faults = 0;

/* helper functions */
int get_index_digits(int num_entries)
{
  int digits = 0;
  while(num_entries >>= 1) {
    ++digits;
  }
  return digits;
}
int get_set(addr_t vpn)
{
  vpn <<= tag_digits;
  vpn >>= tag_digits;
  vpn >>= TOTAL_DIGITS - VPN_DIGITS;
  return vpn;
}
int get_tag(addr_t vpn)
{
  vpn >>= TOTAL_DIGITS - tag_digits;
  return vpn;
}

/* dump function for -x option */
void xdump(tlb_entry_t **tlb, int n_entries, int assoc)
{
  // dump tlb
  printf("TLB Content:\n");
  printf("-------------------------------------\n");
  for (int i = 0; i < assoc; i++)
  {
    if (i == 0)
    {
      printf("    ");
    }
    printf("      WAY[%d]", i);
  }
  printf("\n");
  for (int i = 0; i < n_entries; i++)
  {
    printf("SET[%d]:   ", i);
    for (int j = 0; j < assoc; j++)
    {
      printf(" (v=%d tag=0x%05x ppn=0x%05x d=%d) |", 
              tlb[i][j].valid, tlb[i][j].tag, tlb[i][j].ppn, tlb[i][j].dirty);
    }
    printf("\n");
  }

  // dump page table
  dump_page_table_area();
  return;
}

/* check tlb and returns tlb entry if hit or NULL if miss */
tlb_entry_t* check_tlb(tlb_entry_t **tlb, addr_t vpn)
{
  if(DEBUG_A)printf("%d: tlb accessed on vpn, 0x%0x\n", total_mem_access, vpn);
  total_tlb_access++;
  int set = get_set(vpn);
  int tag = get_tag(vpn);
  for(int i=0; i<num_ways; i++) {
    if(tlb[set][i].valid && tlb[set][i].tag == tag) {
      tlb[set][i].lru = total_mem_access;
      return &tlb[set][i];
    }
  }
  return NULL;
}

/* update tlb with vpn-ppn tuple */
void update_tlb(tlb_entry_t **tlb, addr_t vpn, addr_t ppn, char is_write)
{
  if(DEBUG_A)printf("%d: tlb updated with 0x%0x, 0x%0x, %d\n", total_mem_access, vpn, ppn, is_write);
  int set = get_set(vpn);
  int tag = get_tag(vpn);
  // check already exists. If so, update lru and return
  for(int i=0; i<num_ways; i++) {
    if(DEBUG_A)printf("checking %d, %d, %d, 0x%0x, 0x%0x,\n", set, i, tlb[set][i].valid, tlb[set][i].tag, tag);
    if(tlb[set][i].valid && tlb[set][i].tag == tag) {
      tlb[set][i].lru = total_mem_access;
      tlb[set][i].dirty = is_write;
      return;
    }
  }
  // tlb not exists, update with new block
  for(int i=0; i<num_ways; i++) {
    if(!tlb[set][i].valid) {
      if(DEBUG_B)printf("new entry on %d, %d, 0x%0x, 0x%0x,\n", set, i, ppn, tag);
      tlb[set][i].valid = TRUE;
      tlb[set][i].ppn = ppn;
      tlb[set][i].tag = tag;
      tlb[set][i].vpn = vpn;
      tlb[set][i].lru = total_mem_access;
      // for write, mark dirty to PTE
      addr_t pt_l2_base_addr, physical_page_addr;
      if(is_write && !tlb[set][i].dirty)
      {
        total_page_walks++;if(DEBUG_B)wa++;
        
        pt_l2_base_addr = mem_read_word32(page_table_base_addr() + L1PTN(vpn));
        physical_page_addr = mem_read_word32(pt_l2_base_addr + L2PTN(vpn));
        mem_write_word32(pt_l2_base_addr + L2PTN(vpn), physical_page_addr | is_write<<1 | 1);
      }
      tlb[set][i].dirty = is_write | DIRTY(physical_page_addr);
      return;
    }
  }
  // evict by lru if not available
  int min_lru = total_mem_access;
  int idx = 0;
  for(int i=0; i<num_ways; i++) {
    if(tlb[set][i].lru < min_lru) {
      min_lru = tlb[set][i].lru;
      idx = i;
    }
  }
  if(DEBUG_B)printf("%d:new entry(eviction) on %d, %d, 0x%0x, 0x%0x, lru: %d\n", total_mem_access, set, idx, ppn, tag, min_lru);
  tlb[set][idx].valid = TRUE;
  tlb[set][idx].ppn = ppn;
  tlb[set][idx].tag = tag;
  tlb[set][idx].vpn = vpn;
  tlb[set][idx].lru = total_mem_access;
  // for write, mark dirty to PTE
  addr_t pt_l2_base_addr, physical_page_addr;
  if(is_write && !tlb[set][idx].dirty)
  {
    total_page_walks++;if(DEBUG_B)wb++;
    pt_l2_base_addr = mem_read_word32(page_table_base_addr() + L1PTN(vpn));
    physical_page_addr = mem_read_word32(pt_l2_base_addr + L2PTN(vpn));
    mem_write_word32(pt_l2_base_addr + L2PTN(vpn), physical_page_addr | is_write<<1 | 1);
  }
  tlb[set][idx].dirty = is_write | DIRTY(physical_page_addr);

  return;
}

// page fault handler
addr_t handle_page_fault(addr_t pt_l1_base_addr, addr_t target_addr, char is_write)
{
  if(DEBUG_A)printf("%d: entered page fault handler with 0x%0x, 0x%0x, %d\n", total_mem_access, pt_l1_base_addr, target_addr, is_write);
  addr_t pt_l2_base_addr, physical_page_addr;
  total_page_faults++;
  // check L1 page
  pt_l2_base_addr = mem_read_word32(pt_l1_base_addr + L1PTN(target_addr));
  if(!VALID(pt_l2_base_addr))
  {
    // handle page fault(L1)
    mem_write_word32(pt_l1_base_addr + L1PTN(target_addr), get_new_page_table_node() | 1);
    pt_l2_base_addr = mem_read_word32(pt_l1_base_addr + L1PTN(target_addr));
  }
  // now L1 page exists
  // check L2 page
  physical_page_addr = mem_read_word32(pt_l2_base_addr + L2PTN(target_addr));
  if(!VALID(physical_page_addr))
  {
    // handle page fault(physical)
    // mark as dirty if it is write
    mem_write_word32(pt_l2_base_addr + L2PTN(target_addr), get_new_physical_page() | is_write<<1 | 1);
    physical_page_addr = mem_read_word32(pt_l2_base_addr + L2PTN(target_addr));
  }
  return physical_page_addr;
}

/* page table walker, returns physical page address if successful 
   or return 0 if page fault ocurrs
*/
addr_t walk_page_table(addr_t pt_l1_base_addr, addr_t target_addr, char *is_write)
{
  if(DEBUG_A)printf("%d: entered page table walk with 0x%0x, 0x%0x, %d\n", total_mem_access, pt_l1_base_addr, target_addr, *is_write);
  addr_t pt_l2_base_addr, physical_page_addr;
  total_page_walks++;if(DEBUG_B)wc++;
  // try to walk L1
  // check L1 page entry
  if(!VALID(mem_read_word32(pt_l1_base_addr + L1PTN(target_addr))))
  {
    // handle page fault
    *is_write = TRUE;
    handle_page_fault(pt_l1_base_addr, target_addr, *is_write);
    return 0;
  }
  pt_l2_base_addr = mem_read_word32(pt_l1_base_addr + L1PTN(target_addr));
  // check L2 page entry
  if(!VALID(mem_read_word32(pt_l2_base_addr + L2PTN(target_addr))))
  {
    // handle page fault(physical)
    *is_write = TRUE;
    handle_page_fault(pt_l1_base_addr, target_addr, *is_write);
    return 0;
  }
  physical_page_addr = mem_read_word32(pt_l2_base_addr + L2PTN(target_addr));
  return physical_page_addr;
}

int main(int argc, char *argv[])
{

  init(); /* DO NOT DELETE. */

  // check if x option is set
  int is_x_set = FALSE;
  
  // parse arguments
  int arg_num = 1;
  char *num_entries_buf, *num_ways_buf;
  char *filename;
  while (arg_num < argc - 1)
  {
    if (!strcmp(argv[arg_num], "-c"))
    {
      arg_num += 1;
      num_entries_buf = strtok(argv[arg_num], ":");
      num_ways_buf = strtok(NULL, ":");
      num_entries = strtol(num_entries_buf, NULL, 10);
      num_ways = strtol(num_ways_buf, NULL, 10);
      num_sets = num_entries / num_ways;
    }
    else if (!strcmp(argv[arg_num], "-x"))
    {
      is_x_set = TRUE;
    }
    else
    {
      printf("Error : not appropriate parameters\n");
      exit(1);
    }
    arg_num++;
  }
  filename = argv[arg_num];
  set_digits = get_index_digits(num_sets);
  tag_digits = VPN_DIGITS - set_digits;

  // open program
  FILE *prog;
  prog = fopen(filename, "r");
  if (!prog)
  {
    printf("Error: program doesn't exist\n");
    exit(1);
  }

  // allocate tlb
  if(DEBUG_A)printf("tlb allocated with size of %d x %d\n", num_sets, num_ways);
  tlb = (tlb_entry_t**) malloc (sizeof(tlb_entry_t*) * num_sets);
	for(int i = 0; i < num_sets; i++) {
		tlb[i] = (tlb_entry_t*) malloc(sizeof(tlb_entry_t) * num_ways);
	}
	for(int i = 0; i < num_sets; i++) {
		for(int j = 0; j < num_ways; j++) {
      memset(&tlb[i][j], 0, sizeof(tlb_entry_t));
		}
	}

  // L1 page table base address
  addr_t pt_l1_base_addr = page_table_base_addr();  

  // execute command line by line
  char op, buf[16], is_write;
  tlb_entry_t *tlb_e;
  addr_t target_addr;
  addr_t pt_l2_base_addr, physical_page_addr;
  while (fgets(buf, 15, prog) != NULL)
  {
    op = buf[0];
    target_addr = strtol(buf + 2, NULL, 16);
    total_mem_access++;
    // read operation
    if (op == 'R')
    { 
      total_read_access++;
      is_write = FALSE;
    }
    // write operation
    else if (op == 'W')
    {
      total_write_access++;
      is_write = TRUE;
    }
    // check tlb
    while(!(tlb_e = check_tlb(tlb, target_addr)))
    {
      // tlb missed
      // increase tlb miss
      total_tlb_misses++;
      // try page table walker
      physical_page_addr = walk_page_table(pt_l1_base_addr, target_addr, &is_write);
      // if got valid physical page, end loop
      if(physical_page_addr)
      {
        break;
      }
    }
    // update tlb
    update_tlb(tlb, target_addr, PTN(physical_page_addr), is_write);
    // if hit, increase tlb hits
    if(tlb_e)
    {
      total_tlb_hits++;
    }
  }
  if(DEBUG_B)printf("wa %d, wb %d, wc %d\n", wa, wb, wc);

  // dump
  cdump(num_entries, num_ways);
  sdump(total_mem_access, total_read_access, total_write_access, total_tlb_access,
        total_tlb_hits, total_tlb_misses, total_page_walks, total_page_faults);
  if (is_x_set)
    xdump(tlb, num_sets, num_ways);

  // memory and file cleanup
  for (int i = 0; i < num_sets; i++)
  {
    free(tlb[i]);
  }
  free(tlb);
  fclose(prog);

  return 0;
}
