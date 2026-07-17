#!/usr/bin/python3

from pwn import *
import socket

p = remote("10.0.0.2", 6767, typ="udp")

p.interactive()
