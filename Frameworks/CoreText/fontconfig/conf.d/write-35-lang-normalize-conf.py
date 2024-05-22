#!/usr/bin/env python3
#
# fontconfig write-35-lang-normalize-conf.py

import os
import sys

if len(sys.argv) < 2:
  print('ERROR: usage: {} ORTH_LIST [OUTPUT.CONF]'.format(sys.argv[0]))
  sys.exit(-1)

orth_list_unsorted = sys.argv[1].split(',')

if len(sys.argv) > 2 and sys.argv[2] != '-':
  f_out = open(sys.argv[2], 'w', encoding='utf8')
else:
  f_out = sys.stdout

orth_list = sorted(sys.argv[1].split(','))

print('<?xml version="1.0"?>', file=f_out)
print('<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">', file=f_out)
print('<fontconfig>', file=f_out)

for o in orth_list:
  print(f'  <!-- {o}* -> {o} -->', file=f_out)
  print(f'  <match>', file=f_out)
  print(f'    <test name="lang" compare="contains"><string>{o}</string></test>', file=f_out)
  print(f'    <edit name="lang" mode="assign" binding="same"><string>{o}</string></edit>', file=f_out)
  print(f'  </match>', file=f_out)

print('</fontconfig>', file=f_out)

f_out.close()

sys.exit(0)
