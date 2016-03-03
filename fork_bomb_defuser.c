/*
 *  Adam Mastov, Fan, Christopher David Monken, Martin Kuna
 *  Project 2 part 2
 *  CSE430
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mutex.h>     // for mutual exclusion
#include <kthread.h>         // for kernel threads
#include <signal.h>          // for kill signal

static struct task_struct *monitor_struct;
static struct task_struct *kill_struct;
struct mutex my_mutex;
mutex_init(&my_mutex);

static int fb_monitor(void *unused)
{
        /* do work */
        while (true){
                /* acquire mutex lock */
                mutex_lock(&my_lock);

                /*** put the monitor for fork bomb code here ***/

                /* release mutex lock */
                mutex_unlock(&my_lock);
        }

        return 0;
}

static int fb_killer(void *unused)
{
        /* do work */
        while(true){
                /* acquire mutex lock */
                mutex_lock(&my_lock);

                /*** put the kill fork bomb and its children code here ***/

                /* release mutex lock */
                mutex_unlock(&my_lock);
        )

        return 0;
}

// initialize module
static int __init init_fork_bomb_defuser(void)
{
        monitor_struct = kthread_run(fb_monitor, NULL, "fork_bomb_monitor");
        kill_struct = kthread_run(fb_killer, NULL, "fork_bomb_killer");

	return 0;
}

// exit module
static void __exit exit_fork_bomb_defuser(void)
{
	/* Do nothing */
}

module_init(init_fork_bomb_defuser);
module_exit(exit_fork_bomb_defuser);

/* Module info */
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Project 2 part 2 for CSE430 - Operating Systems, Spring 2016");

