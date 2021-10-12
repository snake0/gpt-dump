#ifndef __KVM_X86_KTCP_H
#define __KVM_X86_KTCP_H
/*
 * Copyright (C) 2019, Trusted Cloud Group, Shanghai Jiao Tong University.
 *
 * Authors:
 *   Yubin Chen <binsschen@sjtu.edu.cn>
 *   Zhuocheng Ding <tcbbd@sjtu.edu.cn>
 *   Jin Zhang <jzhang3002@sjtu.edu.cn>
 *   Boshi Yu <201608ybs@sjtu.edu.cn>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <linux/kernel.h>
#include <net/sock.h>

#define SUCCESS 0
// How many requests can be buffered in the listening queue
#define DEFAULT_BACKLOG 16

#define SIZE_SHIFT 21UL
#define EVAL_ITER 2000

typedef struct tx_add {
	/* Nodes indicated by inv_copyset should be sent INV messages upon write
	 * fault. It's also used to transfer the complete copyset upon read fault. */
	uint16_t inv_copyset;
	/* Pages with different versions MAY have different data. */
	uint16_t version;
	/*
	 * (Hopefully) unique transcation id, which is used to eliminate the
	 * necessity of per-socket locks.
	 */
	uint16_t txid;
} tx_add_t;

struct tx_add;
typedef struct tx_add tx_add_t;
struct ktcp_cb;

typedef uint32_t extent_t;

int ktcp_send(struct ktcp_cb *cb, const char *buffer, size_t length,
		unsigned long flags, const tx_add_t *tx_add);

int ktcp_receive(struct ktcp_cb *cb, char *buffer, size_t length,  unsigned long flags,
		tx_add_t *tx_add);

int ktcp_connect(const char *host, const char *port, struct ktcp_cb **conn_cb);

int ktcp_listen(const char *host, const char *port, struct ktcp_cb **listen_cb);

int ktcp_accept(struct ktcp_cb *listen_cb, struct ktcp_cb **accept_cb, unsigned long flags);

int ktcp_release(struct ktcp_cb *conn_cb);

#endif /* __KVM_X86_KTCP_H */
