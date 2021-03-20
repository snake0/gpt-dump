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
  (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'),                    \
  (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'),                    \
  (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'),                    \
  (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

#define TBYTE_TO_BINARY(tbyte)  \
  (tbyte & 0x04 ? '1' : '0'),    \
  (tbyte & 0x02 ? '1' : '0'), (tbyte & 0x01 ? '1' : '0')

#define UL_TO_PTE_OFFSET(ulong) \
  TBYTE_TO_BINARY(ulong >> 9), TBYTE_TO_BINARY(ulong >> 6), \
  TBYTE_TO_BINARY(ulong >> 3), TBYTE_TO_BINARY(ulong)

#define UL_TO_PTE_INDEX(ulong) \
  TBYTE_TO_BINARY(ulong >> 6), TBYTE_TO_BINARY(ulong >> 3), TBYTE_TO_BINARY(ulong)

#define UL_TO_VADDR(ulong) \
  UL_TO_PTE_INDEX(ulong >> 39), UL_TO_PTE_INDEX(ulong >> 30), \
  UL_TO_PTE_INDEX(ulong >> 21), UL_TO_PTE_INDEX(ulong >> 12), \
  UL_TO_PTE_OFFSET(ulong)


// convert unsigned long to pte
#define UL_TO_PTE_PHYADDR(ulong) \
  BYTE_TO_BINARY(ulong >> 32),    \
  BYTE_TO_BINARY(ulong >> 24), BYTE_TO_BINARY(ulong >> 16), \
  BYTE_TO_BINARY(ulong >> 8), BYTE_TO_BINARY(ulong >> 0)

#define UL_TO_PTE_IR(ulong) \
  UL_TO_PTE_OFFSET(ulong)

#define UL_TO_PTE(ulong) \
  UL_TO_PTE_IR(ulong >> 52), UL_TO_PTE_PHYADDR(ulong >> PAGE_SHIFT), UL_TO_PTE_OFFSET(ulong)


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
#define PTE_PHYADDR_PATTREN \
  BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN \
  BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN \
  BYTE_TO_BINARY_PATTERN " "

#define PTE_IR_PATTERN \
  VADDR_OFFSET_PATTERN " "

#define PTE_PATTERN \
  PTE_IR_PATTERN PTE_PHYADDR_PATTREN VADDR_OFFSET_PATTERN

/* static vals */
unsigned long vaddr, paddr, pgd_idx, pud_idx, pmd_idx, pte_idx; 

/* static inline functions */
static inline void print_ulong_pte(unsigned long ulong, unsigned long i, int level, char *prefix) {
  int j;
  for (j = 0; j < level; ++j)
    pr_cont("... ");
  
  pr_cont("%3lu: %s " PTE_PATTERN"\n", i, prefix, UL_TO_PTE(ulong));
}

static inline void print_ptr_vaddr(void *ptr) {
  unsigned long mask = ((1 << 9) - 1);

  vaddr = (unsigned long) ptr;
  pgd_idx = (vaddr >> 39) & mask;
  pud_idx = (vaddr >> 30) & mask;
  pmd_idx = (vaddr >> 21) & mask;
  pte_idx = (vaddr >> 12) & mask;

  pr_info("PGD index: %lu", pgd_idx);
  pr_info("PUD index: %lu", pud_idx);
  pr_info("PMD index: %lu", pmd_idx);
  pr_info("PTE index: %lu", pte_idx);

  // pr_info("100110010 101101000 111010000 101000110 110010001000");
  pr_info("VADDR [  PGD  ] [  PUD  ] [  PMD  ] [  PTE  ] [  Offset  ]");
  pr_info("VADDR "VADDR_PATTERN"\n", UL_TO_VADDR(vaddr));
}

static void print_pa_check(unsigned long vaddr) {
  unsigned long pfn, offset;

  paddr = __pa(vaddr);
  pfn = paddr >> PAGE_SHIFT;
  offset = paddr & ((1 << PAGE_SHIFT) - 1);

  // pr_info("... ... ... ...  73: pte 100000000000 0000000000000000000100111011 000001100011");
  pr_info("      Physical Frame Number by __pa() " 
    PTE_PHYADDR_PATTREN, UL_TO_PTE_PHYADDR(pfn));
  pr_err("-------------------------------------------------------------------------------");
  // pr_info("VADDR 100101100 010101111 111011110 010010011 ");

  pr_info("VADDR [  PGD  ] [  PUD  ] [  PMD  ] [  PTE  ] [  Offset  ]");
  pr_info("VADDR "VADDR_PATTERN"\n", UL_TO_VADDR(vaddr));
  pr_info("                     Offset by __pa()         " 
    VADDR_OFFSET_PATTERN, UL_TO_PTE_OFFSET(offset));
}

/* page table walker functions */
void dump_pgd(pgd_t *pgtable, int level);

void dump_pud(pud_t *pgtable, int level);

void dump_pmd(pmd_t *pgtable, int level);

void dump_pte(pte_t *pgtable, int level);

int init_module(void) {
  volatile unsigned long *ptr;
  pr_err("----------------------- BEGIN ----------------------------");
  
  ptr = kmalloc(sizeof(int), GFP_KERNEL);
  *ptr = 1771;

  print_ptr_vaddr(ptr);
  dump_pgd(current->mm->pgd, 1);
  print_pa_check(vaddr);
  printk("!!! %lu", *ptr);

  kvm_hypercall2(22, paddr, *ptr);
  return 0;
}

void cleanup_module(void) {
  pr_err("----------------------- END ------------------------------\n");
}

void dump_pgd(pgd_t *pgtable, int level) {
  unsigned long i;
  pgd_t pgd;
  // pr_err("... 171: pgd 100000000000 0000000000000000000010001110101011110110 000001100111");
  // pr_err("VADDR 100110010 101101000 111010000 101000110 101011100000");
  pr_err("----------------------------------------------------------");
  pr_err("Guest Page Table Walk BEGIN");
  pr_err("... [i]: [L] [Rsvd./Ign.] [     Physical Frame Number, 40 Bits   ] [   Flags  ]");
  pr_err("... [i]: [L] [ 12 Bits  ] [ Physical Address is 52-bit Wide for Intel Core i7 ]\n");

  for (i = 0; i < PTRS_PER_PGD; i++) {
    pgd = pgtable[i];

    if (pgd_val(pgd)) {
      if (pgd_present(pgd) && !pgd_large(pgd)) {
        if (i == pgd_idx) {
          print_ulong_pte(pgd_val(pgd), i, level, "pgd");

          dump_pud((pud_t *)pgd_page_vaddr(pgd), 2);
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
          print_ulong_pte(pud_val(pud), i, level, "pud");

          dump_pmd((pmd_t *)pud_page_vaddr(pud), 3);
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
          print_ulong_pte(pmd_val(pmd), i, level, "pmd");

          dump_pte((pte_t *)pmd_page_vaddr(pmd), 4);
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
          print_ulong_pte(pte_val(pte), i, level, "pte");
      }
    }
  }
}
