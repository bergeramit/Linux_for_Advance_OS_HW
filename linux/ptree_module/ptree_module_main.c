#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sched/task.h>
#include <linux/export.h>
#include <linux/sched.h>
#include "../kernel/ptree.h"

MODULE_DESCRIPTION("mine");
MODULE_LICENSE("GPL");

extern int register_ptree(ptree_func func);
extern void unregister_ptree(ptree_func func);
extern struct task_struct *find_task_by_vpid(pid_t nr);

struct bfs_node {
	struct list_head list;
	struct task_struct *task;
	int level;
};

struct list_head bfs_list;

LIST_HEAD(bfs_list);

void insert_to_buf(struct prinfo *buf, struct task_struct *p, int level)
{

	buf->parent_pid = p->real_parent->pid;
	buf->pid = p->pid;
	buf->state = p->state;
	buf->uid = p->cred->uid.val;
	strncpy(buf->comm, p->comm, 16);
	buf->level = level;
}

int add_to_bfs_queue(struct task_struct *task, int level)
{
	struct bfs_node *n = NULL;
	n = (struct bfs_node *)kmalloc(sizeof(struct bfs_node), GFP_KERNEL);
	if (n == NULL) {
		pr_crit("kmalloc failed\n");
		return -1;
	}
	n->task = task;
	n->level = level + 1;
	list_add_tail(&(n->list), &bfs_list);
	return 0;
}

void empty_bfs_queue(void) {
	struct bfs_node *current_node = NULL;

	current_node = list_first_entry_or_null(&bfs_list, struct bfs_node, list);
	while (current_node != NULL) {
		list_del(&(current_node->list));
		kfree(current_node);

		current_node = list_first_entry_or_null(&bfs_list, struct bfs_node, list);
	}
}

int get_ptree_full(struct prinfo *buf, int *nr, int pid)
{
	int rc = 0, current_nr = 0;
	struct task_struct *p = NULL;
	struct task_struct *current_task = NULL;
	struct bfs_node *current_node = NULL;
	struct list_head *current_task_struct_index;

	p = pid_task(find_get_pid(pid), PIDTYPE_PID);
	if (p == NULL) {
		pr_crit("module: Could not get task struct of pid: %d\n", pid);
		goto Exit;
	}
	insert_to_buf(buf, p, 0);
	current_nr++;

	rc = add_to_bfs_queue(p, -1);
	if (rc != 0) {
		goto Exit;
	}

	current_node = list_first_entry_or_null(&bfs_list, struct bfs_node, list);

	while (current_node != NULL) {
		p = current_node->task;
		rcu_read_lock();
		list_for_each(current_task_struct_index, &p->children) {

			if (current_nr >= *nr) {
				/* finished successfully */
				pr_info("module: retreived every requested entries(%d/%d)\n", current_nr, *nr);
				rc = 0;
				rcu_read_unlock();
				goto Exit;
			}

			current_task = list_entry(current_task_struct_index, struct task_struct, sibling);
			insert_to_buf(buf + current_nr, current_task, current_node->level + 1);
			current_nr++;

			rcu_read_unlock();
			rc = add_to_bfs_queue(current_task, current_node->level);
			if (rc != 0) {
				goto Exit;
			}
			rcu_read_lock();

		}
		list_del(&(current_node->list));
		kfree(current_node);

		current_node = list_first_entry_or_null(&bfs_list, struct bfs_node, list);
	}
	rcu_read_lock();
	pr_info("module: retreived (%d/%d), process does not have more\n", current_nr, *nr);

Exit:
	empty_bfs_queue();
	*nr = current_nr;
	return rc;
}

int get_ptree_dummy(struct prinfo *buf, int *nr, int pid)
{
	int i=0;
	for (i=0; i<*nr; i++) {
		(buf + i)->parent_pid = i;
		(buf + i)->pid = i+1;
		(buf + i)->state = 0;
		(buf + i)->uid = 0;
		strncpy((buf + i)->comm, "dummy", 6);
		(buf + i)->level = i;
	}
	*nr = i + 1;
	return 0;
}

static int __init ptree_module_init (void)
{
    int rc = 0;
    pr_info("ptree_module: module loaded\n");
    rc = register_ptree(&get_ptree_full);
    if (rc != 0) {
	    pr_info("ptree_module: Error in registered ptree function\n");
	    goto Exit;
    }
    pr_info("ptree_module: registered ptree function successfully\n");

Exit:
    return 0;
}

static void __exit ptree_module_exit (void)
{
    pr_info("ptree_module: module unloaded\n");
    unregister_ptree(&get_ptree_full);
    pr_info("ptree_module: unregistered ptree function\n");
}

module_init (ptree_module_init);
module_exit (ptree_module_exit);
