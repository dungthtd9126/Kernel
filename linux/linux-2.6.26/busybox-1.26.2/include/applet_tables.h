/* This is a generated file, don't edit */

#define NUM_APPLETS 16
#define KNOWN_APPNAME_OFFSETS 0

const char applet_names[] ALIGN1 = ""
"ifdown" "\0"
"ifup" "\0"
"inetd" "\0"
"ip" "\0"
"ipaddr" "\0"
"iplink" "\0"
"iproute" "\0"
"iprule" "\0"
"iptunnel" "\0"
"nc" "\0"
"ping" "\0"
"ping6" "\0"
"sh" "\0"
"traceroute6" "\0"
"wget" "\0"
"whois" "\0"
;

#define APPLET_NO_ifdown 0
#define APPLET_NO_ifup 1
#define APPLET_NO_inetd 2
#define APPLET_NO_ip 3
#define APPLET_NO_ipaddr 4
#define APPLET_NO_iplink 5
#define APPLET_NO_iproute 6
#define APPLET_NO_iprule 7
#define APPLET_NO_iptunnel 8
#define APPLET_NO_nc 9
#define APPLET_NO_ping 10
#define APPLET_NO_ping6 11
#define APPLET_NO_sh 12
#define APPLET_NO_traceroute6 13
#define APPLET_NO_wget 14
#define APPLET_NO_whois 15

#ifndef SKIP_applet_main
int (*const applet_main[])(int argc, char **argv) = {
ifupdown_main,
ifupdown_main,
inetd_main,
ip_main,
ipaddr_main,
iplink_main,
iproute_main,
iprule_main,
iptunnel_main,
nc_main,
ping_main,
ping6_main,
ash_main,
traceroute6_main,
wget_main,
whois_main,
};
#endif

