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

int init_module(void)
{
	printk(KERN_ERR "---------- Hello world 1. ----------\n");
	dump_pgd(get_current()->mm->pgd, 1);
	return 0;
}

void cleanup_module(void)
{
	printk(KERN_ERR "---------- Goodbye world 1. ----------\n");
}

void dump_pgd(pgd_t *pgtable, int level) {
	int i, j;
	for (i = 0; i < 512; i++) {
		pgd_t pgd = pgtable[i];

		if (pgd_present(pgd)) {
			for (j = 0; j < level; j++) {
				printk(KERN_CONT " ..");
			}
			printk(KERN_CONT "%d: pgd %p pa %p\n", i, (void *)pgd_val(pgd), (void *)(pgd_val(pgd) & PTE_PFN_MASK));

			// dump_pud((pud_t *)pgd_page_vaddr(pgd), 2);
		}
	}
}

void dump_pud(pud_t *pgtable, int level) {
	int i, j;
	for (i = 0; i < 512; i++) {
		pud_t pud = pgtable[i];

		if (pud_present(pud)) {
			for (j = 0; j < level; j++) {
				printk(KERN_CONT " ..");
			}
			printk(KERN_CONT "%d: pud %p pa %p\n", i, (void *)pud_val(pud), (void *)(pud_val(pud) & pud_pfn_mask(pud)));

			dump_pmd((pmd_t *)pud_page_vaddr(pud), 3);
		}
	}
}

void dump_pmd(pmd_t *pgtable, int level) {
	int i, j;
	for (i = 0; i < 512; i++) {
		pmd_t pmd = pgtable[i];

		if (pmd_present(pmd)) {
			for (j = 0; j < level; j++) {
				printk(KERN_CONT " ..");
			}
			printk(KERN_CONT "%d: pmd %p pa %p\n", i, (void *)pmd_val(pmd), (void *)(pmd_val(pmd) & pmd_pfn_mask(pmd)));

			dump_pte((pte_t *)pmd_page_vaddr(pmd), 4);
		}
	}
}

void dump_pte(pte_t *pgtable, int level) {
	int i, j;
	for (i = 0; i < 512; i++) {
		pte_t pte = pgtable[i];

		if (pte_present(pte)) {
			for (j = 0; j < level; j++) {
				printk(KERN_CONT " ..");
			}
			printk(KERN_CONT "%d: pte %p pa %p\n", i, (void *)pte_val(pte), (void *)(pte_val(pte) & PTE_PFN_MASK));
		}
	}
}
