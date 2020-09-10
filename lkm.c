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

static int comm_cmp_func(const void *a, const void *b)
{
	return comm_start[*(int *)b] - comm_start[*(int *)a];
}

static int sum_cmp_func(const void *a, const void *b)
{
	return sum_comm[*(int *)b] - sum_comm[*(int *)a];
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

static int threads_chosen[NTHREADS];

static int core_to_thread[NTHREADS];

struct numa_info {
	int core; // curr core to map thread
	int leader; // leader thread seq
};

static inline bool check_neighbor(int seq1, int seq2)
{
    return (spd_comm[seq1][seq2] >= spd_comm[seq1][spd_comm_sort_idx[seq1][1]]
				|| spd_comm[seq1][seq2] >= spd_comm[seq2][spd_comm_sort_idx[seq2][1]])
				&& spd_comm[seq1][seq2] > spd_comm[seq1][spd_comm_sort_idx[seq1][NTHREADS - 1]]
				&& spd_comm[seq1][seq2] > spd_comm[seq2][spd_comm_sort_idx[seq2][NTHREADS - 1]];
}

int init_module(void)
{
	int i;
	int cores_per_node = 8, nnodes = 2;
	struct numa_info *ni = kmalloc(sizeof(*ni) * nnodes, GFP_KERNEL);

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

	ni[0].core = cores_per_node - 2;
	ni[0].leader = sum_comm_sort_idx[0];
	threads_chosen[ni[0].leader] = cores_per_node - 1;

	for (i = 1; i < nnodes; ++i) {
		int j = 0;
		ni[i].core = cores_per_node * (i + 1) - 1;

		while (threads_chosen[sum_comm_sort_idx[j]] != -1
				|| check_neighbor(sum_comm_sort_idx[j], ni[i - 1].leader))
			j = (j + 1) % NTHREADS;

		ni[i].leader = sum_comm_sort_idx[j];
		threads_chosen[ni[i].leader] = ni[i].core;
		--ni[i].core;
	}
	
	for (i = 0; i < nnodes; ++i) {
		int curr = ni[i].leader, neighbor;

		while (ni[i].core >= i * cores_per_node) {
			int *idx = &spd_comm_sort_idx[curr][0], j = 0;

			while (threads_chosen[idx[j]] != -1)
				j = (j + 1) % NTHREADS;
			neighbor = idx[j];

			threads_chosen[neighbor] = ni[i].core;
			--ni[i].core;

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
	kfree(ni);

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_ERR "Goodbye world 1.\n");
}

