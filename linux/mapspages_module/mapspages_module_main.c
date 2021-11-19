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
#include <asm/page.h>
#include "../kernel/mapspages.h"

#define TO_DIGIT(x) (x + '0')

MODULE_DESCRIPTION("mine");
MODULE_LICENSE("GPL");

extern int register_mapspages(mapspages_func func);
extern void unregister_mapspages(mapspages_func func);

struct page_string_descriptor {
    char *string;
    size_t size;
};

int count_pte_entry(pte_t *pte, unsigned long addr, unsigned long next, struct mm_walk *walk) 
{
    size_t *counter = (size_t *)walk->private;
    (*counter)++;
    return 0;
}

int count_pte_hole(unsigned long addr, unsigned long next, struct mm_walk *walk)
{
    size_t *counter = (size_t *)walk->private;
    (*counter)++;
    return 0;
}

int handle_pte_entry(pte_t *pte, unsigned long addr, unsigned long next, struct mm_walk *walk) 
{
    struct page_string_descriptor *descriptor = (struct page_string_descriptor *)walk->private;
    struct page *current_page = NULL;
    current_page = pte_page(*pte);

    if (current_page->_refcount.counter > 9) {
        *(descriptor->string + descriptor->size) = 'x';
    } else if (current_page->_refcount.counter == 0) {
        *(descriptor->string + descriptor->size) = '.';
    } else {
        *(descriptor->string + descriptor->size) = TO_DIGIT(current_page->_refcount.counter);
    }

    descriptor->size += 1;
    return 0;
}

int handle_pte_hole(unsigned long addr, unsigned long next, struct mm_walk *walk)
{
    struct page_string_descriptor *descriptor = (struct page_string_descriptor *)walk->private;
    *(descriptor->string + descriptor->size) = '.';
    descriptor->size += 1;
    return 0;
}

size_t get_total_pages_in_vma(struct vm_area_struct *current_vma)
{
    size_t total_pages_in_vma = 0;
    struct mm_walk_ops counter_ops = {
        .pte_entry = count_pte_entry,
        .pte_hole = count_pte_hole
    };
    walk_page_vma(current_vma, &counter_ops, &total_pages_in_vma);
    return total_pages_in_vma;
}

size_t insert_vma_record(char *buffer, struct vm_area_struct *current_vma, size_t max_size)
{
    char r, w, x, s;
    size_t ret = 0;
    size_t total_pages_in_vma = 0;
    struct page_string_descriptor descriptor = {0};
    struct mm_walk_ops ops = {
        .pte_entry = handle_pte_entry,
        .pte_hole = handle_pte_hole
    };
    unsigned int major = (current_vma->vm_file) ? MAJOR(current_vma->vm_file->f_inode->i_rdev) : 0;
    unsigned int minor = (current_vma->vm_file) ? MINOR(current_vma->vm_file->f_inode->i_rdev) : 0;
    unsigned long inode = (current_vma->vm_file) ? current_vma->vm_file->f_inode->i_ino : 0;

    total_pages_in_vma = get_total_pages_in_vma(current_vma);
    r = (current_vma->vm_flags & VM_READ) ? 'r' : '-';
    w = (current_vma->vm_flags & VM_WRITE) ? 'w' : '-';
    x = (current_vma->vm_flags & VM_EXEC) ? 'x' : '-';
    s = (current_vma->vm_flags & VM_SHARED) ? 's' : 'p';

    /* Allocate string literal */
    up_read(&current->mm->mmap_sem);
    descriptor.string = (char *)kmalloc((total_pages_in_vma + 1) * sizeof(char), GFP_KERNEL);
    ret = down_read_killable(&current->mm->mmap_sem);
    if (ret) {
        goto Exit;
    }

    walk_page_vma(current_vma, &ops, &descriptor);
    
    /* Safety should be removed */
    *(descriptor.string + total_pages_in_vma) = 0;

    pr_info("mapspages_module: current VMA: %lx-%lx %c%c%c%c %lx %d:%d %lx %s\n",
            current_vma->vm_start,
            current_vma->vm_end,
            r, w, x, s,
            current_vma->vm_pgoff, 
            major,
            minor,
            inode,
            descriptor.string);

    return snprintf(buffer,
                    max_size,
                    "%lx-%lx %c%c%c%c %lx %d:%d %lx %s\n",
                    current_vma->vm_start,
                    current_vma->vm_end,
                    r, w, x, s,
                    current_vma->vm_pgoff, 
                    major,
                    minor,
                    inode,
                    descriptor.string);
                
Exit:
    return ret;
}

void print_args(unsigned long start, unsigned long end, size_t size)
{
    pr_info("mapspages_module: In get_mapspages_dummy from syscall\n");
    pr_info("mapspages_module: start: %lx\n", start);
    pr_info("mapspages_module: end: %lx\n", end);
    pr_info("mapspages_module: size: %ld\n", size);
}

void print_current_addresses_state(unsigned long start, unsigned long vm_start, unsigned long vm_end, unsigned long end)
{
    pr_info("mapspages_module: start: %lx\n", start);
    pr_info("mapspages_module: current_vma->vm_start: %lx\n", vm_start);
    pr_info("mapspages_module: current_vma->vm_end: %lx\n", vm_end);
    pr_info("mapspages_module: end: %lx\n", end);
}

int get_mapspages_full(unsigned long start, unsigned long end, char *buf, size_t size, size_t *out_size)
{
    int ret = 0;
    size_t insert_size = 0;
    struct vm_area_struct *current_vma = NULL;

    (void)print_args(start, end, size);
    ret = down_read_killable(&current->mm->mmap_sem);
    if (ret) {
        goto Exit;
    }

    current_vma = current->mm->mmap;
    while (current_vma != NULL) {
        // pr_info("mapspages_module: In while loop\n");
        // (void)print_current_addresses_state(start, current_vma->vm_start, current_vma->vm_end, end);
        if (start <= current_vma->vm_start && current_vma->vm_start <= end) {
            pr_info("mapspages_module: Passed if statement!\n");

            /* Determine if there is enough space in buff before trying to transfer */
            insert_size = insert_vma_record(0, current_vma, 0);
            if (*out_size + insert_size > size) {
                up_read(&current->mm->mmap_sem);
                goto Exit;
            }
            pr_info("mapspages_module: Successful found the inset_size = %ld!\n", insert_size);

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

int get_mapspages_dummy(unsigned long start, unsigned long end, char *buf, size_t size, size_t *out_size)
{
	(void)print_args(start, end, size);

    /* Dummy test */
    *buf = '*';
    *out_size = 1;
    return 0;
}

static int __init mapspages_module_init (void)
{
    int rc = 0;
    pr_info("mapspages_module: module loaded\n");
    rc = register_mapspages(&get_mapspages_full);
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
    unregister_mapspages(&get_mapspages_full);
    pr_info("mapspages_module: unregistered mapspages function\n");
}

module_init (mapspages_module_init);
module_exit (mapspages_module_exit);
