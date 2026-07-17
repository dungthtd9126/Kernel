Find src to download kernel in this link:
```c
https://www.kernel.org/pub/linux/kernel/v2.6/
```

**base-devel: a group of development tools:
- gcc — C compiler.
- make — Runs the build process according to the Makefile.
- binutils — Provides tools like ld (linker), as (assembler), objdump, nm, and strip.
- patch — Applies source code patches.
- fakeroot — Simulates root ownership when creating packages.
- pkgconf — Helps locate compiler and linker flags for libraries.

**bc**: Command-line calculator. The kernel build system uses it to compute values (e.g., timing constants).

Install qemu with KVM for vm with acceleration:
```c
sudo pacman -S qemu-full libvirt virt-manager
sudo systemctl enable --now libvirtd
```

Thứ tự xài make:
```python
make ARCH=i386 defconfig
# modify config
make ARCH=i386 menuconfig
# get bzImage
# bzImage is a bootable kernel image
# it stand for big compressed kernel image
make ARCH=i386 bzImage
```

- Note that the kernel is too old so I need to do some patch

So I'll have to run patch with fixed.patch to needed file

- See more instruction in this link:

result after make bzImage:
```
Kernel: arch/x86/boot/bzImage is ready  (#1)
```

**initramfs (initial RAM filesystem) is a temporary root filesystem that the kernel loads into RAM during the earliest stage of booting.**

busybox contains multiple linux commands like: ls, cat, cp,...

When qemu boot
```
QEMU
    │
    ▼
bzImage
    │
    ▼
Linux kernel starts
    │
    ▼
Kernel extracts initramfs.gz into RAM
    │
    ▼
Looks for /init
    │
    ▼
Runs /init
    │
    ▼
/init executes BusyBox
    │
    ▼
BusyBox provides the shell and commands
```

bzImage stored in arch/x86/boot

Run this to create default file system:
```
mkdir -p \
bin \
sbin \
etc \
proc \
sys \
dev \
tmp \
usr/bin \
usr/sbin
```

Run this to copy all files and folders into **/src/**:
```
docker run --rm -it -v $(pwd):/src linux-2.6.26-env "<command>"
```

Use this to install busy box in chosen path:
```
./busybox --install -s <path>
or 
make CONFIG_PREFIX=<path> install
```

I installed preconfigured busybox ver 1.26.2:
```
https://busybox.net/downloads/binaries/1.26.2-i686/busybox
```

Create initramfs archive:
```c
find . | cpio -H newc -o | gzip > ../initramfs.gz
// Inspect archive
gzip -dc initramfs.gz | cpio -t
```

## boot with qemu
```
qemu-system-i386 \
    -kernel bzImage \
    -initrd initramfs.gz \
    -append "console=ttyS0" \
    -nographic
```

## init explain

mount attaches a filesystem to the directory tree so I can access its files.

**mount -t proc proc /proc**
```
mount
│
├── -t proc    ← filesystem type
├── proc       ← source (a virtual filesystem)
└── /proc      ← mount point
```
Many programs (including BusyBox applets) expect /proc to exist. 

So I need to mount it in order to see entries in /proc:
```
cpuinfo
meminfo
modules
interrupts
mounts
uptime
version
...
```
**mount -t sysfs sysfs /sys**: mounts another viriual filesystem

The sysfs filesystem exposes the kernel's device model:
```
/sys
├── block/
├── bus/
├── class/
├── devices/
├── firmware/
└── module/
```

## initramfs archive command explain
```
cpio -H newc -o
```
- cpio creates an archive
- -H newc: Use "newc" format (Linux kernels expect initramfs to be in cpio newc format)
- -o: output archive
--> get cpio archive (still uncompressed)

- gzip: compresses it
Failed to execute /init
Kernel panic - not syncing: No init found.  Try passing init= option to.

```
qemu-system-i386 \
    -kernel bzImage \
    -initrd initramfs.gz \
    -append "console=ttyS0 rdinit=/bin/sh" \
    -nographic   

--> RPC: Registered udp transport module.
RPC: Registered tcp transport module.
Using IPI No-Shortcut mode
input: ImExPS/2 Generic Explorer Mouse as /class/input/input1
Root-NFS: No NFS server available, giving up.
VFS: Unable to mount root fs via NFS, trying floppy.
VFS: Insert root floppy and press ENTER
```

I changed to busybox 1.18.5

Then I got some errors when trying to make busybox

The error are related to shell_common.c file

- fix:
```c
// Add this line to shell_common.c because it is missing that
#include <sys/resource.h>
```
Then I'll get a x86_64 busybox arch

But I want i386 one to be simpler

So I will install needed packages for compiling 32bit one
```c
apt update && apt install gcc-multilib libc6-dev-i386

// Then make busybox after menuconfig
make CC="gcc -m32"
```

If I want to build from no config then use this command:
```
make allnoconfig
```
Im currently using 1.18.5 busybox

**Because 1.18.5 too old so I disabled all network thing in it**

## init build
I used busybox so I need to write shebang using it syntax, because the kernel didn't have any command, just only one busybox file. 

I'll run busybox to make it build configured symlink first

Then run busybox mount to mount /proc (must)

If I don't do that step, the symlink will get error because /proc didn't have any contents, only vfs has

# Build kernel module
Run this to create and exec in container for developing changes
```
docker run -it -v $(pwd):/src bitinit/linux-2.6.26-env bash
```

If compile module manually, do this after exec into container
```c
cd linux-2.6.26
make ARCH=i386 oldconfig 
make -C . M=/src/rootfs/home modules
```

By somehow, the docker file system content connect with my host

So now my build.sh will be
```c
#!/bin/sh
# print each command before executing it
set -e

IMAGE="bitinit/linux-2.6.26-env"

docker run --rm \
    -v "$PWD":/src \
    bitinit/linux-2.6.26-env \
    'cd /src/linux-2.6.26 &&
     make -C . M=/src/rootfs/home modules'

echo "success"

cd rootfs/home

./run
```

## Build firewall

NF_INET_PRE_ROUTING: Gói tin vừa đến card mạng, trước khi quyết định đường đi.

NF_INET_LOCAL_IN: Gói tin đi vào hệ thống dành cho chính máy tính.

NF_INET_FORWARD: Gói tin đi qua máy tính (router) để tới đích khác.

NF_INET_LOCAL_OUT: Gói tin xuất phát từ hệ thống.

NF_INET_POST_ROUTING: Gói tin đã được định tuyến xong và chuẩn bị rời khỏi máy.

Linux 2.6.26 provides five IPv4 hook points:
- Netfilter packet flow:
```c
                   Incoming packet
                         │
                   NF_INET_PRE_ROUTING
                         │
              ┌──────────┴──────────┐
              │                     │
        Local machine          Forwarding
              │                     │
      NF_INET_LOCAL_IN     NF_INET_FORWARD
              │                     │
              ▼                     ▼
         User process         Forward packet
                                   │
                             NF_INET_POST_ROUTING

Local generated packet
        │
NF_INET_LOCAL_OUT
        │
NF_INET_POST_ROUTING
```

Example variable declaration:
```c
static short int myshort = 1;
static int myint = 420;
static long int mylong = 9999;
static char *mystring = "blah";
static int myintArray[2] = { -1, -1 };
static int arr_argc = 0;

// macro for exist only in init.data section
static int hello3_data __initdata = 3;
// The same with init macro function:
static int __init hello_3_init(void)
{
	printk(KERN_INFO "Hello, world %d\n", hello3_data);
	return 0;
}

// Execute init when insmod
module_init(hello_3_init);
// execute exit when rmmod
module_exit(hello_3_exit);


/* 
 * module_param(foo, int, 0000)
 * The first param is the parameters name
 * The second param is it's data type
 * The final argument is the permissions bits, 
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 */

module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(myshort, "A short integer");

/*
 * module_param_array(name, type, num, perm);
 * The first param is the parameter's (in this case the array's) name
 * The second param is the data type of the elements of the array
 * The third argument is a pointer to the variable that will store the number
 * of elements of the array initialized by the user at module loading time
 * The fourth argument is the permission bits
 */
module_param_array(myintArray, int, &arr_argc, 0000);
MODULE_PARM_DESC(myintArray, "An array of integers");

#include <asm/uaccess.h>	/* for put_user */
```
### module_param
module_param allows me to add parameters when insmod the module
- example:
```c
// src: 
static char *mystring = "blah";
module_param(mystring, charp, 0000);
MODULE_PARM_DESC(mystring, "A character string");

// run:
insmod hello.ko mystring="Kernel"

// Then 
mystring="Kernel"
```

**modprobe**: modprobe hello

- Finds the module in the standard module directories and automatically loads any modules it depends on before loading hello.

### Makefile only compile the same file name
Example: **obj-m += demo.o**

Then it only compiles demo.c, not hello.c

### kernel library explain
```c
// Contains everything related to kernel modules:
#include <linux/module.h>
// Contains general kernel facilities:
#include <linux/kernel.h>
/*
These tell the kernel:
initialization code is only used while loading
exit code is only used while unloading
*/
```

```#include <linux/init.h>```: Contains the generic Netfilter API.
```c
struct nf_hook_ops

NF_ACCEPT

NF_DROP

nf_register_hook()
*/
struct nf_hook_ops

NF_ACCEPT

NF_DROP

nf_register_hook()
```
```#include <linux/netfilter_ipv4.h>```: Contains IPv4-specific definitions such as
```c
NF_INET_PRE_ROUTING

NF_INET_LOCAL_IN

NF_INET_FORWARD

NF_INET_LOCAL_OUT

NF_INET_POST_ROUTING
```
Create a netfilter hook object: ```static struct nf_hook_ops nfho;```
```
nfho
+-------------------+
| hook      = ?     |
| hooknum   = ?     |
| pf         = ?    |
| priority   = ?    |
+-------------------+
```

# Writing a Netfilter module
## A custom netfilter hook function
```c
unsigned int nf_hookfn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);
```
The parameters are:

- priv – A pointer to private data that was passed when registering the hook function.
- skb – A pointer to the **network packet** as a sk_buff (**socket buffer**) structure.
- state – A pointer to a nf_hook_state structure that contains information about the hook point, such as the network protocol, the network interface, and the routing information.

The return value is one of the possible actions mentioned in the previous subsection
## Register hook function
### Modern kernel
To register a netfilter hook function, using nf_register_net_hook:
```c
int nf_register_net_hook(struct net *net, const struct nf_hook_ops *reg);

// Example nf_hook_ops 
// my_hook_fn: custom defined hook func
my_nf_ops.hook = my_hook_fn;                 // Point to callback function
my_nf_ops.pf = NFPROTO_IPV4;                 // Target IPv4 traffic
my_nf_ops.hooknum = NF_INET_PRE_ROUTING;     // Intercept right after packet arrival
my_nf_ops.priority = NF_IP_PRI_FIRST;         // Establish processing order priority


struct nf_hook_ops {
    /* User fills in from here down. */
    nf_hookfn *hook; // A pointer to the netfilter hook function
    struct net_device *dev;
    void *priv; // A pointer to private data that will be passed to the hook function.
    u_int8_t pf; // The protocol family of the packets to intercept, such as PF_INET for IPv4 or PF_INET6 for IPv6.
    unsigned int hooknum; // A pointer to the netfilter hook function.
    /* Hooks are ordered in ascending priority. */
    int priority; // The priority of the function within the same hook point. Lower values mean higher priority.
};
```
### 2.6.26 kernel
The custom hook function and **nf_register_net_hook** has different args number

Modern kernel uses less args for efficency reason, the old one used too much that it kept extend args number

Prototype of nf_register_hook (that's nf_register_net_hook's name in old version):
```c
int nf_register_hook(struct nf_hook_ops *reg);
```

## Generate local packet

```ping 8.8.8.8``` sends ICMP Echo Request packets to the IP address 8.8.8.8 and waits for ICMP Echo Reply packets.

The following occurs when you ```ping 8.8.8.8```:
```
ping command
      │
      ▼
Creates an ICMP Echo Request
      │
      ▼
Kernel networking stack
      │
      ▼
Netfilter hooks (your firewall module!)
      │
      ▼
Network interface
      │
      ▼
Internet
      │
      ▼
8.8.8.8
      │
      ▼
ICMP Echo Reply
      │
      ▼
Kernel
      │
      ▼
ping prints the result
```

My map to test module:
```
Host (Arch Linux)
        │
        │ ping / nc / curl
        ▼
QEMU virtual NIC
        ▼
Linux 2.6.26 Guest
        ▼
Netfilter hook (your module)
```

Since there is no physical motherboard, there is no physical network card.

NIC: Network Interface Card

So QEMU must pretend there is one.

That pretend card is called a virtual NIC.

- Where my firewall sit:
```
Internet

↓

Network Card

↓

Linux Network Stack

↓

PRE_ROUTING

↓

My hook / firewall
```

So new updated qemu command is:
```c
qemu-system-i386 \
    -kernel ../../bzImage \
    -initrd ../../initramfs.gz \
    -append "console=ttyS0" \
    -device e1000,netdev=n1 \
    -netdev user,id=n1 \
    -nographic
```
-net nic: create an e1000 intel virtual network card:
```
Guest

↓

Network Card
```

-net user: connects it to a tiny virtual network that QEMU creates:
```
Host

↓

Virtual Router

↓

Guest
```

Use this command to check if packets received / responsed:
```py
# cat /proc/net/dev
Inter-|   Receive                                                |  Trat
 face |bytes    packets errs drop fifo frame compressed multicast|bytesd
    lo:       0       0    0    0    0     0          0         0      0
  eth0:     134       1    0    0    0     0          0         0      0
  sit0:       0       0    0    0    0     0          0         0      0
# route -n
Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Usee
10.0.2.0        0.0.0.0         255.255.255.0   U     0      0        00
0.0.0.0         10.0.2.15       0.0.0.0         UG    0      0        00
```

Something should know:
- eth0 = first Ethernet interface
- eth1 = second Ethernet interface
- wlan0 = Wi-Fi interface (older naming)
- lo = loopback interface

### qemu command explain
```c
qemu-system-i386 \
    -kernel ../../bzImage \
    -initrd ../../initramfs.gz \
    -append "console=ttyS0" \
    -device e1000,netdev=n1 \
    -netdev user,id=n1,hostfwd=tcp::3636-:36 \
    -nographic
```
This creates:

- an Intel e1000 virtual NIC,
- one user-mode network backend (n1),
- a TCP port forwarding rule from host port 3636 to guest port 36, and no duplicate IDs.

### Overall flow of ping command
```
ping (user program)
        │
        ▼
System call
        │
        ▼
Kernel networking stack
        │
        ▼
Routing decision
        │
        ▼
Netfilter (LOCAL_OUT)
        │
        ▼
ARP (if needed)
        │
        ▼
Ethernet driver (e1000)
        │
        ▼
QEMU virtual NIC
        │
        ▼
QEMU user-mode NAT
        │
        ▼
Internet
        │
        ▼
8.8.8.8
        │
        ▼
ICMP Echo Reply
        │
        ▼
QEMU
        │
        ▼
e1000 driver
        │
        ▼
Netfilter (PRE_ROUTING)
        │
        ▼
Routing decision
        │
        ▼
Netfilter (LOCAL_IN)
        │
        ▼
ICMP layer
        │
        ▼
ping prints the reply
```
So in this case, it will go through 3 netfilter type:
- local_out
- pre_routing
- local_in

struct of nf_hook_ops:
```c
struct nf_hook_ops {
        struct list_head list;

        /* User fills in from here down. */
        nf_hookfn *hook;
        int pf;
        int hooknum;
        /* Hooks are ordered in ascending priority. */
        int priority;
};
```

### Use tuntap to connect with host and guest
```py
| Range                           | CIDR  | Typical use             |
| ------------------------------- | ----- | ----------------------- |
| `10.0.0.0 - 10.255.255.255`     | `/8`  | Large private networks  |
| `172.16.0.0 - 172.31.255.255`   | `/12` | Medium private networks |
| `192.168.0.0 - 192.168.255.255` | `/16` | Home/lab networks       |
Change /<num> as you like
# Example:
10.0.0.1/24
```
```For simple explanation:```

The /24 means: 
- The first 24 bits identify the network.
- The remaining 8 bits identify hosts.
        - 10: 1 byte
        - 0: 1 byte
        - 0: 1 byte
        - 1: the last byte

Command to add and use tuntap:
```c
sudo ip tuntap add dev tap0 mode tap
ip tuntap // Show all tun/tap devices

sudo ip link set tap0 up  // set status of device <tap0>
- result:
ip addr show dev tap0   // Show status of a specific device
10: tap0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc fq_codel state DOWN group default qlen 1000
    link/ether ae:9a:42:aa:1b:c6 brd ff:ff:ff:ff:ff:ff

sudo ip addr add 10.0.0.1/24 dev tap0 // Link a specific network to device <tap0>
- result:
ip addr show dev tap0   // Show status of a specific device            
10: tap0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc fq_codel state DOWN group default qlen 1000
    link/ether ae:9a:42:aa:1b:c6 brd ff:ff:ff:ff:ff:ff
    inet 10.0.0.1/24 scope global tap0
       valid_lft forever preferred_lft forever
```
Note that each machine must have each own IP address

Example:
- Host: link 10.0.0.1 to tap0
- Guest: link 10.0.0.2 to eth0

We can't use the same IP address for different machines

Because it use the same network 10.0.0 so I can connect it together

Note that I must link a 10.0.0.**num** first to be able to connect it 

**```The default gateway must be a machine that is directly connected to your network and knows how to forward packets.```**

Normal linux will drop packets not for it

Here is ways to make the linux forward the packet between network interfaces


Allow the kernel to forward the packet:
```c
sudo sysctl -w net.ipv4.ip_forward=1
```

Add first rule to be able to forward packet from guest to ping 8.8.8.8
```c
// first command
sudo iptables -A FORWARD -i tap0 -o wlp2s0 -j ACCEPT
```
```c
sudo iptables -L FORWARD -n -v      // check table                 
Chain FORWARD (policy DROP 788 packets, 66192 bytes)
 pkts bytes target     prot opt in     out     source               destination         
  792 66528 DOCKER-USER  all  --  *      *       0.0.0.0/0            0.0.0.0/0           
  792 66528 DOCKER-FORWARD  all  --  *      *       0.0.0.0/0            0.0.0.0/0           
    4   336 ACCEPT     all  --  tap0   wlp2s0  0.0.0.0/0            0.0.0.0/0           
```

Use the connection tracking module, and match only packets that belong to an established connection.

In other words, create second rule, find established connection then forward the reply from google to vm 
```c
// second command
sudo iptables -A FORWARD -i wlp2s0 -o tap0 -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
```
```c
sudo iptables -L FORWARD -n -v      // check table                
Chain FORWARD (policy DROP 802 packets, 67368 bytes)
 pkts bytes target     prot opt in     out     source               destination         
  842 70728 DOCKER-USER  all  --  *      *       0.0.0.0/0            0.0.0.0/0           
  842 70728 DOCKER-FORWARD  all  --  *      *       0.0.0.0/0            0.0.0.0/0           
   29  2436 ACCEPT     all  --  tap0   wlp2s0  0.0.0.0/0            0.0.0.0/0           
   11   924 ACCEPT     all  --  wlp2s0 tap0    0.0.0.0/0            0.0.0.0/0            ctstate RELATED,ESTABLISHED
```

This will easily work but will be much less secure since it accept every packet
```
sudo iptables -A FORWARD -j ACCEPT
```
This will also work too but it will be less secure only with this connection:
```c
sudo iptables -A FORWARD -i tap0 -o wlp2s0 -j ACCEPT
sudo iptables -A FORWARD -o tap0 -i wlp2s0 -j ACCEPT
```

Use this command to see address of live hook functions:
```c
cat /proc/modules
```
Add this argument to be able to debug kernel
```c
// The kernel will stop and wait for gdb attachment
// Default port for gdb attach is 1234
// So use target remote:1234 for gdb attachment
-s -S
```

```c
pwndbg> tel 0xc7811a20
// The hdp_hdr(skb) get start from these, wrong offset
00:0000│ ebx 0xc7811a20 ◂— 0x20000045 /* 'E' */
01:0004│     0xc7811a24 ◂— 0x40d67c
02:0008│     0xc7811a28 ◂— 0xf4a91140
03:000c│     0xc7811a2c ◂— 0x100000a /* '\n' */
04:0010│     0xc7811a30 ◂— 0x200000a /* '\n' */
// The true hdp_hdr in this kernel is this address
05:0014│     0xc7811a34 ◂— 0x240048e2
06:0018│     0xc7811a38 ◂— 0xf81a0c00
07:001c│     0xc7811a3c ◂— 'wdw\n'
pwndbg> 
08:0020│     0xc7811a40 ◂— 0
... ↓     7 skipped
pwndbg> 
```

The asm instructions to get true port:
```c
0xc88391fe <hook_route+214>    lea    edx, [ebx + ecx*4]         EDX => 0xc7811a34 ◂— 0x240048e2
// this get the port and get converted to 0x24 = 36 (target port) later by ntohs
0xc8839201 <hook_route+217>    mov    ax, word ptr [edx + 2]     AX, [0xc7811a36] => 0x2400
// src value
0xc8839205 <hook_route+221>    mov    dx, word ptr [edx]         DX, [0xc7811a34] => 0x48e2
```
# Kernel debug
- To debug a kernel, you must do the these steps respectively. Otherwise, you will load wrong offset of symbols and functions
:
```c
// GDB kernel 
gdb vmlinux
// connect to kernel in gdb
target remote:1234
// Check specific module live address
cat /proc/modules
// Load specific module symbols
add-symbol-file ./rootfs/home/hook.ko 0xc8839000
```
- Right one:
```c
pwndbg> info address hook_route
Symbol "hook_route" is a function at address 0xc8839128.
```

- Wrong one if not do like the above steps:
```c
pwndbg> info address hook_init
Symbol "hook_init" is a function at address 0x24.
```