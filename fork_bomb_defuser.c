#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <sys/syscall.h>

static struct task_struct *thread_fbm;    /* fork bomb monitoring task */
static struct task_struct *thread_fbk;    /* fork bomb killing task */

/* Shared Data */
DEFINE_MUTEX(fb_lock);
unsigned long bomb_pid = 0;    /* from test fb_defuser may need different */
                         /* buffer for actual monitor to killer communication */

inline int detect_fb(){
	unsigned long cur_time = 0;
	int forks_per_sec = 0;
	struct task_struct *p;

	cur_time = jiffies;
	
	read_lock(&tasklist_lock);
	for_each_task(p){
		if(p->uid == current->uid){
			if((cur_time-(p->start_time)) <= 100){
				forks_per_sec++;
			}
	}
	if(forks_per_sec > 50){
		return current->uid;
	}
	return 0;
}

// Function executed by kernel thread
static int fb_monitor(void *unused)
{
    while (!kthread_should_stop())
    {
        mutex_lock(&fb_lock);      /* acquire mutex lock */
        if (bomb_pid < 2)          /* 0 is root, 1 is init */
        {
            /*********************************************************
             ***      put code to monitor for fork bombs here      ***
             *** use global variable bomb_pid to pass what to kill ***
             *********************************************************/
        }else{
            /* wait for fork bomb to be killed */
        }
        mutex_unlock(&fb_lock);    /* release mutex lock */
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
        mutex_lock(&fb_lock);      /* acquire mutex lock */
        if (bomb_pid > 1)          /* 0 is root, 1 is init */
        {
            /*******************************************
             *** put code to kill for fork bomb here ***
             *******************************************/
            bomb_pid = 0;      /* reset bomb_pid once the fork bomb is killed */
        }else{
            /* wait for fork bomb to be detected */
        }
        mutex_unlock(&fb_lock);    /* release mutex lock */
    }
    printk(KERN_INFO "Fork Bomb Killer stopping\n");
    do_exit(0);
    return 0;
}
// Module Initialization
static int __init init_fork_bomb_defuser(void)
{
    printk(KERN_INFO "Creating threads\n");
    /* create thread to monitor for fork bombs */
    thread_fbm = kthread_run(fb_monitor, NULL, "forkbombmonitor");
    if (thread_fbm)
    {
        printk(KERN_INFO "Fork Bomb Monitor thread created successfully\n");
    }else{
        printk(KERN_INFO "Fork Bomb Monitor thread creation failed\n");
    }
    /* create thread to kill identified fork bomb */
    thread_fbk = kthread_run(fb_killer, NULL, "forkbombkiller");
    if (thread_fbk)
    {
        printk(KERN_INFO "Fork Bomb Killer thread created successfully\n");
    }else{
        printk(KERN_INFO "Fork Bomb Killer thread creation failed\n");
    }
    return 0;
}

static void __exit exit_fork_bomb_defuser(void)
{
    printk(KERN_INFO "Cleaning Up\n");
    kthread_stop(thread_fbm);
    printk(KERN_INFO "Fork Bomb Monitor thread stopped\n");
    kthread_stop(thread_fbk);
    printk(KERN_INFO "Fork Bomb Killer thread stopped\n");
}

module_init(init_fork_bomb_defuser);
module_exit(exit_fork_bomb_defuser);

/* Module info */
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Adam Mastov, Christopher David Monken, Fan, Martin Kuna");
MODULE_DESCRIPTION("Project 2 part 2 CSE430 - Operating Systems, Spring 2016");

