#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

MODULE_LICENSE("GPL");

static struct nf_hook_ops *nf_ops = NULL;
static struct nf_hook_ops *nf_in = NULL;
static struct nf_hook_ops *nf_out = NULL;
static struct nf_hook_ops nf_post_rout;

static unsigned int type = 0;
static unsigned int imcp_cnt = 0;
static int udp_cnt = 0;

unsigned int block_port[] = {67, 6767};

int udp_black(int port){
    int i;
    pr_info("Requesting port: %u\n", port);

    for (i =0; i < ARRAY_SIZE(block_port); i++){

        if (port == block_port[i]) {
            pr_info("Black listed port: %u\n", block_port[i]);
            return 1;
        }
    }
    return 0;
}

static unsigned int hook_route(unsigned int hooknum,
                              struct sk_buff *skb,
                              const struct net_device *in,
                              const struct net_device *out,
                              int (*okfn)(struct sk_buff *))
                              {
    struct iphdr *idr;
    unsigned int check=0;
    idr = ip_hdr(skb);
    pr_info("----------------------------------\n");
    pr_info("(*) Packet received!\n");
    pr_info("(*) This is hook_route!!!\n");

    if (!idr){
        pr_info("Invalid ip_v4 header\n");
    }

    // for (int i =0, i < ARRAY_SIZE(block_port), i++) {
    //     if (idr->dest == block_port[i]){
    //         pr_info("Black listed port: %u\n", block_port[i]);
    //         return NF_DROP;
    //     }
    // }

    pr_info("(*) Protocol (%u) = ", idr->protocol);
    switch (idr->protocol)
    {
    case IPPROTO_TCP:
        pr_info("TCP!\n");
        pr_info("Sorry, this is in development! :((( \n");
        pr_info("Please try ICMP and UDP\n");
        return NF_DROP;
    
    case IPPROTO_ICMP:
        pr_info("IMCP!\n");
        break;
    
    case IPPROTO_UDP:
        pr_info("UDP!\n");
        if (udp_cnt >= 10){
            pr_info("Maxed UDP packet sent: 10\n");
            pr_info("----------------------------------\n");

            return NF_DROP;
        }
        struct udphdr *udph; 
        struct udphdr *test = udp_hdr(skb);
        // Result in wrong offset because hdp_hdr is wrong in this kernel
        pr_info("Port get by func: %u\n", ntohs(test->dest)); 
        // udph = udp_hdr(skb);
        udph = (struct udphdr *)((unsigned char *)idr + (idr->ihl * 4));
        
        unsigned int port = ntohs(udph->dest);
        pr_info("src= %u \ndst= %u\n idr->idl= %u\n", ntohs(udph->source), ntohs(udph->dest), idr->ihl);
        check = udp_black(port);
        break;
        
    default:
        pr_info("UKNOWN!\n");
        pr_info("Please send only IMCP and UDP packets! :(( \n");
        return NF_DROP;
    }
    pr_info("----------------------------------\n");

    type = idr->protocol;

    if (check){
        pr_info("check: %d\n", check);
        return NF_DROP;
    }
    
    return NF_ACCEPT;
}

static unsigned int hook_in(unsigned int hooknum,
                              struct sk_buff *skb,
                              const struct net_device *in,
                              const struct net_device *out,
                              int (*okfn)(struct sk_buff *))
{
    pr_info("----------------------------------\n");
    pr_info("(*) This is hook_in function!\n");
    
    if (type == IPPROTO_UDP){
        udp_cnt++;
        pr_info("UDP packet count: %d\n", udp_cnt);
    }
    pr_info("----------------------------------\n");
    


    return NF_ACCEPT;
}   

static unsigned int hook_out(unsigned int hooknum,
                              struct sk_buff *skb,
                              const struct net_device *in,
                              const struct net_device *out,
                              int (*okfn)(struct sk_buff *))
{
    pr_info("----------------------------------\n");

    pr_info("(*) Encounter hook_out function!\n");
    if (type == IPPROTO_ICMP){
        if (imcp_cnt>=10){
            pr_info("Maxed IMCP packet sent: 10\n");
            pr_info("----------------------------------\n");

            return NF_DROP;
        }
        imcp_cnt++;
        pr_info("(*) IMCP packet cnt: %u\n", imcp_cnt);
        pr_info("----------------------------------\n");

    }
    return NF_ACCEPT;
}

static unsigned int hook_post_routing(unsigned int hooknum,
                              struct sk_buff *skb,
                              const struct net_device *in,
                              const struct net_device *out,
                              int (*okfn)(struct sk_buff *)){
    pr_info("----------------------------------\n");
    pr_info("Replied successfully!\n");
    pr_info("----------------------------------\n");

    return NF_ACCEPT;
}
/*
Hook constant:
NF_IP_PRE_ROUTING
NF_IP_LOCAL_IN
NF_IP_FORWARD
NF_IP_LOCAL_OUT
NF_IP_POST_ROUTING

NF options:
NF_DROP
NF_ACCEPT
NF_STOLEN
NF_QUEUE
NF_REPEAT : call self hook func again
*/

static int __init hook_init() {
    pr_info("Welcome to firewall built by netfiler APIs\n");
    pr_info("Waiting for your TCP packet\n...\n");
    nf_ops = (struct nf_hook_ops*)kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);
    nf_in = (struct nf_hook_ops*)kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);
    nf_out = (struct nf_hook_ops*)kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);
    if (!nf_ops || !nf_in || !nf_out){
        pr_info("Allocation failed!\n");
        return 1;
    }
    // Enable hook route
    nf_ops->hook = hook_route;
    nf_ops->hooknum = NF_INET_PRE_ROUTING;
    nf_ops->pf = PF_INET; // pf: protocol family
    nf_ops->priority = NF_IP_PRI_FIRST;
    nf_register_hook(nf_ops);

    // Enable hook in
    nf_in->hook = hook_in;
    nf_in->hooknum = NF_INET_LOCAL_IN;
    nf_in->pf = PF_INET;
    nf_in->priority = NF_IP_PRI_FIRST;
    nf_register_hook(nf_in);

    // Enable hook out
    nf_out->hook = hook_out;
    nf_out->hooknum = NF_INET_LOCAL_OUT;
    nf_out->pf = PF_INET;
    nf_out->priority = NF_IP_PRI_FIRST;
    nf_register_hook(nf_out);

    // Enable hook post routing
    nf_post_rout.hook = hook_post_routing;
    nf_post_rout.hooknum = NF_INET_POST_ROUTING;
    nf_post_rout.pf = PF_INET;
    nf_post_rout.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&nf_post_rout);
    return 0;
}   

static void __exit cleanup(){
    nf_unregister_hook(nf_ops);
    nf_unregister_hook(nf_in);
    nf_unregister_hook(nf_out);
    nf_unregister_hook(&nf_post_rout);

    kfree(nf_in);
    kfree(nf_ops);
    kfree(nf_out);

    nf_ops = NULL;
    nf_in = NULL;
    nf_out = NULL;

    pr_info("Module unloaded successfully! :(\n");   
}

module_init(hook_init);
module_exit(cleanup);

