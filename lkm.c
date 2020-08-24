#include <linux/module.h>  //所有模块都必须包含的头文件
#include <linux/kernel.h> //一些宏定义，例如这里的KERN_INFO
#include <linux/sched.h>
#include <linux/cpu.h>

#define for_each_sibling(s, cpu) for_each_cpu(s, cpu_sibling_mask(cpu))
#define for_each_core(s, cpu) for_each_cpu(s, cpu_core_mask(cpu))
#define for_each_node_cpu(s, node) for_each_cpu(s, cpumask_of_node(node))

int num_nodes = 0, num_cpus = 0, num_cores = 0, num_threads = 0;
int pu[256];

void topo_init(void)
{
	int node, cpu, sibling, core;
	int index = 0, i;
	char seen[256] = {};

	printk("kmaf: detected hardware topology:\n");

	for_each_online_node(node) {
		printk("  node: %d", node);
		num_nodes++;

		for_each_node_cpu(cpu, node) {
			if (seen[cpu])
				continue;
			printk("    processor: %d", cpu);
			num_cpus++;

			for_each_core(core, cpu) {
				if (seen[core])
					continue;
				printk ("      core: %d", core);
				seen[core] = 1;
				num_cores++; num_threads++; pu[index++] = core;
				for_each_sibling(sibling, core) {
					if (seen[sibling])
						continue;
					seen[sibling] = 1;
					num_threads++;
					printk(", %d", sibling);
					pu[index++] = sibling;
				}
			}
		}
	}

	printk("PU: ");
	for (i=0; i<num_threads; i++) {
		printk("%d ", pu[i]);
	}

	num_threads /= num_cores;
	num_cores /= num_cpus;
	num_cpus /= num_nodes;

	printk("\nkmaf: %d nodes, %d processors per node, %d cores per processor, %d threads per core\n", num_nodes, num_cpus, num_cores, num_threads);
}

int init_module(void)
{
    printk(KERN_INFO "Hello world 1.\n");
    /*  
     * 返回非0表示模块初始化失败，无法载入
     */
    topo_init();
    return -1;
}

void cleanup_module(void)
{
    printk(KERN_INFO "Goodbye world 1.\n");
}

