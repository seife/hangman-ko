#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>

int init_module(void)
{
	unsigned long flags;
	spinlock_t lck;
	spin_lock_init(&lck);
	spin_lock_irqsave(&lck, flags);
	printk(KERN_INFO "*hangman*...\n");
#if 0
	ssleep(10);
	spin_unlock_irqrestore(&lck, flags);
#endif
	spin_lock_irqsave(&lck, flags);
	printk(KERN_INFO "     *hangs*\n");

	return 0;
}

void cleanup_module(void)
{
	return;
}


