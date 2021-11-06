#ifndef _PTREE_H_
#define _PTREE_H_

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kmod.h>
#include <linux/printk.h>
#include <linux/ptrace.h>
#include <linux/syscalls.h>


struct prinfo {
        pid_t parent_pid;       /* process id of parent */
        pid_t pid;              /* process id */
        long state;             /* current state of process */
        uid_t uid;              /* user id of process owner */
        char comm[16];          /* name of program executed */
        int level;              /* level of this process in the subtree */
    };

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
int register_ptree(ptree_func func);
void unregister_ptree(ptree_func func);

#endif /* _PTREE_H_ */
