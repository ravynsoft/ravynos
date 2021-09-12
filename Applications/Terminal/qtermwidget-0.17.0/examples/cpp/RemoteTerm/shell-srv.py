#!/usr/bin/env python

import sys
import os
import socket
import pty

def usage(program):
    print "Example(server-side) for remote terminal of QTermWidget."
    print "Usage: %s ipaddr port" %program


def main():
    if len(sys.argv) != 3:
        usage(sys.argv[0])
        sys.exit(1)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.bind((sys.argv[1], int(sys.argv[2])))
        s.listen(0)
        print "[+]Start Server."
    except Exception as e:
        print "[-]Error Happened: %s" %e.message
        sys.exit(2)

    while True:
        c = s.accept()
        os.dup2(c[0].fileno(), 0)
        os.dup2(c[0].fileno(), 1)
        os.dup2(c[0].fileno(), 2)

        # It's important to use pty to spawn the shell.
        pty.spawn("/bin/sh")
        c[0].close()
    
if __name__ == "__main__":
    main()
