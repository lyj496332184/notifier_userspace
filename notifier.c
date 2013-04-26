#include "notifier.h"

/*
 *	Notifier chain core routines.  The exported routines below
 *	are layered on top of these, with appropriate locking added.
 */

static int notifier_chain_register(struct notifier_block **nl,
		struct notifier_block *n)
{
	while ((*nl) != NULL) {
		if (n->priority > (*nl)->priority)
			break;
		nl = &((*nl)->next);
	}
	n->next = *nl;
	*nl = n;
	return 0;
}

static int notifier_chain_unregister(struct notifier_block **nl,
		struct notifier_block *n)
{
	while ((*nl) != NULL) {
		if ((*nl) == n) {
			*nl = n->next;
			return 0;
		}
		nl = &((*nl)->next);
	}
	return -1;
}

/**
 * notifier_call_chain - Informs the registered notifiers about an event.
 *	@nl:		Pointer to head of the blocking notifier chain
 *	@val:		Value passed unmodified to notifier function
 *	@v:		Pointer passed unmodified to notifier function
 *	@nr_to_call:	Number of notifier functions to be called. Don't care
 *			value of this parameter is -1.
 *	@nr_calls:	Records the number of notifications sent. Don't care
 *			value of this field is NULL.
 *	@returns:	notifier_call_chain returns the value returned by the
 *			last notifier function called.
 */
static int notifier_call_chain(struct notifier_block **nl,
					unsigned long val, void *v,
					int nr_to_call,	int *nr_calls)
{
	int ret = NOTIFY_DONE;
	struct notifier_block *nb, *next_nb;

	nb = *nl;

	while (nb && nr_to_call) {
		next_nb =nb->next;
		ret = nb->notifier_call(nb, val, v);

		if (nr_calls)
			(*nr_calls)++;

		if ((ret & NOTIFY_STOP_MASK) == NOTIFY_STOP_MASK)
			break;
		nb = next_nb;
		nr_to_call--;
	}
	return ret;
}

/*
 *	Blocking notifier chain routines.  All access to the chain is
 *	synchronized by an rwsem.
 */

/**
 *	blocking_notifier_chain_register - Add notifier to a blocking notifier chain
 *	@nh: Pointer to head of the blocking notifier chain
 *	@n: New entry in notifier chain
 *
 *	Adds a notifier to a blocking notifier chain.
 *	Must be called in process context.
 *
 *	Currently always returns zero.
 */
int blocking_notifier_chain_register(struct blocking_notifier_head *nh,
		struct notifier_block *n)
{
	int ret;

	pthread_mutex_lock(&nh->mutex);
	ret = notifier_chain_register(&nh->head, n);
	pthread_mutex_unlock(&nh->mutex);
	return ret;
}

/**
 *	blocking_notifier_chain_unregister - Remove notifier from a blocking notifier chain
 *	@nh: Pointer to head of the blocking notifier chain
 *	@n: Entry to remove from notifier chain
 *
 *	Removes a notifier from a blocking notifier chain.
 *	Must be called from process context.
 *
 *	Returns zero on success or %-ENOENT on failure.
 */
int blocking_notifier_chain_unregister(struct blocking_notifier_head *nh,
		struct notifier_block *n)
{
	int ret;

	pthread_mutex_lock(&nh->mutex);
	ret = notifier_chain_unregister(&nh->head, n);
	pthread_mutex_unlock(&nh->mutex);
	return ret;
}

/**
 *	__blocking_notifier_call_chain - Call functions in a blocking notifier chain
 *	@nh: Pointer to head of the blocking notifier chain
 *	@val: Value passed unmodified to notifier function
 *	@v: Pointer passed unmodified to notifier function
 *	@nr_to_call: See comment for notifier_call_chain.
 *	@nr_calls: See comment for notifier_call_chain.
 *
 *	Calls each function in a notifier chain in turn.  The functions
 *	run in a process context, so they are allowed to block.
 *
 *	If the return value of the notifier can be and'ed
 *	with %NOTIFY_STOP_MASK then blocking_notifier_call_chain()
 *	will return immediately, with the return value of
 *	the notifier function which halted execution.
 *	Otherwise the return value is the return value
 *	of the last notifier function called.
 */
int __blocking_notifier_call_chain(struct blocking_notifier_head *nh,
				   unsigned long val, void *v,
				   int nr_to_call, int *nr_calls)
{
	int ret = NOTIFY_DONE;

	/*
	 * We check the head outside the lock, but if this access is
	 * racy then it does not matter what the result of the test
	 * is, we re-check the list after having taken the lock anyway:
	 */
	if (nh->head) {
		pthread_mutex_lock(&nh->mutex);
		ret = notifier_call_chain(&nh->head, val, v, nr_to_call,
					nr_calls);
		pthread_mutex_unlock(&nh->mutex);
	}
	return ret;
}

int blocking_notifier_call_chain(struct blocking_notifier_head *nh,
		unsigned long val, void *v)
{
	return __blocking_notifier_call_chain(nh, val, v, -1, NULL);
}
