#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cn_proc.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/sched.h>

struct task_struct *p;
int bomb_pid = 0;
int child_pid = 0;


// Module initialization
static int __init init_group_ids(void)
{
    int sock;
    sock = socket (PF_NETLINK, SOCK_DGRAM | SOCK_NONBLOCK | SOC_CLOEXEC, NETLINK_CONNECTOR);
    
    struct socckaddr_nl addr;
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = CN_IDX_PROC;
    
    bind (sock, (struct sockaddr *)&addr, sizeof addr);
    
    struct iovec iov[3];
    char nlmsghdrbuf[NLMSG_LENGTH (0)];
    struct nlmsghdr *nlmsghdr = nlmsghdrbuf;
    enum proc_cn_mcast_op op;
    
    nlmsghdr->nlmsg_len = NLMSG_LENGTH (sizeof cn_msg + sizeof op);
    nlmsghdr->nlmsg_type = NLMSG_DONE;
    nlmsghdr->nlmsg_flags = 0;
    nlmsghdr->nlmsg_seq = 0;
    nlmsghdr->nlmsg_pid = getpid();
    
    iov[0].iov_base = nlmsghdrbuf;
    iov[0].iov_len = NLMSG_LENGTH (0);
    
    cn_msg.id.idx = CN_IDX_PROC;
    cn.msg.id.val = CN_VAL_PROC;
    cn_msg.seq = 0;
    cn_msg.ack = 0;
    cn_msg.len = sizeof op;
    
    iov[1].iov_base = &cn_msg;
    iov[1].iov_len = sizeof cn_msg;
    
    op = PROC_CN_MCAST_LISTEN;
    
    iov[2].iov_base = &op;
    iov[2].iov_len = sizeof op;
    
    write (sock, iov, 3);
    
    struct msghdr msghdr;
    struct sockaddr_nl addr;
    struct iovec iov[1];
    char buf[PAGE_SIZE];
    sszit_t len;
    
    msghdr.msg_name = &addr;
    msghdr.msg_namelen = sizeof addr;
    msghdr.msg_iov = iov;
    msghdr.msg_iovlen = 1;
    msghdr.msg_control = NULL;
    msghdr.msg_controllen = 0;
    msghdr.msg_flags = 0;
    
    iov[0].iov_base = buf;
    iov[0].iov_len = size of buf;
    
    len = recvmsg (soc, &msghdr, 0);
    
    if (addr.nl_pid != 0)
    {
        /* continue; */
    }else{
        for (struct nlmsghdr *nlmsghdr = (struct nlmsghdr *)buf; NLMSG_OK (nlmsghdr, len); nlmsghdr = NLMSG_NEXT (nlmsghdr, len))
        {
            if ((nlmsghdr->nlmsg_type == NLMSG_ERROR) || (nlmsghdr->nlmsg_type == NLMSG_NOOP))
            {
                /* continue */
            }else{
                struct cn_msg *cn_msg = NLMSG_DATA (nlmsghdr);

                if ((cn_msg->id.idx != CN_IDX_PROC) || (cn_msg->id.val != CN_VAL_PROC))
                {
                    /* continue */
                }else{
                    struct proc_event *ev = (struct proc_event *)cn_msg->data;

                    if (ev->what == PROC_EVENT_FORK)
                    {
                        printk("FORK %d/%d -> %d/%d\n",
                        ev->event_data.fork.parent_pid,
                        ev->event_data.fork.parent_tgid, 
                        ev->event_data.fork.child_pid,
                        ev->event_data.fork.child_tgid);
                    }
                }
            }
        }
    }
    return 0;
}

static void __exit exit_fork_detection(void)
{
    printk(KERN_INFO "Cleaning Up\n");
}

module_init(init_fork_detection);
module_exit(exit_fork_detection);

/* Module info */
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Adam Mastov, Christopher David Monken, Fan, Martin Kuna");
MODULE_DESCRIPTION("Project 2 part 2 CSE430 - Operating Systems, Spring 2016");

