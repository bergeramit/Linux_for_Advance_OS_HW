#include "mapspages.h"

DEFINE_SPINLOCK(register_maps_lock);

mapspages_func get_mapspages_g = NULL;
EXPORT_SYMBOL(get_mapspages_g);

int register_mapspages(mapspages_func func)
{
	int rc = 0;
	spin_lock(&register_maps_lock);
	if (get_mapspages_g == NULL) {
		get_mapspages_g = func;
		goto Exit;
	}
	rc = -EBUSY;

Exit:
	spin_unlock(&register_maps_lock);
	return rc;
}
EXPORT_SYMBOL(register_mapspages);

void unregister_mapspages(mapspages_func func)
{
	spin_lock(&register_maps_lock);
	if (get_mapspages_g == func) {
		get_mapspages_g = NULL;
	}
	spin_unlock(&register_maps_lock);
}
EXPORT_SYMBOL(unregister_mapspages);


SYSCALL_DEFINE4(mapspages, unsigned long, start, unsigned long, end, char __user *, buf, size_t, size)
{
	long rc = 0;
	size_t out_size = 0;
	size_t bytes_not_copied = 0;
	char *_buf = NULL;

	if (buf == NULL || size == NULL) {
		rc = -EINVAL;
		goto Exit;
	}

	pr_info("syscall: entered syscall mapspages\n");
	if (get_mapspages_g == NULL) {
		rc = request_module("mapspages_module");
		if (rc != 0) {
			pr_crit("syscall: Failed to request_module\n");
			rc = -ENOSYS;
			goto Exit;
		}
	}

	_buf = (char *)kmalloc(size * sizeof(char), GFP_KERNEL);
	if (_buf == NULL) {
		pr_crit("syscall: Failed to kmalloc\n");
		rc = -EFAULT;
		goto Exit;
	}

	if (get_mapspages_g != NULL) {
		pr_info("syscall: Calling get_mapspages_g \n");
		rc = get_mapspages_g(start, end, _buf, size, &out_size);
		if (rc != 0) {
			goto Exit;
		}
		pr_info("syscall: Copied: %d entries\n", out_size);
	} else {
		pr_crit("syscall: get_mapspages_g is still NULL...\n");
		rc = -ENOSYS;
		goto Exit;
	}

	pr_info("syscall: get_mapspages run successfully!\n");
	bytes_not_copied = copy_to_user(buf, _buf, size * sizeof(char));
	if (bytes_not_copied > 0) {
		pr_crit("syscall: Failed to copy_to_user buf, left to copy: %d\n", bytes_not_copied);
		rc = -EFAULT;
		goto Exit;
	}

	pr_info("syscall: Finished copy to user, exiting...\n");

Exit:
	return out_size;
}