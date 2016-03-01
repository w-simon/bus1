#ifndef __BUS1_PEER_H
#define __BUS1_PEER_H

/*
 * Copyright (C) 2013-2016 Red Hat, Inc.
 *
 * bus1 is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 */

/**
 * Peers
 *
 * XXX
 */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/rcupdate.h>
#include <linux/rbtree.h>
#include <linux/rwsem.h>
#include <linux/seqlock.h>
#include <linux/wait.h>
#include "active.h"
#include "pool.h"
#include "queue.h"
#include "user.h"

struct bus1_domain;

/**
 * struct bus1_peer_info - peer specific runtime information
 * @lock:			data lock
 * @rcu:			rcu
 * @user:			object owner
 * @quota:			quota handling
 * @pool:			data pool
 * @queue:			message queue, rcu-accessible
 * @map_handles_by_id:		map of owned handles, by handle id
 * @map_handles_by_node:	map of owned handles, by node pointer
 * @seqcount:			sequence counter
 * @handle_ids:			handle ID allocator
 */
struct bus1_peer_info {
	union {
		struct mutex lock;
		struct rcu_head rcu;
	};
	struct bus1_user *user;
	struct bus1_user_quota quota;
	struct bus1_pool pool;
	struct bus1_queue queue;
	struct rb_root map_handles_by_id;
	struct rb_root map_handles_by_node;
	struct seqcount seqcount;
	u64 handle_ids;
};

#define bus1_peer_info_from_pool(_pool) \
	container_of((_pool), struct bus1_peer_info, pool)

/**
 * struct bus1_peer - peer handle
 * @rwlock:		runtime lock
 * @rcu:		rcu
 * @waitq:		peer wide wait queue
 * @active:		active references
 * @info:		underlying peer information
 * @link_domain:	link into domain
 */
struct bus1_peer {
	union {
		struct rw_semaphore rwlock;
		struct rcu_head rcu;
	};
	wait_queue_head_t waitq;
	struct bus1_active active;
	struct bus1_peer_info __rcu *info;
	struct list_head link_domain;
};

struct bus1_peer *bus1_peer_new(void);
struct bus1_peer *bus1_peer_free(struct bus1_peer *peer);
int bus1_peer_teardown(struct bus1_peer *peer, struct bus1_domain *domain);
void bus1_peer_teardown_domain(struct bus1_peer *peer,
			       struct bus1_domain *domain);
struct bus1_peer *bus1_peer_acquire(struct bus1_peer *peer);
struct bus1_peer *bus1_peer_release(struct bus1_peer *peer);
struct bus1_peer_info *bus1_peer_dereference(struct bus1_peer *peer);
void bus1_peer_wake(struct bus1_peer *peer);
int bus1_peer_ioctl(struct bus1_peer *peer,
		    struct bus1_domain *domain,
		    const struct file *file,
		    unsigned int cmd,
		    unsigned long arg);

#endif /* __BUS1_PEER_H */
