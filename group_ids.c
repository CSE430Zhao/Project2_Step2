#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>

int bomb_pid = 1175;
// Module initialization
struct task_struct *p;

static int __init init_group_ids(void)
{
    for_each_process(p)
    {
        if (p->pid == bomb_pid)
        {
            struct list_head *list;
            struct task_struct *task;
            list_for_each (list, &p->children)
            {
                task = list_entry(list, struct task_struct, children);
                printk(KERN_INFO "Parent PID is %d and Child PID is  %d\n", p->pid, task->pid);
            }
        }
    }
/*
struct list_head *p;
struct my_struct *my;

list_for_each(p, &mine->list) {
        * my points to the structure in which the list is embedded *
        my = list_entry(p, struct my_struct, list);
}
*/
/*    
        if (p->pid == 2881)
        {
            bomb_id = p->gid;
            printk(KERN_INFO "the Group ID is %d\n", bomb_id);
        }
    }
    for_each_process(task)
    {
        if (task->pgrp == bomb_id)
        {
            printk(KERN_INFO "%d\n",task->pid);
        }
    }
    */
    return 0;
}

static void __exit exit_group_ids(void)
{
    printk(KERN_INFO "Cleaning Up\n");
}

module_init(init_group_ids);
module_exit(exit_group_ids);

/* Module info */
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Adam Mastov, Christopher David Monken, Fan, Martin Kuna");
MODULE_DESCRIPTION("Project 2 part 2 CSE430 - Operating Systems, Spring 2016");

