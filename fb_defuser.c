/*
 *  Adam Mastov, Christopher David Monken, Fan, Martin Kuna
 *  Project 2 part 2
 *  CSE430
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>

static struct task_struct *thread_fbm;    /* fork bomb monitoring task */
static struct task_struct *thread_fbk;    /* fork bomb killing task */

/* Shared Data */
DEFINE_MUTEX(my_lock);
unsigned long bomb_pid = 0;

// Function executed by kernel thread
static int fb_monitor(void *unused)
{
    while (!kthread_should_stop())
    {
        mutex_lock(&my_lock);      /* acquire mutex lock */
        if (bomb_pid < 2)          /* 0 is root, 1 is init */
        {
            printk(KERN_INFO "Fork Bomb Monitor running\n");
            bomb_pid++;
            ssleep(5);
        }else{
            /* wait for fork bomb to be killed */
        }
        mutex_unlock(&my_lock);    /* release mutex lock */
    }
    printk(KERN_INFO "Fork Bomb Monitor stopping\n");
    do_exit(0);
    return 0;
}
// Function executed by kernel thread
static int fb_killer(void *unused)
{
    while (!kthread_should_stop())
    {
        mutex_lock(&my_lock);      /* acquire mutex lock */
        if (bomb_pid > 1)          /* 0 is root, 1 is init */
        {
            printk(KERN_INFO "Killing Fork Bomb, PID: %lu\n", bomb_pid);
            bomb_pid = 0;      /* reset bomb_pid once the fork bomb is killed */
        }else{
            /* wait for fork bomb to be detected */
        }
        mutex_unlock(&my_lock);    /* release mutex lock */
    }
    printk(KERN_INFO "Fork Bomb Killer stopping\n");
    do_exit(0);
    return 0;
}
// Module Initialization
static int __init init_fb_defuser(void)
{
        printk(KERN_INFO "Creating Threads\n");
    /* create thread to monitor for fork bombs */
//    thread_fbm = kthread_create(fb_monitor, NULL, "forkbombmonitor");
    thread_fbm = kthread_run(fb_monitor, NULL, "forkbombmonitor");
    if (thread_fbm)
    {
        printk(KERN_INFO "Fork Bomb Monitor thread created successfully\n");
//        wake_up_process(thread_fbm);
    }else{
        printk(KERN_INFO "Fork Bomb Monitor thread creation failed\n");
    }
    /* create thread to kill identified fork bomb */
//    thread_fbk = kthread_create(fb_killer, NULL, "forkbombkiller");
    thread_fbk = kthread_run(fb_killer, NULL, "forkbombkiller");
    if (thread_fbk){
        printk(KERN_INFO "Fork Bomb Killer thread created successfully\n");
//        wake_up_process(thread_fbk);
    }else{
        printk(KERN_INFO "Fork Bomb Killer thread creation failed\n");
    }
    return 0;
}

static void __exit exit_fb_defuser(void)
{
    printk(KERN_INFO "Cleaning Up\n");
    kthread_stop(thread_fbm);
    printk(KERN_INFO "Fork Bomb Monitor thread stopped\n");
    kthread_stop(thread_fbk);
    printk(KERN_INFO "Fork Bomb Killer thread stopped\n");
}

module_init(init_fb_defuser);
module_exit(exit_fb_defuser);

/* Module info */
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Adam Mastov, Christopher David Monken, Fan, Martin Kuna");
MODULE_DESCRIPTION("Project 2 part 2 CSE430 - Operating Systems, Spring 2016");

