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

MODULE_LICENSE("GPL");

#define RANGE 512

#define PFN(entry) ((entry)&PTE_PFN_MASK)

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c "
#define BYTE_TO_BINARY(byte)                                                   \
  (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'),                        \
      (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'),                    \
      (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'),                    \
      (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

#define UL_TO_BINARY_PATTERN                                                   \
  BYTE_TO_BINARY_PATTERN                                                       \
  BYTE_TO_BINARY_PATTERN                                                       \
  BYTE_TO_BINARY_PATTERN                                                       \
  BYTE_TO_BINARY_PATTERN                                                       \
  BYTE_TO_BINARY_PATTERN                                                       \
  BYTE_TO_BINARY_PATTERN                                                       \
  BYTE_TO_BINARY_PATTERN                                                       \
  BYTE_TO_BINARY_PATTERN

#define UL_TO_BINARY(ulong)                                                    \
  BYTE_TO_BINARY(ulong >> 56), BYTE_TO_BINARY(ulong >> 48),                    \
      BYTE_TO_BINARY(ulong >> 40), BYTE_TO_BINARY(ulong >> 32),                \
      BYTE_TO_BINARY(ulong >> 24), BYTE_TO_BINARY(ulong >> 16),                \
      BYTE_TO_BINARY(ulong >> 8), BYTE_TO_BINARY(ulong >> 0)

static inline bool is_hypervisor_range(int idx) {
#ifdef CONFIG_X86_64
  /*
   * ffff800000000000 - ffff87ffffffffff is reserved for
   * the hypervisor.
   */
  return (idx >= pgd_index(__PAGE_OFFSET) - 16) &&
         (idx < pgd_index(__PAGE_OFFSET));
#else
  return false;
#endif
}

typedef unsigned long ul;

void dump_pgd(pgd_t *pgtable, int level);

void dump_pud(pud_t *pgtable, int level);

void dump_pmd(pmd_t *pgtable, int level);

void dump_pte(pte_t *pgtable, int level);

// static inline ul pgd_page_vaddr(pgd_t pgd)
// {
// 	return (ul)__va((ul)pgd_val(pgd) & PTE_PFN_MASK);
// }

// static inline ul pud_page_vaddr(pud_t pud)
// {
// 	return (ul)__va(pud_val(pud) & pud_pfn_mask(pud));
// }

// static inline ul pmd_page_vaddr(pmd_t pmd)
// {
// 	return (ul)__va(pmd_val(pmd) & pmd_pfn_mask(pmd));
// }

int init_module(void) {
  printk(KERN_ERR "---------- Hello world 1. ----------\n");
  dump_pgd(current->mm->pgd, 1);
  // dump_pgd(init_level4_pgt, 1);

  return 0;
}

void cleanup_module(void) {
  printk(KERN_ERR "---------- Goodbye world 1. ----------\n");
}

void dump_pgd(pgd_t *pgtable, int level) {
  int i, j;
  pgd_t pgd;

  for (i = 0; i < PTRS_PER_PGD; i++) {
    pgd = pgtable[i];

    if (pgd_val(pgd)) {
      if (pgd_present(pgd) && !pgd_large(pgd)) {
        for (j = 0; j < level; j++)
          printk(KERN_CONT " ...");

        printk(KERN_CONT "%3d: pgd " UL_TO_BINARY_PATTERN "\n", i,
               UL_TO_BINARY(pgd_val(pgd)));

        dump_pud((pud_t *)pgd_page_vaddr(pgd), 2);
      }
    }
  }
}

void dump_pud(pud_t *pgtable, int level) {
  int i, j;
  pud_t pud;

  for (i = 0; i < PTRS_PER_PUD; i++) {
    pud = pgtable[i];

    if (pud_val(pud)) {
      if (pud_present(pud) && !pud_large(pud)) {
        for (j = 0; j < level; j++)
          printk(KERN_CONT " ...");

        printk(KERN_CONT "%3d: pud " UL_TO_BINARY_PATTERN "\n", i,
               UL_TO_BINARY(pud_val(pud)));

        dump_pmd((pmd_t *)pud_page_vaddr(pud), 3);
      }
    }
  }
}

void dump_pmd(pmd_t *pgtable, int level) {
  int i, j;
  pmd_t pmd;

  for (i = 0; i < PTRS_PER_PMD; i++) {
    pmd = pgtable[i];

    if (pmd_val(pmd)) {
      if (pmd_present(pmd) && !pmd_large(pmd)) {
        for (j = 0; j < level; j++)
          printk(KERN_CONT " ...");

        printk(KERN_CONT "%3d: pmd " UL_TO_BINARY_PATTERN "\n", i,
               UL_TO_BINARY(pmd_val(pmd)));

        dump_pte((pte_t *)pmd_page_vaddr(pmd), 4);
      }
    }
  }
}

void dump_pte(pte_t *pgtable, int level) {
  int i, j;
  pte_t pte;

  for (i = 0; i < PTRS_PER_PTE; i++) {
    pte = pgtable[i];

    // if (pte_val(pte)) {
      // if (pte_present(pte)) {
        for (j = 0; j < level; j++)
          printk(KERN_CONT " ...");

        printk(KERN_CONT "%3d: pte " UL_TO_BINARY_PATTERN "\n", i,
               UL_TO_BINARY(pte_val(pte)));
      // }
    // }
  }
}
