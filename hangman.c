#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/sched.h> /* tasklist_lock */

/* for delayed triggers */
static struct timer_list timer;

static int irqsave2;
module_param(irqsave2, int, 0);
MODULE_PARM_DESC(irqsave2, "hang via 2 x spin_lock_irqsave in init function");

static int irqsave;
module_param(irqsave, int, 0);
MODULE_PARM_DESC(irqsave, "hang via unbalanced spin_lock_irqsave in timer");

static int tasklist;
module_param(tasklist, int, 0);
MODULE_PARM_DESC(tasklist, "hang via unbalanced tasklist_lock");

static int proc_subdir;
module_param(proc_subdir, int, 0);
MODULE_PARM_DESC(proc_subdir, "hang via unbalanced proc_subdir_lock");

static void timer_func(unsigned long d);
static spinlock_t lck;
static unsigned long flags;

/* those pointer values need to be taken from System.map for now */
rwlock_t *my_tasklist_lock = (rwlock_t *)0xffffffff81a05040;
spinlock_t *my_proc_subdir_lock = (spinlock_t *)0xffffffff81cf7de0;

static int __init hangman_init(void)
{
	printk(KERN_INFO "%s...\n", __func__);
	if (irqsave2) {
		printk(KERN_INFO "taking spin_lock_irqsave twice...\n");
		spin_lock_init(&lck);
		spin_lock_irqsave(&lck, flags);
		spin_lock_irqsave(&lck, flags); /* instant deadlock */
	}
	if (irqsave) {
		/* start timer function */
		spin_lock_init(&lck);
		init_timer(&timer);
		timer.function = timer_func;
		timer.expires = jiffies + HZ;
		add_timer(&timer);
	}
	if (tasklist) {
		printk(KERN_INFO "taking tasklist_lock \n");
		read_lock(my_tasklist_lock);
	}
	if (proc_subdir) {
		printk(KERN_INFO "taking my_proc_subdir_lock\n");
		spin_lock(my_proc_subdir_lock);
	}
	printk(KERN_INFO "%s done.\n", __func__);

	return 0;
}

static void __exit hangman_exit(void)
{
	printk(KERN_INFO "hangman unloading\n");
	return;
}

module_init(hangman_init);
module_exit(hangman_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefan Seyfried");
MODULE_DESCRIPTION("This module hangs your kenrel in various ways");

static void timer_func(unsigned long d)
{
	printk(KERN_INFO "hangman: timer function\n");
	spin_lock_irqsave(&lck, flags);
	timer.expires = jiffies + HZ;
	add_timer(&timer);
}

