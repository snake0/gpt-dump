#ifndef __CLIQUE_H__
#define __CLIQUE_H__

#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/string.h>
#include <linux/sched/signal.h>
#include <linux/hashtable.h>
#include <linux/sched.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>
#include <asm/kvm_para.h>


#define NTHREADS 32
#define NPROCESS 1
#define C_PRINT
#define VALID_ONLY
//#define C_USEMAX

#define C_ASSERT(v) 										\
	{ 																		\
		if (unlikely(!(v))) {								\
			printk(KERN_ERR "assert failed!");\
		}																		\
	}

struct clique {
    int pids[NTHREADS];
    int size;
    enum {
        C_VALID, C_REUSE, C_INVALID
    } flag;
};

struct process_info {
	char comm[TASK_COMM_LEN];
	int alloc_threads;
	atomic_t nthreads;
	int *matrix;
	struct task_struct *balancer;
};

extern struct process_info *process_list;
extern int alloc_processes;
extern atomic_t nprocesses;

void init_process_list(void);
void destroy_process_list(void);

static inline
struct process_info *search_process(char *comm) {
	int size = atomic_read(&nprocesses), i;
	for (i = 0; i < size; ++i) {
		if (!strcmp(process_list[i].comm, comm)) {
			return process_list + i;
		}
	}
	return NULL;
}

static inline
void insert_process(char *comm) {
	int i;
	C_ASSERT(search_process(comm) == NULL);
	if (atomic_read(&nprocesses) == alloc_processes) {
		process_list = (struct process_info *)
			krealloc(process_list, 2 * alloc_processes, GFP_KERNEL);
		alloc_processes = 2 * alloc_processes;
	}
	i = atomic_inc_return(&nprocesses) - 1;
	strcpy(process_list[i].comm, comm);
	atomic_set(&process_list[i].nthreads, 0);
	process_list[i].alloc_threads = NTHREADS;
	process_list[i].matrix = (int *) kmalloc(NTHREADS * NTHREADS);
}

static inline
void remove_process(char *comm) {
	// TODO: use linked list
	kfree(process_list[0].matrix);
	process_list[0].comm = "";
	process_list[0].alloc_threads = 0;
	atomic_set(&process_list[i].nthreads, 0);
}

static inline
int check_name(char *comm) {
	
}

static inline
void insert_thread(int pid) {
	
}

#endif