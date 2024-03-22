#!/usr/bin/python3

import sys
import socket

host = sys.argv[1]
port = sys.argv[2]
mode = sys.argv[3]
relay = sys.argv[4]
msg = None

if mode == "on":
    msg = b'\x20'
else:
    msg = b'\x21'

msg += int(relay).to_bytes(1, 'big')
msg += b'\x00'

c = socket.create_connection((host, int(port)))
c.sendall(msg)

data = c.recv(1)
c.close()

if data[0] == b'\x01':
    print('Command failed')
    sys.exit(1)
