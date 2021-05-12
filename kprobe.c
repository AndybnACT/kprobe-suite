/*
 * NOTE: This example is works on x86 and powerpc.
 * Here's a sample kernel module showing the use of kprobes to dump a
 * stack trace and selected registers when _do_fork() is called.
 *
 * For more information on theory of operation of kprobes, see
 * Documentation/kprobes.txt
 *
 * You will see the trace data in /var/log/messages and on the console
 * whenever _do_fork() is invoked to create a new process.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kprobes.h>
#include <linux/blkdev.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/percpu-refcount.h>
#include <asm-generic/atomic-long.h>

struct completion wait;

/*
 * fault_handler: this is called if an exception is generated for any
 * instruction within the pre- or post-handler, or when Kprobes
 * single-steps the probed instruction.
 */
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	printk(KERN_INFO "fault_handler: p->addr = 0x%p, trap #%dn",
		p->addr, trapnr);
	/* Return 0 because we don't handle the fault. */
	return 0;
}

#define DEFINE_PROBE(_sym) \
	int probe_##_sym(struct kprobe *p, struct pt_regs *regs);	\
	struct kprobe  _sym##_probe = {	\
		.symbol_name	= #_sym,		\
		.pre_handler	= probe_##_sym,	\
		.fault_handler	= handler_fault,	\
	};\
	u64 __attribute__((section (".my_probes"))) _sym##addr = (u64) &_sym##_probe;\
	int probe_##_sym(struct kprobe *p, struct pt_regs *regs)

DEFINE_PROBE(nvme_dev_disable)
{
	reinit_completion(&wait);
	complete(&wait);
	pr_warn("complete!!\n");
	return 0;
}

DEFINE_PROBE(nvme_scan_work)
{
	init_completion(&wait);
	enable_kprobe(&nvme_dev_disable_probe);
	pr_warn("enable dev_disable_probe, waiting\n");
	wait_for_completion(&wait);
	pr_warn("done, disable dev_disable_probe\n");
	disable_kprobe(&nvme_dev_disable_probe);
	return 0;
}

DEFINE_PROBE(nvme_reset_work)
{
	pr_alert("reseting \n");
	return 0;
}

static int nr_probes;
static struct kprobe **myprobes;

extern u32 __MYPROBE_SECTION_START, __MYPROBE_SECTION_END;
static int __init kprobe_init(void)
{
	int ret;

	nr_probes = (u32) ((void *) &__MYPROBE_SECTION_END -
			  (void *) &__MYPROBE_SECTION_START) / sizeof (void*);
	myprobes = (void *) &__MYPROBE_SECTION_START;

	for (size_t i = 0; i < nr_probes; i++)
		printk(KERN_INFO "kprobe: %s\n", myprobes[i]->symbol_name);

	ret = register_kprobes(myprobes, nr_probes);
	if (ret < 0) {
		printk(KERN_INFO "register_kprobe failed, returned %d\n", ret);
		return ret;
	}

	disable_kprobe(&nvme_dev_disable_probe);

	for (size_t i = 0; i < nr_probes; i++)
		printk(KERN_INFO "Planted kprobe at %px\n", (void*) myprobes[i]->addr);
	return 0;
}

static void __exit kprobe_exit(void)
{
	unregister_kprobes((struct kprobe **) myprobes, nr_probes);
	printk("module unloaded");
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");
