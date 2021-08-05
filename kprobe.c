/*
 * NOTE: This example is works on x86
 * For more information on theory of operation of kprobes, see
 * Documentation/kprobes.txt
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kprobes.h>

#include <linux/fs.h>

#define KGDB_BREAK()  asm volatile ("int $3\r\n")

#define DEFINE_PROBE(_sym) \
	int probe_##_sym(struct kprobe *p, struct pt_regs *regs);	\
	struct kprobe  _sym##_probe = {	\
		.symbol_name	= #_sym,		\
		.pre_handler	= probe_##_sym,	\
	};\
	u64 __attribute__((section (".my_probes"))) _sym##addr = (u64) &_sym##_probe;\
	int probe_##_sym(struct kprobe *p, struct pt_regs *regs)

#define DEFINE_PROBE_AT(name, _sym, off) \
	int probe_##_sym##_##name(struct kprobe *p, struct pt_regs *regs);	\
	struct kprobe  name##_probe = {	\
		.symbol_name	= #_sym,		\
		.pre_handler= probe_##_sym##_##name,	\
		.offset		= off,			\
	};\
	u64 __attribute__((section (".my_probes"))) _sym##_##name = (u64) &name##_probe;\
	int probe_##_sym##_##name(struct kprobe *p, struct pt_regs *regs)

#define DEFINE_POST_PROBE_AT(_sym, off) \
	void probe_##_sym##off(struct kprobe *p, struct pt_regs *regs, unsigned long flags);	\
	struct kprobe  _sym##_probe##off = {	\
		.symbol_name	= #_sym,		\
		.post_handler= probe_##_sym##off,	\
		.offset		= off,			\
	};\
	u64 __attribute__((section (".my_probes"))) _sym##addr##off = (u64) &_sym##_probe;\
	void probe_##_sym##off(struct kprobe *p, struct pt_regs *regs, unsigned long flags)

/*
 * Define your kprobe is as simple as
 * DEFINE_PROBE(a_symbol_shown_on_kallsyms)
 * {
 * 	do_something_with_first_arg(regs->di);
 * 	...
 * 	return 0;
 * }
 *
 * You may define as many kprobe as you wish, play with variables and the
 * internal state of the kernel, and the magic of linker script will take care
 * of the rest of things.
 *
 */

DEFINE_PROBE(do_open_execat)
{
	const struct filename *filename  = (void *)regs->si;
	pr_info("execv: %s\n", filename->name);

	// KGDB_BREAK();

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

	pr_info("nr_probes = %d, myprobes = %llx\n", nr_probes, (u64)myprobes);
	for (size_t i = 0; i < nr_probes; i++)
		printk(KERN_INFO "kprobe: %s\n", myprobes[i]->symbol_name);

	ret = register_kprobes(myprobes, nr_probes);
	if (ret < 0) {
		printk(KERN_INFO "register_kprobe failed, returned %d\n", ret);
		return ret;
	}

//	disable_kprobe(&__nvme_submit_cmd_probe);

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

