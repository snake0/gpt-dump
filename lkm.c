#include <linux/module.h> //所有模块都必须包含的头文件
#include <linux/kernel.h> //一些宏定义，例如这里的KERN_ERR
#include <linux/sched.h>
#include <linux/cpu.h>
#include <linux/sort.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define NTHREADS 16
int spd_comm[NTHREADS][NTHREADS] = {
	{0, 17, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 2, 1, 0, 13},
	{17, 0, 23, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
	{0, 23, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 24, 0, 10, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
	{1, 1, 0, 10, 0, 17, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
	{1, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 19, 0, 23, 0, 0, 0, 0, 0},
	{1, 0, 0, 1, 1, 0, 0, 0, 0, 23, 0, 7, 1, 1, 0, 0},
	{1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 7, 0, 10, 1, 0, 0},
	{2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 10, 0, 21, 0, 0},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 21, 0, 3, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 5},
	{13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0}};

int spd_comm_sort_idx[NTHREADS][NTHREADS] = {
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};

static int sum_comm[NTHREADS];
static int sum_comm_sort_idx[NTHREADS];

static int comm_offset, sum_offset;
static int *comm_start;

static int comm_cmp_func(const void *spd_comm, const void *spd_comm_sort_idx)
{
	return comm_start[*(int *)spd_comm_sort_idx] - comm_start[*(int *)spd_comm];
}

static int sum_cmp_func(const void *spd_comm, const void *spd_comm_sort_idx)
{
	return sum_comm[*(int *)spd_comm_sort_idx] - sum_comm[*(int *)spd_comm];
}

static void spd_print_comm(int k[][NTHREADS])
{
	int i, j;
	printk(KERN_ERR
		   "------[ matrix ]------\n");
	for (i = 0; i < NTHREADS; ++i)
	{
		for (j = 0; j < NTHREADS; ++j)
		{
			printk(KERN_CONT
				   "%d ",
				   k[i][j]);
		}
		printk(KERN_CONT
			   "\n");
	}
	printk(KERN_ERR
		   "======[ matrix ]======\n");
}

static void spd_print_arr(int k[NTHREADS])
{
	int i;
	printk(KERN_ERR
		   "------[ array ]------\n");
	for (i = 0; i < NTHREADS; ++i)
	{
		printk(KERN_CONT
			   "%d ",
			   k[i]);
	}
	printk(KERN_ERR
	   "======[ array ]======\n");
}

static int sum_arr(int *arr)
{
	int i, ret = 0;
	for (i = 0; i < NTHREADS; ++i)
		ret += arr[i];
	return ret;
}

static int now_idx[NTHREADS], threads_chosen[NTHREADS];

static int core_to_thread[NTHREADS];


int init_module(void)
{
	int i;
	int cores_per_node = 8, nnodes = 2;
	int *now_core = (int *) kmalloc(sizeof(int) * nnodes, GFP_KERNEL);

	memset(threads_chosen, -1, NTHREADS * sizeof(int));

	comm_offset = &spd_comm_sort_idx[0][0] - &spd_comm[0][0];
	sum_offset = &sum_comm_sort_idx[0] - &sum_comm[0];
	
	for (i = 0; i < NTHREADS; ++i)
	{
		comm_start = &spd_comm[0][0] + i * NTHREADS;
		sum_comm_sort_idx[i] = i;
		sum_comm[i] = sum_arr(comm_start);
		sort(comm_start + comm_offset, NTHREADS, sizeof(int), comm_cmp_func, NULL);
	}
	sort(sum_comm_sort_idx, NTHREADS, sizeof(int), sum_cmp_func, NULL);

	for (i = 0; i < NTHREADS; ++i)
	{
		printk(KERN_CONT "%d -> %d\n",
			   sum_comm_sort_idx[i], sum_comm[sum_comm_sort_idx[i]]);
	}

	for (i = 0; i < nnodes; ++i) {
		now_core[i] = cores_per_node - 1;
		threads_chosen[sum_comm_sort_idx[i]] = now_core[i] + i * cores_per_node;
		--now_core[i];
	}
	
	for (i = 0; i < nnodes; ++i) {
		int curr = sum_comm_sort_idx[i], neighbor;

		while (now_core[i] >= 0) {
			int *idx = &spd_comm_sort_idx[curr][0];

			while (threads_chosen[idx[now_idx[curr]]] != -1)
				now_idx[curr] = (now_idx[curr] + 1) % NTHREADS;
			neighbor = idx[now_idx[curr]];

			threads_chosen[neighbor] = now_core[i] + i * cores_per_node;
			--now_core[i];

			curr = neighbor;
		}
	}

	for (i = 0; i < NTHREADS; ++i) {
		core_to_thread[threads_chosen[i]] = i;
	}
	

	// printk(KERN_ERR "Hello world 1.\n");
	// spd_print_comm(spd_comm);
	// spd_print_comm(spd_comm_sort_idx);
	// now = 0;
	// printk("Greatest comm:");
	// for (i = 0; i < NTHREADS; ++i) {
	// 	printk("%d -> %d", spd_comm_sort_idx[i][NTHREADS-1], spd_comm[i][spd_comm_sort_idx[i][NTHREADS-1]]);
	// }

	spd_print_comm(spd_comm);
	spd_print_comm(spd_comm_sort_idx);
	spd_print_arr(core_to_thread);

	//topo_init();
	kfree(now_core);

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_ERR "Goodbye world 1.\n");
}

