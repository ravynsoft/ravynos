#!/usr/bin/python3

import re
import sys

def main():
    if len(sys.argv) != 3:
        print("Missing arguments: ./radv_check_va.py <bo_history> <64-bit VA>")
        sys.exit(1)

    bo_history = str(sys.argv[1])
    va = int(sys.argv[2], 16)

    va_found = False
    with open(bo_history) as f:
        for line in f:
            p = re.compile('timestamp=(.*), VA=(.*)-(.*), destroyed=(.*), is_virtual=(.*)')
            m = p.match(line)
            if m == None:
                continue

            va_start = int(m.group(2), 16)
            va_end = int(m.group(3), 16)

            # Check if the given VA was ever valid and print info.
            if va >= va_start and va < va_end:
                print("VA found: %s" % line, end='')
                va_found = True
    if not va_found:
        print("VA not found!")

if __name__ == '__main__':
    main()
