#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>

#include <linux/kallsyms.h>

/* for delayed triggers */
static struct timer_list timer;

/* instant hard hang -> module_init does not return */
static int irqsave2;
module_param(irqsave2, int, 0);
MODULE_PARM_DESC(irqsave2, "hang via 2 x spin_lock_irqsave in init function");

/* hard hang after two seconds */
static int irqsave;
module_param(irqsave, int, 0);
MODULE_PARM_DESC(irqsave, "hang via unbalanced spin_lock_irqsave in timer");

/* hard hang *soon* */
static int tasklist;
module_param(tasklist, int, 0);
MODULE_PARM_DESC(tasklist, "hang via unbalanced tasklist_lock");

/* needed by many /proc operations -> hangs soon, but soft -> ping still works */
static int proc_subdir;
module_param(proc_subdir, int, 0);
MODULE_PARM_DESC(proc_subdir, "hang via unbalanced proc_subdir_lock");

static void timer_func(unsigned long d);
static spinlock_t lck;
static unsigned long flags;

static rwlock_t *my_tasklist_lock;
static spinlock_t *my_proc_subdir_lock;

static int __init hangman_init(void)
{
	printk(KERN_INFO "%s...\n", __func__);
	my_tasklist_lock = (rwlock_t *)kallsyms_lookup_name("tasklist_lock");
	my_proc_subdir_lock = (spinlock_t *)kallsyms_lookup_name("proc_subdir_lock");
	printk(KERN_INFO "%s: tasklist_lock:     0x%p\n", __func__, my_tasklist_lock);
	printk(KERN_INFO "%s: proc_subdir_lock:  0x%p\n", __func__, my_proc_subdir_lock);
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
MODULE_DESCRIPTION("This module hangs your kernel in various ways");

static void timer_func(unsigned long d)
{
	printk(KERN_INFO "hangman: timer function\n");
	spin_lock_irqsave(&lck, flags);
	timer.expires = jiffies + HZ;
	add_timer(&timer);
}

