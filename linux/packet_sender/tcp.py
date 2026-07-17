#!/usr/bin/python3

from pwn import *
import socket

p = remote("10.0.0.45", 68)

p.interactive()
