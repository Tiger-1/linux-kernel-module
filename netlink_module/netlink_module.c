#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <net/net_namespace.h>
#define NETLINK_TEST 17

MODULE_LICENSE("GPL");

static struct sock *socketptr = NULL;

static void nl_recv_msg(struct sk_buff *skb){
	struct nlmsgdhr *nlh = NULL;
	if(skb == NULL) {
		printk("skb is NULL \n");
		return ;
	}
	nlh = (struct nlmsgdhr *)skb->data;
	printk(KERN_INFO "%s: received netlink message payload: %s\n", __FUNCTION__, NLMSG_DATA(nlh));
}

struct netlink_kernel_cfg cfg = {
	.input = nl_recv_msg,
};

static int __init my_module_init(void) {
	printk(KERN_INFO "Initializing Netlink Socket");
	socketptr = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
	return 0;
}

static void __exit my_module_exit(void) {
	printk(KERN_INFO "Goodbye");
	sock_release(socketptr->sk_socket);
}

module_init(my_module_init);
module_exit(my_module_exit);
