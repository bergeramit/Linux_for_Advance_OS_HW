#ifndef _MAPSPAGES_H_
#define _MAPSPAGES_H_

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kmod.h>
#include <linux/printk.h>
#include <linux/ptrace.h>
#include <linux/spinlock.h>
#include <linux/syscalls.h>


typedef int (*mapspages_func)(unsigned long start, unsigned long end, char *buf, size_t size, size_t *out_size);
int register_mapspages(mapspages_func func);
void unregister_mapspages(mapspages_func func);

#endif /* _MAPSPAGES_H_ */
