#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h> // macro support
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include <linux/skbuff.h>

static int __init hello_init(void)
{
    pr_info("Hello, world!\n");
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("Goodbye, world!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");