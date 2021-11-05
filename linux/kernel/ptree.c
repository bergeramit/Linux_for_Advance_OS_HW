#include "ptree.h"

ptree_func get_ptree_g = NULL;
EXPORT_SYMBOL(get_ptree_g);

int register_ptree(ptree_func func)
{
	if (get_ptree_g == NULL) {
		get_ptree_g = func;
		pr_info("syscall: (in register_ptree): Set get_ptree_g to %p\n", func);
		pr_info("syscall: (in register_ptree): get_ptree_g:  %p\n", get_ptree_g);
		pr_info("syscall: (in register_ptree): get_ptree_g's address:  %p\n", &get_ptree_g);
		return 0;

	}
	return -EBUSY;
}
EXPORT_SYMBOL(register_ptree);

void unregister_ptree(ptree_func func)
{
	if (get_ptree_g == func) {
		get_ptree_g = NULL;
	}
}
EXPORT_SYMBOL(unregister_ptree);

asmlinkage int sys_ptree(struct prinfo *buf, int *nr, int pid)
{
	int rc = -1;
	int _nr = 0;
	int bytes_copied = 0;
	struct prinfo *_buf = NULL;

	pr_info("syscall: entered syscall ptree\n");
	if (get_ptree_g == NULL) {
		rc = request_module("ptree_module");
		if (rc != 0) {
			pr_crit("syscall: Failed to request_module\n");
			rc = -ENOSYS;
			goto Exit;
		}
	}
	pr_info("syscall: request_module success\n");
	pr_info("syscall: (in main): get_ptree_g:  %p\n", get_ptree_g);
	pr_info("syscall: (in main): get_ptree_g's address:  %p\n", &get_ptree_g);
	bytes_copied = copy_from_user(&_nr, nr, sizeof(int));
	if (bytes_copied != sizeof(int)) {
		pr_crit("Failed to copy_from_user nr\n");
		rc = -ENOSYS;
		goto Exit;
	}

	_buf = (struct prinfo *)kmalloc(_nr * sizeof(struct prinfo), GFP_KERNEL);
	if (_buf == NULL) {
		pr_crit("syscall: Failed to kmalloc\n");
		rc = -ENOSYS;
		goto Exit;
	}

	if (get_ptree_g != NULL) {
		pr_info("syscall: Calling get_ptree_g \n");
		rc = get_ptree_g(_buf, &_nr, pid);
		if (rc != 0) {
			goto Exit;
		}
	} else {
		pr_crit("syscall: get_ptree_g is still NULL...\n");
		rc = -ENOSYS;
		goto Exit;
	}

	pr_info("syscall: Return success from get_ptree_g\n");
	bytes_copied = copy_to_user(buf, _buf, _nr * sizeof(struct prinfo));
	if (bytes_copied != (_nr * sizeof(struct prinfo))) {
		pr_crit("syscall: Failed to copy_to_user buf\n");
		rc = -ENOSYS;
		goto Exit;
	}

	bytes_copied = copy_to_user(nr, &_nr, sizeof(int));
	if (bytes_copied != sizeof(int)) {
		pr_crit("syscall: Failed to copy_to_user nr\n");
		rc = -ENOSYS;
		goto Exit;
	}
	pr_info("syscall: Finished copy to user, exiting\n");

Exit:
	return rc;
}

