#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/string.h>
#include <linux/sched/task.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/pagewalk.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <asm/current.h>
#include "../kernel/mapspages.h"

MODULE_DESCRIPTION("mine");
MODULE_LICENSE("GPL");

extern int register_mapspages(mapspages_func func);
extern void unregister_mapspages(mapspages_func func);

size_t insert_vma_record(char *buffer, struct vm_area_struct *current_vma, size_t max_size)
{
    char r, w, x, s;
    r = (current_vma->vm_flags & VM_READ) ? 'r' : '-';
    w = (current_vma->vm_flags & VM_WRITE) ? 'w' : '-';
    x = (current_vma->vm_flags & VM_EXEC) ? 'x' : '-';
    s = (current_vma->vm_flags & VM_SHARED) ? 's' : 'p';
    return snprintf(buffer,
                    max_size,
                    "%ld-%ld %c%c%c%c %ld %d:%d %ld\n",
                    current_vma->vm_start,
                    current_vma->vm_end,
                    r, w, x, s,
                    current_vma->vm_pgoff, 
                    MAJOR(current_vma->vm_file->f_inode->i_rdev),
                    MINOR(current_vma->vm_file->f_inode->i_rdev),
                    current_vma->vm_file->f_inode->i_ino);
}

int get_mapspages_full_walk(unsigned long start, unsigned long end, char *buf, size_t size, size_t *out_size)
{
    int ret = 0;
    size_t insert_size = 0;
    struct vm_area_struct *current_vma = NULL;

    ret = down_read_killable(&current->mm->mmap_sem);
    if (ret) {
        goto Exit;
    }

    current_vma = current->mm->mmap;
    while (current_vma != NULL) {
        if (start <= current_vma->vm_start && current_vma->vm_end <= end) {

            /* Determine if there is enough space in buff before trying to transfer */
            insert_size = insert_vma_record(NULL, current_vma, size - *out_size);
            if (*out_size + insert_size > size) {
                up_read(&current->mm->mmap_sem);
                goto Exit;
            }

            insert_vma_record(&(buf[*out_size]), current_vma, size - *out_size);
            buf[*out_size + insert_size-1] = '\n';
            *out_size += insert_size;
        }
        current_vma = current_vma->vm_next;
    }
    up_read(&current->mm->mmap_sem);

Exit:
    return 0;
}

int get_mapspages_full(unsigned long start, unsigned long end, char *buf, size_t size, size_t *out_size)
{
	pr_info("mapspages_module: In get_mapspages_full from syscall\n");
    pr_info("mapspages_module: start: %ld\n", start);
    pr_info("mapspages_module: end: %ld\n", end);
    pr_info("mapspages_module: size: %ld\n", size);
    *buf = '*';
    *out_size = 1;
    return 0;
}

static int __init mapspages_module_init (void)
{
    int rc = 0;
    pr_info("mapspages_module: module loaded\n");
    rc = register_mapspages(&get_mapspages_full_walk);
    if (rc != 0) {
	    pr_info("mapspages_module: Error in registered mapspages function\n");
	    goto Exit;
    }
    pr_info("mapspages_module: registered mapspages function successfully\n");

Exit:
    return 0;
}

static void __exit mapspages_module_exit (void)
{
    pr_info("mapspages_module: module unloaded\n");
    unregister_mapspages(&get_mapspages_full_walk);
    pr_info("mapspages_module: unregistered mapspages function\n");
}

module_init (mapspages_module_init);
module_exit (mapspages_module_exit);
