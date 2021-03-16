#include <linux/module.h> //所有模块都必须包含的头文件
#include <linux/kernel.h> //一些宏定义，例如这里的KERN_ERR
#include <linux/sched.h>
#include <linux/cpu.h>
#include <linux/sort.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

#define RANGE 512

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c "
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define UL_TO_BINARY_PATTERN \
	BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN

#define UL_TO_BINARY(ulong) \
	BYTE_TO_BINARY(ulong >> 56), \
	BYTE_TO_BINARY(ulong >> 48), \
	BYTE_TO_BINARY(ulong >> 40), \
	BYTE_TO_BINARY(ulong >> 32), \
	BYTE_TO_BINARY(ulong >> 24), \
	BYTE_TO_BINARY(ulong >> 16), \
	BYTE_TO_BINARY(ulong >> 8), \
	BYTE_TO_BINARY(ulong >> 0) \

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
    printk(KERN_ERR
    "---------- Hello world 1. ----------\n");
    dump_pgd(get_current()->mm->pgd, 1);
    return 0;
}

void cleanup_module(void) {
    printk(KERN_ERR
    "---------- Goodbye world 1. ----------\n");
}

void dump_pgd(pgd_t *pgtable, int level) {
    int i, j;

    for (i = 0; i < RANGE; i++) {
        pgd_t pgd = pgtable[i];

        if (pgd_present(pgd) && pgd_val(pgd)) {
            for (j = 0; j < level; j++) {
                printk(KERN_CONT
                " ...");
            }

            printk(KERN_CONT "%3d: pgd "UL_TO_BINARY_PATTERN"pa "UL_TO_BINARY_PATTERN"\n", i, 
                UL_TO_BINARY(pgd_val(pgd)), UL_TO_BINARY(pgd_val(pgd) & PTE_PFN_MASK));
            dump_pud((pud_t *) pgd_page_vaddr(pgd), 2);
        }
    }
}

void dump_pud(pud_t *pgtable, int level) {
    int i, j;

    for (i = 0; i < RANGE; i++) {
        pud_t pud = pgtable[i];

        if (pud_present(pud) && pud_val(pud)) {
            for (j = 0; j < level; j++) {
                printk(KERN_CONT
                " ...");
            }

            printk(KERN_CONT "%3d: pud "UL_TO_BINARY_PATTERN"pa "UL_TO_BINARY_PATTERN"\n", i, 
                UL_TO_BINARY(pud_val(pud)), UL_TO_BINARY(pud_val(pud) & pud_pfn_mask(pud)));

            dump_pmd((pmd_t *) pud_page_vaddr(pud), 3);
        }
    }
}

void dump_pmd(pmd_t *pgtable, int level) {
    int i, j;

    for (i = 0; i < RANGE; i++) {
        pmd_t pmd = pgtable[i];

        if (pmd_present(pmd) && pmd_val(pmd)) {
            for (j = 0; j < level; j++) {
                printk(KERN_CONT
                " ...");
            }

            printk(KERN_CONT "%3d: pmd "UL_TO_BINARY_PATTERN"pa "UL_TO_BINARY_PATTERN"\n", i, 
                UL_TO_BINARY(pmd_val(pmd)), UL_TO_BINARY(pmd_val(pmd) & pmd_pfn_mask(pmd)));

            dump_pte((pte_t *) pmd_page_vaddr(pmd), 4);
        }
    }
}

void dump_pte(pte_t *pgtable, int level) {
    int i, j;

    for (i = 0; i < RANGE; i++) {
        pte_t pte = pgtable[i];

        if (pte_present(pte) && pte_val(pte)) {
            for (j = 0; j < level; j++) {
                printk(KERN_CONT
                " ...");
            }

            printk(KERN_CONT "%3d: pte "UL_TO_BINARY_PATTERN"pa "UL_TO_BINARY_PATTERN"\n", i, 
                UL_TO_BINARY(pte_val(pte)), UL_TO_BINARY(pte_val(pte) & PTE_PFN_MASK));
        }
    }
}
