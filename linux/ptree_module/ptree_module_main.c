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
	n->level = level;
	list_add_tail(&(n->list), &bfs_list);
	return 0;
}


int ptree_implementation_2(struct prinfo *buf, int *nr, int pid)
{
	int level = 1, rc = 0, current_nr = 1;
	struct task_struct *p;
	struct task_struct *current_task;
	struct bfs_node *current_node;
	struct list_head *current_task_struct_index;

	p = pid_task(find_get_pid(pid), PIDTYPE_PID);
	insert_to_buf(buf, p, 0);

	rc = add_to_bfs_queue(p, level);
	if (rc != 0) {
		goto Exit;
	}

	current_node = list_first_entry_or_null(&bfs_list, struct bfs_node, list);

	while (current_nr < *nr && current_node != NULL) {
		pr_info("module: current_nr: %d/%d\n", current_nr, *nr);
		p = current_node->task;
		list_for_each(current_task_struct_index, &p->children) {

			current_task = list_entry(current_task_struct_index, struct task_struct, sibling);
			insert_to_buf(buf + current_nr, current_task, current_node->level);
			current_nr++;

			rc = add_to_bfs_queue(current_task, current_node->level);
			if (rc != 0) {
				goto Exit;
			}

		}
		list_del(&(current_node->list));
		kfree(current_node);
		level++;

		current_node = list_first_entry_or_null(&bfs_list, struct bfs_node, list);
	}

Exit:
	*nr = current_nr;
	return rc;
}

int ptree_implementation(struct prinfo *buf, int *nr, int pid)
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
	//*nr = i + 1;
	return 0;
}

static int __init ptree_module_init (void)
{
    int rc = 0;
    pr_info("ptree_module: module loaded\n");
    rc = register_ptree(&ptree_implementation_2);
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
    unregister_ptree(&ptree_implementation_2);
    pr_info("ptree_module: unregistered ptree function\n");
}

module_init (ptree_module_init);
module_exit (ptree_module_exit);
