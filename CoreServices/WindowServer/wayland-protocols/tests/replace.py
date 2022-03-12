#!/usr/bin/env python3

import sys

execpath, inpath, outpath, *dict_list = sys.argv

dictonary = {}
while dict_list:
    key, value, *rest = dict_list
    dictonary[key] = value
    dict_list = rest

infile = open(inpath, 'r')
outfile = open(outpath, 'w')

buf = infile.read()
infile.close()

for key, value in dictonary.items():
    buf = buf.replace('@{}@'.format(key), value)

outfile.write(buf)
outfile.close()
