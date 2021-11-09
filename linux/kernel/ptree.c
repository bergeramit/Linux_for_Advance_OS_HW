#include "ptree.h"

ptree_func get_ptree_g = NULL;
EXPORT_SYMBOL(get_ptree_g);

int register_ptree(ptree_func func)
{
	if (get_ptree_g == NULL) {
		get_ptree_g = func;
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


SYSCALL_DEFINE3(ptree, struct prinfo __user *, buf, int __user *, nr, int , pid)
{
	long rc = 0;
	int _nr = 0;
	int bytes_not_copied = 0;
	struct prinfo *_buf = NULL;

	if (buf == NULL || nr == NULL) {
		rc = -EINVAL;
		goto Exit;
	}

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
	bytes_not_copied = copy_from_user(&_nr, nr, sizeof(int));
	if (bytes_not_copied > 0) {
		pr_crit("Failed to copy_from_user nr. left to copy: %d\n", bytes_not_copied);
		rc = -EFAULT;
		goto Exit;
	}

	_buf = (struct prinfo *)kmalloc(_nr * sizeof(struct prinfo), GFP_KERNEL);
	if (_buf == NULL) {
		pr_crit("syscall: Failed to kmalloc\n");
		rc = -EFAULT;
		goto Exit;
	}

	if (get_ptree_g != NULL) {
		pr_info("syscall: Calling get_ptree_g \n");
		rc = get_ptree_g(_buf, &_nr, pid);
		if (rc != 0) {
			goto Exit;
		}
		pr_info("syscall: Copied: %d entries\n", _nr);
	} else {
		pr_crit("syscall: get_ptree_g is still NULL...\n");
		rc = -ENOSYS;
		goto Exit;
	}

	pr_info("syscall: get_ptree run successfully!\n");
	bytes_not_copied = copy_to_user(buf, _buf, _nr * sizeof(struct prinfo));
	if (bytes_not_copied > 0) {
		pr_crit("syscall: Failed to copy_to_user buf, left to copy: %d\n", bytes_not_copied);
		rc = -EFAULT;
		goto Exit;
	}
	pr_info("syscall: trying to copy _nr to user\n");
	bytes_not_copied = copy_to_user(nr, &_nr, sizeof(int));
	if (bytes_not_copied > 0) {
		pr_crit("syscall: Failed to copy_to_user nr, left to copy: %d\n", bytes_not_copied);
		rc = -EFAULT;
		goto Exit;
	}
	pr_info("syscall: Finished copy to user, exiting...\n");

Exit:
	return rc;
}

