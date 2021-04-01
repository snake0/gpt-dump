#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/string.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>
#include <asm/kvm_para.h>

MODULE_LICENSE("GPL");

/* convert to 0, 0, 1, 0, 1, 1, 0, ... */

// convert unsigned long to vaddr
#define BYTE_TO_BINARY(byte) \
  ((byte) & 0x80 ? '1' : '0'), ((byte) & 0x40 ? '1' : '0'),                    \
  ((byte) & 0x20 ? '1' : '0'), ((byte) & 0x10 ? '1' : '0'),                    \
  ((byte) & 0x08 ? '1' : '0'), ((byte) & 0x04 ? '1' : '0'),                    \
  ((byte) & 0x02 ? '1' : '0'), ((byte) & 0x01 ? '1' : '0')

#define TBYTE_TO_BINARY(tbyte)  \
  ((tbyte) & 0x04 ? '1' : '0'),    \
  ((tbyte) & 0x02 ? '1' : '0'), ((tbyte) & 0x01 ? '1' : '0')

#define UL_TO_PTE_OFFSET(ulong) \
  TBYTE_TO_BINARY((ulong) >> 9), TBYTE_TO_BINARY((ulong) >> 6), \
  TBYTE_TO_BINARY((ulong) >> 3), TBYTE_TO_BINARY((ulong))

#define UL_TO_PTE_INDEX(ulong) \
  TBYTE_TO_BINARY((ulong) >> 6), TBYTE_TO_BINARY((ulong) >> 3), TBYTE_TO_BINARY((ulong))

#define UL_TO_VADDR(ulong) \
  UL_TO_PTE_INDEX((ulong) >> 39), UL_TO_PTE_INDEX((ulong) >> 30), \
  UL_TO_PTE_INDEX((ulong) >> 21), UL_TO_PTE_INDEX((ulong) >> 12), \
  UL_TO_PTE_OFFSET((ulong))


// convert unsigned long to pte
#define UL_TO_PTE_PHYADDR(ulong) \
  BYTE_TO_BINARY((ulong) >> 32),    \
  BYTE_TO_BINARY((ulong) >> 24), BYTE_TO_BINARY((ulong) >> 16), \
  BYTE_TO_BINARY((ulong) >> 8), BYTE_TO_BINARY((ulong) >> 0)

#define UL_TO_PTE_IR(ulong) \
  UL_TO_PTE_OFFSET(ulong)

#define UL_TO_PTE(ulong) \
  UL_TO_PTE_IR((ulong) >> 52), UL_TO_PTE_PHYADDR((ulong) >> PAGE_SHIFT), UL_TO_PTE_OFFSET(ulong)

#define UL_TO_PADDR(ulong) \
  UL_TO_PTE_PHYADDR((ulong) >> PAGE_SHIFT), UL_TO_PTE_OFFSET(ulong)

/* printk pattern strings */

// convert unsigned long to vaddr
#define TBYTE_TO_BINARY_PATTERN   "%c%c%c"
#define BYTE_TO_BINARY_PATTERN    "%c%c%c%c%c%c%c%c"

#define PTE_INDEX_PATTERN \
  TBYTE_TO_BINARY_PATTERN TBYTE_TO_BINARY_PATTERN TBYTE_TO_BINARY_PATTERN " "

#define VADDR_OFFSET_PATTERN \
  TBYTE_TO_BINARY_PATTERN TBYTE_TO_BINARY_PATTERN \
  TBYTE_TO_BINARY_PATTERN TBYTE_TO_BINARY_PATTERN

#define VADDR_PATTERN \
  PTE_INDEX_PATTERN PTE_INDEX_PATTERN \
  PTE_INDEX_PATTERN PTE_INDEX_PATTERN \
  VADDR_OFFSET_PATTERN


// convert unsigned long to pte
// 40 bits
#define PTE_PHYADDR_PATTREN \
  BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN \
  BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN \
  BYTE_TO_BINARY_PATTERN " "

// 12 bits
#define PTE_IR_PATTERN \
  VADDR_OFFSET_PATTERN " "

// 12 + 40 + 12 bits
#define PTE_PATTERN \
  PTE_IR_PATTERN PTE_PHYADDR_PATTREN VADDR_OFFSET_PATTERN

#define PADDR_PATTERN \
  PTE_PHYADDR_PATTREN VADDR_OFFSET_PATTERN

/* static vals */
unsigned long vaddr, paddr, pgd_idx, pud_idx, pmd_idx, pte_idx;
const char *prefixes[] = {"pgd", "pud", "pmd", "pte"};
const char *PREFIXES[] = {"PGD", "PUD", "PMD", "PTE"};

/* static inline functions */
static inline void print_ulong_pte(unsigned long address, unsigned long ulong, 
                                  unsigned long i, int level) {
  pr_cont(" PhyAddr  =  ");

  if (level == 1)
    pr_cont(PADDR_PATTERN" (From CR3) \n", UL_TO_PADDR(address));
  else
    pr_cont(PADDR_PATTERN" (From %s) \n", UL_TO_PADDR(address), prefixes[level - 2]);
  
  pr_cont("          +  ");
  pr_cont(PADDR_PATTERN" * 64 (From %s Idx)\n", UL_TO_PADDR((i)), PREFIXES[level - 1]);

  pr_cont(" %3lu: %s " PTE_PATTERN"\n", i, prefixes[level - 1], UL_TO_PTE(ulong));
  pr_err("-----------------------------------------------------------------------------------------\n");
}

static inline void print_ptr_vaddr(volatile unsigned long *ptr) {
  unsigned long mask = ((1 << 9) - 1);

  vaddr = (unsigned long) ptr;
  pgd_idx = (vaddr >> 39) & mask;
  pud_idx = (vaddr >> 30) & mask;
  pmd_idx = (vaddr >> 21) & mask;
  pte_idx = (vaddr >> 12) & mask;
  pr_info("GPT PGD index: %lu", pgd_idx);
  pr_info("GPT PUD index: %lu", pud_idx);
  pr_info("GPT PMD index: %lu", pmd_idx);
  pr_info("GPT PTE index: %lu", pte_idx);

  pr_info("         %lu       %lu       %lu       %lu", pgd_idx, pud_idx, pmd_idx, pte_idx);
  pr_info("GVA [PGD Idx] [PUD Idx] [PMD Idx] [PTE Idx] [  Offset  ]");
  pr_info("GVA "VADDR_PATTERN"\n", UL_TO_VADDR(vaddr));
}

static inline void print_pa_check(unsigned long vaddr) {
  // unsigned long pfn, offset;

  paddr = __pa(vaddr);
  // pfn = paddr >> PAGE_SHIFT;
  // offset = paddr & ((1 << PAGE_SHIFT) - 1);

  // pr_info("... ... ... ...  73: pte 100000000000 0000000000000000000100111011 000001100011");
  // pr_info("  35: pte 100000000000 0000000000000000000100110101010000100011 000001100011");
  pr_info("        __pa(vaddr) =  " PADDR_PATTERN "\n", UL_TO_PADDR(paddr));
  // pr_err("-----------------------------------------------------------------------------------------");
  // pr_info("VADDR 100101100 010101111 111011110 010010011 ");

  // pr_info("VADDR [  PGD  ] [  PUD  ] [  PMD  ] [  PTE  ] [  Offset  ]");
  // pr_info("VADDR "VADDR_PATTERN"\n", UL_TO_VADDR(vaddr));
  // pr_info("                     Offset by __pa()         \n" 
  //   VADDR_OFFSET_PATTERN, UL_TO_PTE_OFFSET(offset));
}

/* page table walker functions */
void dump_pgd(pgd_t *pgtable, int level);
void dump_pud(pud_t *pgtable, int level);
void dump_pmd(pmd_t *pgtable, int level);
void dump_pte(pte_t *pgtable, int level);

int init_module(void) {
  volatile unsigned long *ptr;
  
  ptr = kmalloc(sizeof(int), GFP_KERNEL);
  *ptr = 1772333;

  print_ptr_vaddr(ptr);
  dump_pgd(current->mm->pgd, 1);
  print_pa_check(vaddr);
  // printk("!!! %lu", ++*ptr);

  kvm_hypercall2(22, paddr, *ptr);
  kfree((const void *)ptr);

  return -1;
}

void cleanup_module(void) {}

void dump_pgd(pgd_t *pgtable, int level) {
  unsigned long i;
  pgd_t pgd;
  pr_err("-----------------------------------------------------------------------------------------\n");
  
  pr_cont("CR3:         ");
  pr_cont(PADDR_PATTERN "\n", UL_TO_PADDR(__pa(pgtable)));

  pr_err("Page Table Printing Format:\n");
  pr_err(" Idx: Lvl [Rsvd./Ign.] [     Physical Frame Number, 40 Bits   ] [   Flags  ]");
  pr_err(" Idx: Lvl [ 12 Bits  ] [ Physical Address is 52-bit Wide for Intel Core i7 ]\n");
  pr_err("-----------------------------------------------------------------------------------------\n");

  for (i = 0; i < PTRS_PER_PGD; i++) {
    pgd = pgtable[i];

    if (pgd_val(pgd)) {
      if (pgd_present(pgd) && !pgd_large(pgd)) {
        if (i == pgd_idx) {
          print_ulong_pte(__pa(pgtable), pgd_val(pgd), i, level);

          dump_pud((pud_t *)pgd_page_vaddr(pgd), level + 1);
        }
      }
    }
  }
}

void dump_pud(pud_t *pgtable, int level) {
  unsigned long i;
  pud_t pud;

  for (i = 0; i < PTRS_PER_PUD; i++) {
    pud = pgtable[i];

    if (pud_val(pud)) {
      if (pud_present(pud) && !pud_large(pud)) {
        if (i == pud_idx) {
          print_ulong_pte(__pa(pgtable), pud_val(pud), i, level);

          dump_pmd((pmd_t *)pud_page_vaddr(pud), level + 1);
        }
      }
    }
  }
}

void dump_pmd(pmd_t *pgtable, int level) {
  unsigned long i;
  pmd_t pmd;

  for (i = 0; i < PTRS_PER_PMD; i++) {
    pmd = pgtable[i];

    if (pmd_val(pmd)) {
      if (pmd_present(pmd) && !pmd_large(pmd)) {
        if (i == pmd_idx) {
          print_ulong_pte(__pa(pgtable), pmd_val(pmd), i, level);

          dump_pte((pte_t *)pmd_page_vaddr(pmd), level + 1);
        }
      }
    }
  }
}

void dump_pte(pte_t *pgtable, int level) {
  unsigned long i;
  pte_t pte;

  for (i = 0; i < PTRS_PER_PTE; i++) {
    pte = pgtable[i];

    if (pte_val(pte)) {
      if (pte_present(pte)) {
        if (i == pte_idx)
          print_ulong_pte(__pa(pgtable), pte_val(pte), i, level);
      }
    }
  }
}
