#define pr_fmt(fmt) "fb_monitor: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>

#include <linux/moduleparam.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/timer.h>

#include <linux/connector.h>
#include <linux/cn_proc.h>

#include <linux/delay.h>

static struct cb_id fb_mon_id = { CN_NETLINK_USERS + 3, 0x456 };
static char fb_mon_name[] = "fb_monitor";
static struct sock *nls;
static struct timer_list fb_mon_timer;

static struct task_struct *thread_fbm;    /* fork bomb monitoring task */
static struct task_struct *thread_fbk;    /* fork bomb killing task */

/* Shared Data */
DEFINE_MUTEX(fb_lock);
unsigned long bomb_pid = 0;

static void fb_mon_callback(struct cn_msg *msg, struct netlink_skb_parms *nsp)
{
    pr_info("%s: %lu: idx=%x, val=%x, seq=%u, ack=%u, len=%d: %s.\n",
	        __func__, jiffies, msg->id.idx, msg->id.val,
	        msg->seq, msg->ack, msg->len,
	        msg->len ? (char *)msg->data : "");
}

static u32 fb_mon_timer_counter;
static void fb_mon_timer_func(unsigned long __data)
{
    struct cn_msg *m;
    char data[32];

    pr_debug("%s: timer fired with data %lu\n", __func__, __data);

    m = kzalloc(sizeof(*m) + sizeof(data), GFP_ATOMIC);
    if (m)
    {
        memcpy(&m->id, &fb_mon_id, sizeof(m->id));
        m->seq = fb_mon_timer_counter;
        m->len = sizeof(data);

        m->len =
            scnprintf(data, sizeof(data), "counter = %u",
                  fb_mon_timer_counter) + 1;

        memcpy(m + 1, data, m->len);

        cn_netlink_send(m, 0, 0, GFP_ATOMIC);
        kfree(m);
    }

    fb_mon_timer_counter++;

    mod_timer(&fb_mon_timer, jiffies + msecs_to_jiffies(1000));
}

// Monitor function kernel thread
static int fb_monitor(void *unused)
{
    while (!kthread_should_stop())
    {
        mutex_lock(&fb_lock);      /* acquire mutex lock */
        if (bomb_pid > 1)          /* 0 is root, 1 is init */
        {
            /* wait for fork bomb to be killed */
        }else{
            printk(KERN_INFO "Fork Bomb Monitor running\n");
            bomb_pid++;
            ssleep(5);
        }
        mutex_unlock(&fb_lock);    /* release mutex lock */
    }
    printk(KERN_INFO "Fork Bomb Monitor stopping\n");
    do_exit(0);
    return 0;
}
// Kill function kernel thread
static int fb_killer(void *unused)
{
    while (!kthread_should_stop())
    {
        mutex_lock(&fb_lock);      /* acquire mutex lock */
        if (bomb_pid < 2)          /* 0 is root, 1 is init */
        {
            /* wait for fork bomb to be detected */
        }else{
            printk(KERN_INFO "Killing Fork Bomb, PID: %lu\n", bomb_pid);
            bomb_pid = 0;      /* reset bomb_pid once the fork bomb is killed */
        }
        mutex_unlock(&fb_lock);    /* release mutex lock */
    }
    printk(KERN_INFO "Fork Bomb Killer stopping\n");
    do_exit(0);
    return 0;
}
// Module initialization
static int __init init_fbomb_defuser(void)
{
    int err;

    err = cn_add_callback(&fb_mon_id, fb_mon_name, fb_mon_callback);
    if (err)
        goto err_out;
    fb_mon_id.val++;
    err = cn_add_callback(&fb_mon_id, fb_mon_name, fb_mon_callback);
    if (err)
    {
        cn_del_callback(&fb_mon_id);
        goto err_out;
    }

    setup_timer(&fb_mon_timer, fb_mon_timer_func, 0);
    mod_timer(&fb_mon_timer, jiffies + msecs_to_jiffies(1000));

    pr_info("initialized with id={%u.%u}\n",
    fb_mon_id.idx, fb_mon_id.val);

    printk(KERN_INFO "Creating Threads\n");
    /* create thread to monitor for fork bombs */
    thread_fbm = kthread_run(fb_monitor, NULL, "forkbombmonitor");
    if (thread_fbm)
    {
        printk(KERN_INFO "Fork Bomb Monitor thread created successfully\n");
    }else{
        printk(KERN_INFO "Fork Bomb Monitor thread creation failed\n");
        goto thread_err_out;
    }
    /* create thread to kill identified fork bomb */
    thread_fbk = kthread_run(fb_killer, NULL, "forkbombkiller");
    if (thread_fbk){
        printk(KERN_INFO "Fork Bomb Killer thread created successfully\n");
    }else{
        printk(KERN_INFO "Fork Bomb Killer thread creation failed\n");
        goto thread_err_out;
    }
    return 0;

    err_out:
	    if (nls && nls->sk_socket)
		    sock_release(nls->sk_socket);
        return err;

    thread_err_out:
	    if (nls && nls->sk_socket)
		    sock_release(nls->sk_socket);
        return 1;
}

static void __exit exit_fbomb_defuser(void)
{
    del_timer_sync(&fb_mon_timer);
    cn_del_callback(&fb_mon_id);
    fb_mon_id.val--;
    cn_del_callback(&fb_mon_id);
    printk(KERN_INFO "Connection callback closed\n");
    if (nls && nls->sk_socket)
    {
        sock_release(nls->sk_socket);
        printk(KERN_INFO "Socket connection closed\n");
    }

    kthread_stop(thread_fbm);
    printk(KERN_INFO "Fork Bomb Monitor thread stopped\n");
    kthread_stop(thread_fbk);
    printk(KERN_INFO "Fork Bomb Killer thread stopped\n");
}

module_init(init_fbomb_defuser);
module_exit(exit_fbomb_defuser);

/* Module info */
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Adam Mastov, Christopher David Monken, Fan, Martin Kuna");
MODULE_DESCRIPTION("Project 2 part 2 CSE430 - Operating Systems, Spring 2016");

