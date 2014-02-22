/*
 * linux/kernel/irq/proc.c
 *
 * Copyright (C) 1992, 1998-2004 Linus Torvalds, Ingo Molnar
 *
 * This file contains the /proc/irq/ handling code.
 */

#include <linux/irq.h>
#include <linux/gfp.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>

#ifdef CONFIG_MSTAR_CHIP
#include <linux/poll.h>
#include <linux/sched.h>
#include "chip_int.h"
#endif  /* End of CONFIG_MSTAR_CHIP */

#include "internals.h"

static struct proc_dir_entry *root_irq_dir;

#ifdef CONFIG_SMP

static int show_irq_affinity(int type, struct seq_file *m, void *v)
{
	struct irq_desc *desc = irq_to_desc((long)m->private);
	const struct cpumask *mask = desc->irq_data.affinity;

#ifdef CONFIG_GENERIC_PENDING_IRQ
	if (irqd_is_setaffinity_pending(&desc->irq_data))
		mask = desc->pending_mask;
#endif
	if (type)
		seq_cpumask_list(m, mask);
	else
		seq_cpumask(m, mask);
	seq_putc(m, '\n');
	return 0;
}

static int irq_affinity_hint_proc_show(struct seq_file *m, void *v)
{
	struct irq_desc *desc = irq_to_desc((long)m->private);
	unsigned long flags;
	cpumask_var_t mask;

	if (!zalloc_cpumask_var(&mask, GFP_KERNEL))
		return -ENOMEM;

	raw_spin_lock_irqsave(&desc->lock, flags);
	if (desc->affinity_hint)
		cpumask_copy(mask, desc->affinity_hint);
	raw_spin_unlock_irqrestore(&desc->lock, flags);

	seq_cpumask(m, mask);
	seq_putc(m, '\n');
	free_cpumask_var(mask);

	return 0;
}

#ifndef is_affinity_mask_valid
#define is_affinity_mask_valid(val) 1
#endif

int no_irq_affinity;
static int irq_affinity_proc_show(struct seq_file *m, void *v)
{
	return show_irq_affinity(0, m, v);
}

static int irq_affinity_list_proc_show(struct seq_file *m, void *v)
{
	return show_irq_affinity(1, m, v);
}


static ssize_t write_irq_affinity(int type, struct file *file,
		const char __user *buffer, size_t count, loff_t *pos)
{
	unsigned int irq = (int)(long)PDE(file->f_path.dentry->d_inode)->data;
	cpumask_var_t new_value;
	int err;

	if (!irq_can_set_affinity(irq) || no_irq_affinity)
		return -EIO;

	if (!alloc_cpumask_var(&new_value, GFP_KERNEL))
		return -ENOMEM;

	if (type)
		err = cpumask_parselist_user(buffer, count, new_value);
	else
		err = cpumask_parse_user(buffer, count, new_value);
	if (err)
		goto free_cpumask;

	if (!is_affinity_mask_valid(new_value)) {
		err = -EINVAL;
		goto free_cpumask;
	}

	/*
	 * Do not allow disabling IRQs completely - it's a too easy
	 * way to make the system unusable accidentally :-) At least
	 * one online CPU still has to be targeted.
	 */
	if (!cpumask_intersects(new_value, cpu_online_mask)) {
		/* Special case for empty set - allow the architecture
		   code to set default SMP affinity. */
		err = irq_select_affinity_usr(irq, new_value) ? -EINVAL : count;
	} else {
		irq_set_affinity(irq, new_value);
		err = count;
	}

free_cpumask:
	free_cpumask_var(new_value);
	return err;
}

static ssize_t irq_affinity_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *pos)
{
	return write_irq_affinity(0, file, buffer, count, pos);
}

static ssize_t irq_affinity_list_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *pos)
{
	return write_irq_affinity(1, file, buffer, count, pos);
}

static int irq_affinity_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, irq_affinity_proc_show, PDE(inode)->data);
}

static int irq_affinity_list_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, irq_affinity_list_proc_show, PDE(inode)->data);
}

static int irq_affinity_hint_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, irq_affinity_hint_proc_show, PDE(inode)->data);
}

static const struct file_operations irq_affinity_proc_fops = {
	.open		= irq_affinity_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= irq_affinity_proc_write,
};

static const struct file_operations irq_affinity_hint_proc_fops = {
	.open		= irq_affinity_hint_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations irq_affinity_list_proc_fops = {
	.open		= irq_affinity_list_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= irq_affinity_list_proc_write,
};

static int default_affinity_show(struct seq_file *m, void *v)
{
	seq_cpumask(m, irq_default_affinity);
	seq_putc(m, '\n');
	return 0;
}

static ssize_t default_affinity_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	cpumask_var_t new_value;
	int err;

	if (!alloc_cpumask_var(&new_value, GFP_KERNEL))
		return -ENOMEM;

	err = cpumask_parse_user(buffer, count, new_value);
	if (err)
		goto out;

	if (!is_affinity_mask_valid(new_value)) {
		err = -EINVAL;
		goto out;
	}

	/*
	 * Do not allow disabling IRQs completely - it's a too easy
	 * way to make the system unusable accidentally :-) At least
	 * one online CPU still has to be targeted.
	 */
	if (!cpumask_intersects(new_value, cpu_online_mask)) {
		err = -EINVAL;
		goto out;
	}

	cpumask_copy(irq_default_affinity, new_value);
	err = count;

out:
	free_cpumask_var(new_value);
	return err;
}

static int default_affinity_open(struct inode *inode, struct file *file)
{
	return single_open(file, default_affinity_show, PDE(inode)->data);
}

static const struct file_operations default_affinity_proc_fops = {
	.open		= default_affinity_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= default_affinity_write,
};

static int irq_node_proc_show(struct seq_file *m, void *v)
{
	struct irq_desc *desc = irq_to_desc((long) m->private);

	seq_printf(m, "%d\n", desc->irq_data.node);
	return 0;
}

static int irq_node_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, irq_node_proc_show, PDE(inode)->data);
}

static const struct file_operations irq_node_proc_fops = {
	.open		= irq_node_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static int irq_spurious_proc_show(struct seq_file *m, void *v)
{
	struct irq_desc *desc = irq_to_desc((long) m->private);

	seq_printf(m, "count %u\n" "unhandled %u\n" "last_unhandled %u ms\n",
		   desc->irq_count, desc->irqs_unhandled,
		   jiffies_to_msecs(desc->last_unhandled));
	return 0;
}

static int irq_spurious_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, irq_spurious_proc_show, PDE(inode)->data);
}

static const struct file_operations irq_spurious_proc_fops = {
	.open		= irq_spurious_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#ifdef CONFIG_MSTAR_CHIP
struct irq_proc {
	int irq;
	wait_queue_head_t q;
	atomic_t count;
	char devname[TASK_COMM_LEN];
};

irqreturn_t irq_proc_irq_handler(int irq, void *vidp)
{
	struct irq_proc *idp = (struct irq_proc *)vidp;

	BUG_ON(idp->irq != irq);
	disable_irq_nosync(irq);
	atomic_inc(&idp->count);
	wake_up(&idp->q);
	return IRQ_HANDLED;
}

/*
 * Signal to userspace an interrupt has occured.
 * Note: no data is ever transferred to/from user space!
 */
ssize_t irq_proc_read(struct file *fp, char *bufp, size_t len, loff_t *where)
{
	struct irq_proc *ip = (struct irq_proc *)fp->private_data;
//	irq_desc_t *idp = irq_desc + ip->irq; 
	irq_desc_t *idp = irq_to_desc(ip->irq);
	int i;
	int err;

	DEFINE_WAIT(wait);

	if (len < sizeof(int))
		return -EINVAL;

	if ((i = atomic_read(&ip->count)) == 0) {
		if (idp->irq_data.state_use_accessors& IRQD_IRQ_DISABLED)
			enable_irq(ip->irq);
		if (fp->f_flags & O_NONBLOCK)
			return -EWOULDBLOCK;
	}

	while (i == 0) {
		prepare_to_wait(&ip->q, &wait, TASK_INTERRUPTIBLE);
		if ((i = atomic_read(&ip->count)) == 0)
			schedule();
		finish_wait(&ip->q, &wait);
		if (signal_pending(current))
		{
			return -ERESTARTSYS;
	    }
	}

	if ((err = copy_to_user(bufp, &i, sizeof i)))
		return err;
	*where += sizeof i;

	atomic_sub(i, &ip->count);
	return sizeof i;
}

ssize_t irq_proc_write(struct file *fp, const char *bufp, size_t len, loff_t *where)
{
	struct irq_proc *ip = (struct irq_proc *)fp->private_data;
	int enable;
	int err;

	if (len < sizeof(int))
		return -EINVAL;

	if ((err = copy_from_user(&enable, bufp, sizeof enable)))
		return err;

	if (enable)
	{
		// if irq has been enabled, does not need eable again 
		if((irq_to_desc(ip->irq)->irq_data.state_use_accessors& IRQD_IRQ_DISABLED)) {
			enable_irq(ip->irq);
		}
	}
	else
	{
	    disable_irq_nosync(ip->irq);
	}

	*where += sizeof enable;
	return sizeof enable;
}

int irq_proc_open(struct inode *inop, struct file *fp)
{
	struct irq_proc *ip;
	struct proc_dir_entry *ent = PDE(inop);
	int error;
	unsigned long  irqflags;

	ip = kmalloc(sizeof *ip, GFP_KERNEL);
	if (ip == NULL)
		return -ENOMEM;

	memset(ip, 0, sizeof(*ip));
	strcpy(ip->devname, current->comm);
	init_waitqueue_head(&ip->q);
	atomic_set(&ip->count, 0);
	ip->irq = (int)(unsigned long)ent->data;

	if(ip->irq == E_IRQ_DISP)
		irqflags = IRQF_SHARED;
	else
		irqflags = SA_INTERRUPT;

	if ((error = request_irq(ip->irq,
					irq_proc_irq_handler,
					irqflags,
					ip->devname,
					ip)) < 0) {
		kfree(ip);
		printk(KERN_EMERG"[%s][%d] %d\n", __FUNCTION__, __LINE__, error);
		return error;
	}
	fp->private_data = (void *)ip;
	if(!(irq_to_desc(ip->irq)->irq_data.state_use_accessors& IRQD_IRQ_DISABLED))
		disable_irq_nosync(ip->irq);
	return 0;
}

int irq_proc_release(struct inode *inop, struct file *fp)
{

	if(fp->private_data != NULL)
	{
	    struct irq_proc *ip = (struct irq_proc *)fp->private_data;
	    free_irq(ip->irq, ip);
	    kfree(ip);
	    fp->private_data = NULL;
	}
	return 0;
}

unsigned int irq_proc_poll(struct file *fp, struct poll_table_struct *wait)
{
    int i;
    struct irq_proc *ip = (struct irq_proc *)fp->private_data;

    poll_wait(fp, &ip->q, wait);

#if 0 //let user mode driver take interrupt enable responsibility
    /* if interrupts disabled and we don't have one to process */
    if (idp->status & IRQ_DISABLED && atomic_read(&ip->count) == 0)
        enable_irq(ip->irq);
#endif

    if ((i = atomic_read(&ip->count)) > 0)
    {
        //atomic_sub(i, &ip->count);//clear counter
        atomic_sub(1, &ip->count);//only take one of it
        return POLLIN | POLLRDNORM; /* readable */
    }
    return 0;
}

long irq_proc_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case 137://special command to free_irq immediately
        {
            struct irq_proc *ip = (struct irq_proc *)fp->private_data;
            free_irq(ip->irq, ip);
            kfree(ip);
            fp->private_data = NULL;
        }
            break;
        default:
            return -1;
    }
        return 0;
}

struct file_operations irq_proc_file_operations = {
	.read = irq_proc_read,
	.write = irq_proc_write,
	.open = irq_proc_open,
	.release = irq_proc_release,
	.poll = irq_proc_poll,
	.unlocked_ioctl= irq_proc_ioctl,  //Procfs ioctl handlers must use unlocked_ioctl.
};

#endif /* End of CONFIG_MSTAR_CHIP */

#define MAX_NAMELEN 128

static int name_unique(unsigned int irq, struct irqaction *new_action)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irqaction *action;
	unsigned long flags;
	int ret = 1;

	raw_spin_lock_irqsave(&desc->lock, flags);
	for (action = desc->action ; action; action = action->next) {
		if ((action != new_action) && action->name &&
				!strcmp(new_action->name, action->name)) {
			ret = 0;
			break;
		}
	}
	raw_spin_unlock_irqrestore(&desc->lock, flags);
	return ret;
}

void register_handler_proc(unsigned int irq, struct irqaction *action)
{
	char name [MAX_NAMELEN];
	struct irq_desc *desc = irq_to_desc(irq);

	if (!desc->dir || action->dir || !action->name ||
					!name_unique(irq, action))
		return;

	memset(name, 0, MAX_NAMELEN);
	snprintf(name, MAX_NAMELEN, "%s", action->name);

	/* create /proc/irq/1234/handler/ */
	action->dir = proc_mkdir(name, desc->dir);
}

#undef MAX_NAMELEN

#define MAX_NAMELEN 10

void register_irq_proc(unsigned int irq, struct irq_desc *desc)
{
	char name [MAX_NAMELEN];

#ifdef CONFIG_MSTAR_CHIP
	struct proc_dir_entry *entry;
#endif  /* End of CONFIG_MSTAR_CHIP */

	if (!root_irq_dir || (desc->irq_data.chip == &no_irq_chip) || desc->dir)
		return;

	memset(name, 0, MAX_NAMELEN);
	sprintf(name, "%d", irq);

	/* create /proc/irq/1234 */
	desc->dir = proc_mkdir(name, root_irq_dir);
	if (!desc->dir)
		return;

#ifdef CONFIG_SMP
	/* create /proc/irq/<irq>/smp_affinity */
	proc_create_data("smp_affinity", 0600, desc->dir,
			 &irq_affinity_proc_fops, (void *)(long)irq);

	/* create /proc/irq/<irq>/affinity_hint */
	proc_create_data("affinity_hint", 0400, desc->dir,
			 &irq_affinity_hint_proc_fops, (void *)(long)irq);

	/* create /proc/irq/<irq>/smp_affinity_list */
	proc_create_data("smp_affinity_list", 0600, desc->dir,
			 &irq_affinity_list_proc_fops, (void *)(long)irq);

	proc_create_data("node", 0444, desc->dir,
			 &irq_node_proc_fops, (void *)(long)irq);
#endif

	proc_create_data("spurious", 0444, desc->dir,
			 &irq_spurious_proc_fops, (void *)(long)irq);

#ifdef CONFIG_MSTAR_CHIP
	entry = create_proc_entry("irq", 0600, desc->dir);
	if (entry) {
		entry->data = (void *)(long)irq;
		entry->read_proc = NULL;
        entry->write_proc = NULL;
        entry->proc_fops = &irq_proc_file_operations;
	}
#endif  /* End of CONFIG_MSTAR_CHIP */
}

void unregister_irq_proc(unsigned int irq, struct irq_desc *desc)
{
	char name [MAX_NAMELEN];

	if (!root_irq_dir || !desc->dir)
		return;
#ifdef CONFIG_SMP
	remove_proc_entry("smp_affinity", desc->dir);
	remove_proc_entry("affinity_hint", desc->dir);
	remove_proc_entry("smp_affinity_list", desc->dir);
	remove_proc_entry("node", desc->dir);
#endif
	remove_proc_entry("spurious", desc->dir);

	memset(name, 0, MAX_NAMELEN);
	sprintf(name, "%u", irq);
	remove_proc_entry(name, root_irq_dir);
}

#undef MAX_NAMELEN

void unregister_handler_proc(unsigned int irq, struct irqaction *action)
{
	if (action->dir) {
		struct irq_desc *desc = irq_to_desc(irq);

		remove_proc_entry(action->dir->name, desc->dir);
	}
}

static void register_default_affinity_proc(void)
{
#ifdef CONFIG_SMP
	proc_create("irq/default_smp_affinity", 0600, NULL,
		    &default_affinity_proc_fops);
#endif
}

void init_irq_proc(void)
{
	unsigned int irq;
	struct irq_desc *desc;

	/* create /proc/irq */
	root_irq_dir = proc_mkdir("irq", NULL);
	if (!root_irq_dir)
		return;

	register_default_affinity_proc();

	/*
	 * Create entries for all existing IRQs.
	 */
	for_each_irq_desc(irq, desc) {
		if (!desc)
			continue;

		register_irq_proc(irq, desc);
	}
}

#ifdef CONFIG_GENERIC_IRQ_SHOW

int __weak arch_show_interrupts(struct seq_file *p, int prec)
{
	return 0;
}

#ifndef ACTUAL_NR_IRQS
# define ACTUAL_NR_IRQS nr_irqs
#endif

int show_interrupts(struct seq_file *p, void *v)
{
	static int prec;

	unsigned long flags, any_count = 0;
	int i = *(loff_t *) v, j;
	struct irqaction *action;
	struct irq_desc *desc;

	if (i > ACTUAL_NR_IRQS)
		return 0;

	if (i == ACTUAL_NR_IRQS)
		return arch_show_interrupts(p, prec);

	/* print header and calculate the width of the first column */
	if (i == 0) {
		for (prec = 3, j = 1000; prec < 10 && j <= nr_irqs; ++prec)
			j *= 10;

		seq_printf(p, "%*s", prec + 8, "");
		for_each_online_cpu(j)
			seq_printf(p, "CPU%-8d", j);
		seq_putc(p, '\n');
	}

	desc = irq_to_desc(i);
	if (!desc)
		return 0;

	raw_spin_lock_irqsave(&desc->lock, flags);
	for_each_online_cpu(j)
		any_count |= kstat_irqs_cpu(i, j);
	action = desc->action;
	if (!action && !any_count)
		goto out;

	seq_printf(p, "%*d: ", prec, i);
	for_each_online_cpu(j)
		seq_printf(p, "%10u ", kstat_irqs_cpu(i, j));

	if (desc->irq_data.chip) {
		if (desc->irq_data.chip->irq_print_chip)
			desc->irq_data.chip->irq_print_chip(&desc->irq_data, p);
		else if (desc->irq_data.chip->name)
			seq_printf(p, " %8s", desc->irq_data.chip->name);
		else
			seq_printf(p, " %8s", "-");
	} else {
		seq_printf(p, " %8s", "None");
	}
#ifdef CONFIG_GENERIC_IRQ_SHOW_LEVEL
	seq_printf(p, " %-8s", irqd_is_level_type(&desc->irq_data) ? "Level" : "Edge");
#endif
	if (desc->name)
		seq_printf(p, "-%-8s", desc->name);

	if (action) {
		seq_printf(p, "  %s", action->name);
		while ((action = action->next) != NULL)
			seq_printf(p, ", %s", action->name);
	}

#ifdef CONFIG_IRQ_TIME
	seq_printf(p, "  (max %d usec)", desc->runtime);
#endif
	seq_putc(p, '\n');
out:
	raw_spin_unlock_irqrestore(&desc->lock, flags);
	return 0;
}
#endif

#ifdef CONFIG_UNHANDLED_IRQ_TRACE_DEBUGGING
static DEFINE_RAW_SPINLOCK(vd_irq_lock);
static int vd_irq_trace[NR_IRQS]={0,};

void vd_irq_set(int irq)
{
	unsigned long flags;
	raw_spin_lock_irqsave(&vd_irq_lock, flags);
	vd_irq_trace[irq]++;
	raw_spin_unlock_irqrestore(&vd_irq_lock, flags);
}

void vd_irq_unset(int irq)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&vd_irq_lock, flags);
	vd_irq_trace[irq]--;
	raw_spin_unlock_irqrestore(&vd_irq_lock, flags);
}
void show_irq(void)
{
	static int prec;
	int i, j;
	struct irq_desc *desc=NULL;
	struct irqaction *action;
	unsigned long flags;


	printk( KERN_ALERT "=============================================================\n");
	printk( KERN_ALERT "NR_IRQS : %d\n", NR_IRQS);
	printk( KERN_ALERT "incomplete irq list....\n");
	printk( KERN_ALERT "-------------------------------------------------------------\n");
	for(i=0; i<NR_IRQS;i++)
	{
		if(vd_irq_trace[i])
		{
			printk( "[%3d] : %5d ", i, vd_irq_trace[i]);
			desc = irq_to_desc(i);
			if (desc)
			{
				action = desc->action;
				while(action)
				{
					printk( "%s ", action->name);
					action = action->next;
				}
				printk("\n");
			}
			else
				printk( "  => There is no desc!!\n");
		}
	}
	printk( KERN_ALERT "-------------------------------------------------------------\n");
	/* print header and calculate the width of the first column */
	for (prec = 3, j = 1000; prec < 10 && j <= nr_irqs; ++prec)
		j *= 10;

	printk("%*s", prec + 8, "");
	for_each_online_cpu(j)
		printk("CPU%-8d", j);
	printk("\n");

	for(i=0; i<nr_irqs;i++)	
	{
		desc = irq_to_desc(i);
		if (!desc)
			continue;

		raw_spin_lock_irqsave(&desc->lock, flags);
		action = desc->action;
		if (!action)
			goto out;

		printk("%*d: ", prec, i);
		for_each_online_cpu(j)
			printk("%10u ", kstat_irqs_cpu(i, j));

		if (desc->irq_data.chip) {
			if (desc->irq_data.chip->name)
				printk(" %8s", desc->irq_data.chip->name);
			else
				printk(" %8s", "-");
		} else {
			printk(" %8s", "None");
		}
#ifdef CONFIG_GENERIC_IRQ_SHOW_LEVEL
		printk(" %-8s", irqd_is_level_type(&desc->irq_data) ? "Level" : "Edge");
#endif
		if (desc->name)
			printk("-%-8s", desc->name);

		if (action) {
			printk("  %s", action->name);
			while ((action = action->next) != NULL)
				printk(", %s", action->name);
		}

		printk("\n");
out:
		raw_spin_unlock_irqrestore(&desc->lock, flags);
	}
}
#endif
