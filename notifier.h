#ifndef _NOTIFIER_H
#define _NOTIFIER_H

#include <pthread.h>

struct notifier_block {
	int (*notifier_call)(struct notifier_block *, unsigned long, void *);
	struct notifier_block *next;
	int priority;
};

struct blocking_notifier_head {
	pthread_mutex_t mutex;
	struct notifier_block *head;
};

#define BLOCKING_INIT_NOTIFIER_HEAD(name) do {	\
		pthread_mutex_init(&(name)->mutex, NULL);	\
		(name)->head = NULL;		\
	} while (0)


#define BLOCKING_NOTIFIER_INIT(name) {				\
		.mutex = PTHREAD_MUTEX_INITIALIZER,	\
		.head = NULL }

#define BLOCKING_NOTIFIER_HEAD(name)				\
	struct blocking_notifier_head name =			\
		BLOCKING_NOTIFIER_INIT(name)

int blocking_notifier_chain_register(struct blocking_notifier_head *nh,
		struct notifier_block *nb);

int blocking_notifier_chain_cond_register(
		struct blocking_notifier_head *nh,
		struct notifier_block *nb);

int blocking_notifier_chain_unregister(struct blocking_notifier_head *nh,
		struct notifier_block *nb);
int blocking_notifier_call_chain(struct blocking_notifier_head *nh,
		unsigned long val, void *v);
int __blocking_notifier_call_chain(struct blocking_notifier_head *nh,
	unsigned long val, void *v, int nr_to_call, int *nr_calls);

#define NOTIFY_DONE		0x0000		/* Don't care */
#define NOTIFY_OK		0x0001		/* Suits me */
#define NOTIFY_STOP_MASK	0x8000		/* Don't call further */
#define NOTIFY_BAD		(NOTIFY_STOP_MASK|0x0002)
						/* Bad/Veto action */
/*
 * Clean way to return from the notifier and stop further calls.
 */
#define NOTIFY_STOP		(NOTIFY_OK|NOTIFY_STOP_MASK)

#endif /* _NOTIFIER_H */
