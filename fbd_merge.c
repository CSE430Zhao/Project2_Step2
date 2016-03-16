#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/signal.h>
#include <linux/slab.h>

static struct task_struct *thread_fbm;    /* fork bomb monitoring task */
static struct task_struct *thread_fbk;    /* fork bomb killing task */
struct task_struct *task;

DEFINE_MUTEX(fb_lock);
char *target;    /* from test fb_defuser may need different */

class processList{
    public:
        char name[64];
        int pCount;
        struct list_head list;
};


struct processTable{
    char name[64];
    int pCount;
    struct list_head list;
};

struct processTable myTable;
struct processTable *tempProc;

bool checkName(char* name)
{
    struct list_head *pos;
    struct processTable *temp;
    list_for_each(pos, &(myTable.list))
    {
        temp = list_entry(pos, struct processTable, list);
        if (strcmp(temp->name, name) == 0)
        {
            return false;
        }
    }
	return true;
}

// Kill function - kernel thread
static int fb_killer(void *unused)
{
    while (!kthread_should_stop())
    {
        mutex_lock(&fb_lock);      /* acquire mutex lock, sleep otherise */
        if (target != NULL)        /* if target is identified */
        {
            for_each_process(task)
            {
                if (strcmp(task->comm, target) == 0)
                {
                    printk("fb_killer() eliminated: %s %d\n", task->comm,
                           task->pid);
                    send_sig(SIGKILL, task, 0);
                }
            }
            strcpy (target, "");   /* empty target after fork bomb killed */
        }else{
            /* wait for fork bomb to be detected */
        }
        mutex_unlock(&fb_lock);    /* release mutex lock */
    }
    printk(KERN_INFO "Fork Bomb Killer stopping\n");
    do_exit(0);
    return 0;
}

// Monitor function - kernel thread
static int fb_monitor(void *unused)
{
    struct list_head       *p;
    struct processTable    *temp;
    struct processTable    *ts;
    INIT_LIST_HEAD(&myTable.list);
    while (!kthread_should_stop())
    {
        mutex_lock(&fb_lock);           /* acquire mutex lock, sleep otherise */
        if (strcmp(target, "") == 0)    /* if no target identified */
//        if (target == NULL)             /* if no target identified */
        {
            for_each_process(task)
            {
                if (task->pid != 1)     /* not init */
                {
                    if (!checkName(task->comm))
                    {
                        list_for_each(p, &(myTable.list))
                        {
                            ts = list_entry(p, struct processTable, list);
                            if (strcmp(ts->name, task->comm) == 0)
                            {
                                ts->pCount++;
                            }
                        }
                    }else{
                        temp = kmalloc(sizeof(struct processTable), GFP_KERNEL);
                        strcpy(temp->name, task->comm);
                        temp->pCount = 1;
                        list_add(&(temp->list), &(myTable.list));
                    }
                }
            }
            list_for_each(p, &(myTable.list))
            {
                ts = list_entry(p, struct processTable, list);
                if (ts->pCount > 50)
                {
                    strcpy(target, ts->name);
                }
            }
            list_for_each(p, &(myTable.list))
            {
                ts = list_entry(p, struct processTable, list);
                ts->pCount = 0;
            }
        }else{
            /* wait for fork bomb to be killed */
        }
        mutex_unlock(&fb_lock);    /* release mutex lock */
    }
    printk(KERN_INFO "Fork Bomb Monitor stopping\n");
    do_exit(0);
    return 0;
}

// Module Initialization
static int __init init_fbd_merge(void)
{
    printk(KERN_INFO "Creating threads\n");
    /* create thread to monitor for fork bombs */
    thread_fbm = kthread_run(fb_monitor, NULL, "forkbombmonitor");
    if (thread_fbm)
    {
        printk(KERN_INFO "Fork Bomb Monitor thread created successfully\n");
    }else{
        printk(KERN_INFO "Fork Bomb Monitor thread creation failed\n");
//        return 0;
    }
    /* create thread to kill identified fork bomb */
    thread_fbk = kthread_run(fb_killer, NULL, "forkbombkiller");
    if (thread_fbk)
    {
        printk(KERN_INFO "Fork Bomb Killer thread created successfully\n");
    }else{
        printk(KERN_INFO "Fork Bomb Killer thread creation failed\n");
//        return 0;
    }
    return 0;
}

static void __exit exit_fbd_merge(void)
{
    printk(KERN_INFO "Cleaning Up\n");
    kthread_stop(thread_fbm);
    printk(KERN_INFO "Fork Bomb Monitor thread stopped\n");
    kthread_stop(thread_fbk);
    printk(KERN_INFO "Fork Bomb Killer thread stopped\n");
}

module_init(init_fbd_merge);
module_exit(exit_fbd_merge);

/* Module info */
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Adam Mastov, Christopher David Monken, Fan, Martin Kuna");
MODULE_DESCRIPTION("Project 2 part 2 CSE430 - Operating Systems, Spring 2016");

