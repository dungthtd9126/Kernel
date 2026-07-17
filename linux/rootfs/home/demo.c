#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

static struct nf_hook_ops nfho;

static unsigned int hook_func(unsigned int hooknum,
                              struct sk_buff *skb,
                              const struct net_device *in,
                              const struct net_device *out,
                              int (*okfn)(struct sk_buff *))
{
    pr_info("Packet received!\n");

    return NF_ACCEPT;
}

static int __init firewall_init(void)
{
    pr_info("Firewall module loaded\n");

    nfho.hook = hook_func;

    nfho.hooknum = NF_INET_PRE_ROUTING;

    nfho.pf = PF_INET;

    nfho.priority = NF_IP_PRI_FIRST;

    nf_register_hook(&nfho);

    return 0;
}

static void __exit firewall_exit(void)
{
    nf_unregister_hook(&nfho);

    pr_info("Firewall module unloaded\n");
}

module_init(firewall_init);
module_exit(firewall_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("saitomu");
MODULE_DESCRIPTION("Simple Netfilter Firewall");