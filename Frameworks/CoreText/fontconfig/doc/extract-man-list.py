#!/usr/bin/env python3
#
# fontconfig/doc/extract-man-list.py
#
# Parses .fncs files and extracts list of man pages that will be generated
#
# Copyright © 2020 Tim-Philipp Müller
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of the author(s) not be used in
# advertising or publicity pertaining to distribution of the software without
# specific, written prior permission.  The authors make no
# representations about the suitability of this software for any purpose.  It
# is provided "as is" without express or implied warranty.
#
# THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

import sys
import re

replacement_sets = []

# -------------------------------------
# Read replacement sets from .fncs file
# -------------------------------------

def read_fncs_file(fn):
  global replacement_sets

  with open(fn, 'r', encoding='utf8') as f:
    fncs_text = f.read()

    # split into replacement sets
    fncs_chunks = fncs_text.strip().split('@@')

    for chunk in fncs_chunks:
      # get rid of any preamble such as license and FcFreeTypeQueryAll decl in fcfreetype.fncs
      start = chunk.find('@')
      if start:
        chunk = chunk[start:]

      # split at '@' and remove empty lines (keep it simple instead of doing fancy
      # things with regular expression matches, we control the input after all)
      lines = [line for line in chunk.split('@') if line.strip()]

      replacement_set = {}

      while lines:
        tag = lines.pop(0).strip()
        # FIXME: this hard codes the tag used in funcs.sgml - we're lazy
        if tag.startswith('PROTOTYPE'):
          text = ''
        else:
          text = lines.pop(0).strip()
          if text.endswith('%'):
            text = text[:-1] + ' '

        replacement_set[tag] = text

      if replacement_set:
        replacement_sets += [replacement_set]

# ----------------------------------------------------------------------------
#  Main
# ----------------------------------------------------------------------------

if len(sys.argv) < 2:
  sys.exit('Usage: {} FILE1.FNCS [FILE2.FNCS...]'.format(sys.argv[0]))

fout = sys.stdout

for input_fn in sys.argv[1:]:
  read_fncs_file(input_fn)

# process template for each replacement set
for rep in replacement_sets:
  if 'FUNC+' in rep:
    man_page_title = rep.get('TITLE', rep['FUNC'])
  else:
    man_page_title = rep['FUNC']
  print(man_page_title)
