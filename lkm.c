#include <linux/module.h>  //所有模块都必须包含的头文件
#include <linux/kernel.h> //一些宏定义，例如这里的KERN_ERR
#include <linux/sched.h>
#include <linux/cpu.h>
#include <linux/sort.h>

int a[][4] = {{2,1,4,3},{2,1,4,3},{2,1,4,3},{2,1,4,3}};
int b[][4] = {{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3}};

int offset;

int cmp_func(const void *a, const void *b) {
	return *((int *) a - offset) - *((int *) b - offset);
}

static void spd_print_comm(int k[][4]) {
    int i, j;
#define NTHREADS 4
    printk(KERN_ERR
    "------[ matrix ]------\n");
    for (i = 0; i < NTHREADS; ++i) {
        for (j = 0; j < NTHREADS; ++j) {
            printk(KERN_CONT
            "%d ", k[i][j]);
        }
        printk(KERN_CONT
        "\n");
    }
    printk(KERN_ERR
    "======[ matrix ]======");
}

int init_module(void)
{
	int i;
	offset = &b[0][0] - &a[0][0];
	for (i = 0; i< 4;++i)
		sort(&a[0][0] + 4 * i, 4, sizeof(int), cmp_func, NULL);
    printk(KERN_ERR "Hello world 1.\n");
	spd_print_comm(a);
	spd_print_comm(b);

    /*  
     * 返回非0表示模块初始化失败，无法载入
     */
    //topo_init();
	
    return 0;
}

void cleanup_module(void)
{
    printk(KERN_ERR "Goodbye world 1.\n");
}

