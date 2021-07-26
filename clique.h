#ifndef __CLIQUE_H__
#define __CLIQUE_H__

#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
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
#define NPROCESS 2
#define C_PRINT
#define VALID_ONLY
#define PID_HASH_BITS 14UL


//#define C_USEMAX

#define C_ASSERT(v) 										\
	{ 																		\
		if (unlikely(!(v))) {								\
			printk(KERN_ERR "C_ASSERT failed in %s at %d", __FUNCTION__, __LINE__);\
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
	int *pids;
	struct list_head list;
};

struct c_thread_info {
	struct process_info *pi;
	int tid; // for indexing matrix
};

extern struct c_thread_info thread_list[1UL << PID_HASH_BITS];
extern struct process_info process_list;

static inline
void *resize(void *old, unsigned long old_size, unsigned long new_size) {
	void *ret = kmalloc(new_size, GFP_KERNEL);
	memcpy(ret, old, old_size);
	kfree(old);
	memset(ret + old_size, 0, new_size - old_size);
	return ret;
}

// called before insert_process & insert_thread
static inline
int check_name(char *comm) {
	int len = strlen(comm);
	if (unlikely(comm[len - 2] == '.' && comm[len - 1] == 'x'))
		return 1;
	return 0;
}

static inline
void insert_process(char *comm, int pid) {
	struct process_info *pi = (struct process_info *) 
		kmalloc(sizeof(struct process_info), GFP_KERNEL);
	int h = hash_32(pid, PID_HASH_BITS);
	C_ASSERT(pi != NULL);

	strcpy(pi->comm, comm);
	pi->alloc_threads = NTHREADS;
	atomic_set(&pi->nthreads, 1);
	pi->matrix = (int *) kmalloc(sizeof(int) 
		* pi->alloc_threads * pi->alloc_threads, GFP_KERNEL);
	pi->pids = (int *) kmalloc(sizeof(int) * pi->alloc_threads, GFP_KERNEL);
	C_ASSERT(pi->matrix != NULL);
	C_ASSERT(pi->pids != NULL);
	memset(pi->matrix, 0, sizeof(int)
		* pi->alloc_threads * pi->alloc_threads);
	
	pi->pids[0] = pid;
	INIT_LIST_HEAD(&pi->list);
	list_add(&pi->list, &process_list.list);

	C_ASSERT(thread_list[h].pi == NULL);
	thread_list[h].pi = pi;
	thread_list[h].tid = 0;
}

static inline
void insert_thread(char *comm, int pid) {
	int h = hash_32(pid, PID_HASH_BITS), tid;
	struct process_info *pi = NULL;
	struct list_head *curr;
	
	list_for_each(curr, &process_list.list) {
		pi = list_entry(curr, struct process_info, list);
		if (!strcmp(pi->comm, comm)) {
			break;
		}
	}

	if (!pi) {
		C_ASSERT(pi != NULL);
		printk(KERN_ERR "No process %s, %d", comm, pid);
		return;
	}

	if (thread_list[h].pi) {
		C_ASSERT(thread_list[h].pi == NULL);
		printk("Thread hash collision %s, %d", comm, pid);
		return;
	}

	thread_list[h].pi = pi;
	tid = atomic_inc_return(&pi->nthreads) - 1;
	
	if (unlikely(tid + 1 > pi->alloc_threads)) {
		// realloc everything
		pi->matrix = (int *) resize(pi->matrix, 
			sizeof(int) * pi->alloc_threads * pi->alloc_threads, 
			4 * sizeof(int) * pi->alloc_threads * pi->alloc_threads
		);
		pi->pids = (int *) resize(pi->pids,
			sizeof(int) * pi->alloc_threads,
			2 * sizeof(int) * pi->alloc_threads
		);
		pi->alloc_threads = 2 * pi->alloc_threads;
		C_ASSERT(pi->matrix);
		C_ASSERT(pi->pids);
	}

	pi->pids[tid] = pid;
	thread_list[h].tid = tid;
}

void remove_thread(char *comm, int pid) {
	int h = hash_32(pid, PID_HASH_BITS), tid;
	struct process_info *pi = NULL;
	struct list_head *curr;
	
	list_for_each(curr, &process_list.list) {
		pi = list_entry(curr, struct process_info, list);
		if (!strcmp(pi->comm, comm)) {
			break;
		}
	}

	if (!pi) {
		C_ASSERT(pi);
		printk(KERN_ERR "No process %s, %d", comm, pid);
		return;
	}
	if (!thread_list[h].pi) {
		C_ASSERT(thread_list[h].pi);
		printk(KERN_ERR "No process info %s, %d", comm, pid);
		return;
	}
	
	thread_list[h].pi = NULL;
	thread_list[h].tid = 0;

	tid = atomic_dec_return(&pi->nthreads);
	if (unlikely(tid == 0)) {
		memset(pi->matrix, 0, pi->alloc_threads * pi->alloc_threads * sizeof(int));
		memset(pi->pids, 0, pi->alloc_threads * sizeof(int));
		// we do not dealloc here
	}
}

static inline
void record_access(int pid, unsigned long address) {
	
}

#endif