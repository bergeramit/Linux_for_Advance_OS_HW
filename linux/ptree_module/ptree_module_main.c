#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_DESCRIPTION("mine");
MODULE_LICENSE("Dual BSD/GPL - can be anything");

 
extern int register_ptree(ptree_func func);
extern void unregister_ptree(ptree_func func);

int ptree_implementation(struct prinfo *buf, int *nr, int pid)
{
	return 0;
}

static int ptree_module_init (void)
{
    int rc = 0;
    pr_info("ptree_module: module loaded\n");
    pr_info("ptree_module: calling register_ptree at: %p\n", register_ptree);
    rc = register_ptree(&ptree_implementation);
    if (rc != 0) {
	    pr_info("ptree_module: Error in registered ptree function\n");
	    goto Exit;
    }
    pr_info("ptree_module: registered ptree function successfully\n");

Exit:
    return 0;
}

static void ptree_module_exit (void)
{
    pr_info("ptree_module: module unloaded\n");
    unregister_ptree(&ptree_implementation);
    pr_info("ptree_module: unregistered ptree function\n");
}

module_init (ptree_module_init);
module_exit (ptree_module_exit);
