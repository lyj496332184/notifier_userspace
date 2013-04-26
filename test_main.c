#include "notifier.h"
#include <stdio.h>

static BLOCKING_NOTIFIER_HEAD(test_chain);

int register_test_chain_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&test_chain, nb);
}

int unregister_test_chain_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&test_chain, nb);
}

int call_test_chain_notifier(unsigned long event, void *data)
{
	return blocking_notifier_call_chain(&test_chain, event, data);
}

int test_notifier_func(struct notifier_block * nb, unsigned long event, void * data)
{
	printf("%s %d:event:%lu\n", __FUNCTION__, __LINE__, event);

	if (data)
	{
		printf("data:%s\n", (char *)data);
	}

	return NOTIFY_DONE;
}

int test_notifier_func2(struct notifier_block * nb, unsigned long event, void * data)
{
	printf("%s %d:event:%lu\n", __FUNCTION__, __LINE__, event);

	if (data)
	{
		printf("data:%s\n", (char *)data);
	}

	return NOTIFY_DONE;
}

static struct notifier_block test_notifier = {
	.notifier_call = test_notifier_func,
};

static struct notifier_block test_notifier2 = {
	.notifier_call = test_notifier_func2,
};

int main()
{
	register_test_chain_notifier(&test_notifier);
	register_test_chain_notifier(&test_notifier2);
	
	call_test_chain_notifier(2, NULL);
	call_test_chain_notifier(4, "123_test");

	return 0;
}
