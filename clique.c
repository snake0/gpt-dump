#include "clique.h"

struct process_info *process_list = NULL;
int alloc_processes = 0;
atomic_t nprocesses = ATOMIC_INIT(0);

int *cpu_state = NULL;
int threads_chosen[NTHREADS];
int num_nodes = 2, cliques_size, cores_per_node = 8;
static struct clique cliques[NTHREADS];

// communication rates between threads
int default_matrix[NTHREADS * NTHREADS] = {
    0,  12, 5,  3,  0,  1,  1,  1,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  1, 0, 0, 0, 1,  0,  0,  0,  0,  1,  0,  1,  4,  11,
    12, 0,  12, 2,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  1,  0,  1,  0,
    5,  12, 0,  12, 1,  4,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,
    3,  2,  12, 0,  5,  1,  1,  1,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  1,  0,  0,  1,
    0,  0,  1,  5,  0,  10, 0,  1,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  0,  4,  1,  10, 0,  15, 3,  1, 0,  2,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  0,  0,  1,  0,  15, 0,  18, 0, 1,  1,  0,  0,  1,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  0,  0,  1,  1,  3,  18, 0,  8, 3,  2,  2,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  1,  0,  8,  0, 6,  3,  2,  0,  0,  0, 1,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  1,  3,  6, 0,  12, 2,  1,  2,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  2,  1,  2,  3, 12, 0,  10, 1,  1,  0, 1,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  2,  2, 2,  10, 0,  8,  3,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 1,  1,  8,  0,  16, 1, 3,  3,  0,  1, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  1,  0,  0, 2,  1,  3,  16, 0,  6, 2,  3,  1,  0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  1,  6,  0, 8,  0,  0,  2, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  1, 0,  1,  0,  3,  2,  8, 0,  11, 1,  0, 0, 0, 1, 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  3,  3,  0, 11, 0,  12, 5, 1, 2, 0, 0,  0,  1,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  1,  0, 1,  12, 0,  9, 0, 1, 1, 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  1,  0,  2, 0,  5,  9,  0, 7, 5, 0, 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  1,  0,  7, 0, 7, 1, 0,  0,  3,  0,  0,  0,  1,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  2,  1,  5, 7, 0, 9, 1,  2,  2,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 1,  0,  1,  0, 1, 9, 0, 8,  0,  2,  0,  0,  0,  0,  0,  0,  0,
    1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 1,  0,  1,  1, 0, 1, 8, 0,  10, 3,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 2, 0, 10, 0,  14, 2,  0,  1,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  1,  0,  0, 3, 2, 2, 3,  14, 0,  12, 0,  1,  2,  2,  0,  2,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  2,  12, 0,  16, 2,  2,  2,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  16, 0,  5,  1,  1,  0,  0,
    1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  1,  1,  2,  5,  0,  10, 4,  4,  1,
    0,  1,  0,  1,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 1, 0, 0, 0,  0,  2,  2,  1,  10, 0,  10, 1,  1,
    1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  2,  2,  1,  4,  10, 0,  15, 2,
    4,  1,  1,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  0,  0,  0,  4,  1,  15, 0,  13,
    11, 0,  1,  1,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0, 0, 0, 0, 0,  0,  2,  0,  0,  1,  1,  2,  13, 0
};

void init_clique(void) {

}

void exit_clique(void) {
    
}

void init_process_list(void) {
    process_list = (struct process_info *) kmalloc(sizeof(struct process_info) * NPROCESS, GFP_KERNEL);
    alloc_processes = NPROCESS;
    C_ASSERT(process_list != NULL);
}

void destroy_process_list(void) {
    kfree(process_list);
    process_list = NULL;
    alloc_processes = 0;
}

#ifdef C_PRINT

static void print_matrix(int *k, int size) {
    int i, j;
    printk(KERN_ERR "======[ matrix ]======\n");
    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            printk(KERN_CONT "%2d ", k[i * size + j]);
        }
        printk(KERN_CONT "\n");
    }
    printk(KERN_ERR "======[ matrix ]======\n");
}

static void print_clique_sizes(void) {
    int i;
    printk(KERN_ERR "");
    for (i = 0; i < NTHREADS; ++i) {
        if (cliques[i].flag == C_VALID) {
            printk(KERN_CONT "%d ", cliques[i].size);
        }
    }
    printk(KERN_CONT"\n");
}

int print_clique(struct clique *c) {
    int j;
#ifdef VALID_ONLY
    if (c->flag == C_VALID) {
        printk(KERN_ERR "{");
        for (j = 0; j < c->size; ++j) {
            printk(KERN_CONT "%d ", c->pids[j]);
        }
        printk(KERN_CONT "}");
        return 0;
    }
    return -1;
#endif
    switch (c->flag) {
        case C_VALID:
            printk("VALID ");
            break;
        case C_REUSE:
            printk("REUSE ");
            break;
        case C_INVALID:
            printk("INVALID ");
            break;
    }
    printk("{");
    for (j = 0; j < c->size; ++j) {
        printk("%d ", c->pids[j]);
    }
    printk("}");
    return 0;
}

void print_cliques(void) {
    int i, r;
    printk(KERN_ERR "-------------------------------------------------------\n");
    for (i = 0; i < NTHREADS; ++i) {
        r = print_clique(cliques + i);
        if (r == 0) {
            printk(KERN_CONT"\n");
        }
    }
}

#endif // C_PRINT

int clique_distance(struct clique *c1, struct clique *c2, int *matrix, int size) {
    int ret = 0, i, j;
    if (c1 && c2) {
#ifndef C_USEMAX
        for (i = 0; i < c1->size; ++i) {
            for (j = 0; j < c2->size; ++j) {
                ret += matrix[c1->pids[i] * size + c2->pids[j]];
            }
        }
#else
        for (i = 0; i < c1->size; ++i) {
            for (j = 0; j < c2->size; ++j) {
                if (ret < matrix[c1->pids[i] * size + c2->pids[j]]) {
                    ret = matrix[c1->pids[i] * size + c2->pids[j]];
                }
            }
        }
#endif
    } else {
        printk("clique_distance: NULL pointer\n");
        ret = -1;
    }
    return ret;
}

void merge_clique(struct clique *c1, struct clique *c2) {
    if (c1 && c2) {
#ifdef C_PRINT
        printk("Merging: ");
        print_clique(c1);
        print_clique(c2);
        printk("\n");
#endif
        if (c1->flag != C_VALID) {
            if (c2->flag == C_VALID) {
                c2->flag = C_REUSE;
                cliques_size--;
            }
        } else {
            if (c2->flag != C_VALID) {
                c1->flag = C_REUSE;
                cliques_size--;
            } else {
                memcpy(c1->pids + c1->size, c2->pids, sizeof(int) * c2->size);
                c1->size = c1->size + c2->size;
                c1->flag = C_REUSE;
                c2->flag = C_INVALID;
                cliques_size -= 2;
            }
        }
    } else {
        printk("merge_clique: NULL pointer\n");
    }
}

struct clique *get_first_valid(void) {
    struct clique *ret = cliques;
    while (ret->flag != C_VALID) {
        ret++;
        if (ret == cliques + NTHREADS) {
            printk("get_first_valid: NO valid clique\n");
            return NULL;
        }
    }
    return ret;
}

struct clique *find_neighbor(struct clique *c1) {
    struct clique *c2, *temp = cliques;
    int distance = -1, temp_int;
    while (temp < cliques + NTHREADS) {
        if (temp != c1 && temp->flag == C_VALID) {
            temp_int = clique_distance(c1, temp, default_matrix, NTHREADS);
            if (temp_int > distance) {
                distance = temp_int;
                c2 = temp;
            }
        }
        temp++;
    }
    return c2;
}

void reset_cliques(void) {
    struct clique *temp = cliques;
    while (temp < cliques + NTHREADS) {
        if (temp->flag == C_REUSE) {
            temp->flag = C_VALID;
            ++cliques_size;
        }
        temp++;
    }
}

void init_cliques(void) {
    int i;
    for (i = 0; i < NTHREADS; ++i) {
        cliques[i].pids[0] = i;
        cliques[i].size = 1;
        cliques[i].flag = C_VALID;
    }
    cliques_size = NTHREADS;
}

void init_matrix(int *matrix) {
    int i, j;
    for (i = 0; i < NTHREADS; ++i) {
        for (j = 0; j < NTHREADS; ++j) {
            matrix[i * NTHREADS + j] = default_matrix[i * NTHREADS + j];
        }
    }
}

void assign_cpus_for_clique(struct clique *c, int node) {
    int cpu_curr = cores_per_node * node, size = c->size, *pids = c->pids, i;
    for (i = 0; i < size; ++i) {
        threads_chosen[pids[i]] = cpu_curr++;
        if (cpu_curr == (node + 1) * cores_per_node) {
            cpu_curr = cores_per_node * node;
        }
    }
}

void calculate_threads_chosen(void) {
    int i, node = 0;
    for (i = 0; i < NTHREADS; ++i) {
        if (cliques[i].flag == C_VALID) {
            assign_cpus_for_clique(cliques + i, node++);
        }
    }
#ifdef C_PRINT
    printk("Threads chosen:\n");
    for (i = 0; i < NTHREADS; ++i) {
        printk("%d -> %d\n", i, threads_chosen[i]);
    }
#endif
}

void clique_analysis(void) {
    struct clique *c1, *c2;
    init_cliques();
#ifdef C_PRINT
    print_matrix(default_matrix, NTHREADS);
    print_cliques();
#endif
    while (cliques_size > num_nodes) {
        while (cliques_size > 0) {
            c1 = get_first_valid();
            c2 = find_neighbor(c1);
            merge_clique(c1, c2);
        }
        reset_cliques();
#ifdef C_PRINT
        printk(KERN_ERR "cliques:");
        print_cliques();
        printk(KERN_ERR "cliques_size: %d, with ", cliques_size);
        print_clique_sizes();
#endif
    }
    calculate_threads_chosen();
}



int init_module(void) {
    return 0;
}

void cleanup_module(void) {}

MODULE_LICENSE("GPL");