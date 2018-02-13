#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

MODULE_DESCRIPTION("Probes module");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

/*
 * Pre-entry point for do_execveat_common.
 */
static int my_do_execveat_common(int fd, struct filename * filename,
				 char __user *__user *argv,
				 char __user *__user *envp,
				 int flags)
{
	pr_info("do_execveat_common for %s %s(%d) \n",
		filename->name, current->comm, current->pid);
	/* Always end with a call to jprobe_return(). */
	jprobe_return();
	/*NOTREACHED*/
	return 0;
}

static struct jprobe my_jprobe = {
	.entry = (kprobe_opcode_t *) my_do_execveat_common
};

static int my_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	/* TODO: print return value, parent process PID and process PID. */
	return 0;
}

static struct kretprobe my_kretprobe = {
	.handler = my_ret_handler,
};

static int my_probe_init(void)
{
	int ret;

	my_jprobe.kp.addr =
		(kprobe_opcode_t *) kallsyms_lookup_name("do_execveat_common");
	if (my_jprobe.kp.addr == NULL) {
		pr_info("Couldn't find %s to plant jprobe\n", "do_execveat_common");
		return -1;
	}

	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
		pr_info("register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	pr_info("Planted jprobe at %p, handler addr %p\n", my_jprobe.kp.addr,
		my_jprobe.entry);

	/* TODO: Find address of do_fork and register kretprobe. */

	return 0;
}

static void my_probe_exit(void)
{
	unregister_jprobe(&my_jprobe);
	pr_info("jprobe unregistered\n");
	/* TODO: Uregister kretprobe. */
}

module_init(my_probe_init);
module_exit(my_probe_exit);
