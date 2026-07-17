#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Developer");
MODULE_DESCRIPTION("Basic Netfilter Hook API Example");
MODULE_VERSION("1.0");

// The structural definition for the hook operations
static struct nf_hook_ops my_nf_ops;

// Callback function executed whenever a packet hits the hook
static unsigned int my_hook_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *ip_header;

    if (!skb) return NF_ACCEPT;

    // Extract network layer IP header
    ip_header = ip_hdr(skb);
    if (!ip_header) return NF_ACCEPT;

    // Log the packet capture event to kernel ring buffer
    pr_info("Netfilter API: Captured packet from source IP: %pI4\n", &ip_header->saddr);

    // NF_ACCEPT passes packet along; return NF_DROP to block it instead
    return NF_ACCEPT; 
}

static int __init my_netfilter_init(void) {
    pr_info("Netfilter API: Initializing module...\n");

    my_nf_ops.hook = my_hook_fn;                 // Point to callback function
    my_nf_ops.pf = NFPROTO_IPV4;                 // Target IPv4 traffic
    my_nf_ops.hooknum = NF_INET_PRE_ROUTING;     // Intercept right after packet arrival
    my_nf_ops.priority = NF_IP_PRI_FIRST;         // Establish processing order priority

    // Register hook function into the active network infrastructure
    return nf_register_net_hook(&init_net, &my_nf_ops);
}

static void __exit my_netfilter_exit(void) {
    pr_info("Netfilter API: Cleaning up and exiting module...\n");
    // Safely unregister the hook
    nf_unregister_net_hook(&init_net, &my_nf_ops);
}

module_init(my_netfilter_init);
module_exit(my_netfilter_exit);
