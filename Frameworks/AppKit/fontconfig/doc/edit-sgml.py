#!/usr/bin/env python3
#
# fontconfig/doc/edit-sgml.py
#
# Copyright © 2003 Keith Packard
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

import argparse
import sys
import re

parser = argparse.ArgumentParser()
parser.add_argument('template')
parser.add_argument('input')
parser.add_argument('output')

args = parser.parse_known_args()

template_fn = args[0].template
output_fn = args[0].output
input_fn = args[0].input

# -------------
# Read template
# -------------

with open(template_fn, 'r', encoding='utf8') as f:
  template_text = f.read()

template_lines = template_text.strip().split('\n')

# -------------------------------------
# Read replacement sets from .fncs file
# -------------------------------------

replacement_sets = []

# TODO: also allow '-' for stdin
with open(input_fn, 'r', encoding='utf8') as f:
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

# ----------------
# Open output file
# ----------------

if output_fn == '-':
  fout = sys.stdout
else:
  fout = open(output_fn, "w", encoding='utf8')

# ----------------
# Process template
# ----------------

def do_replace(template_lines, rep, tag_suffix=''):
  skip_tag = None
  skip_lines = False
  loop_lines = []
  loop_tag = None

  for t_line in template_lines:
    # This makes processing easier and is the case for our templates
    if t_line.startswith('@') and not t_line.endswith('@'):
      sys.exit('Template lines starting with @ are expected to end with @, please fix me!')

    if loop_tag:
      loop_lines += [t_line]

    # Check if line starts with a directive
    if t_line.startswith('@?'):
      tag = t_line[2:-1] + tag_suffix
      if skip_tag:
        sys.exit('Recursive skipping not supported, please fix me!')
      skip_tag = tag
      skip_lines = tag not in rep
    elif t_line.startswith('@:'):
      if not skip_tag:
        sys.exit('Skip else but no active skip list?!')
      skip_lines = skip_tag in rep
    elif t_line.startswith('@;'):
      if not skip_tag:
        sys.exit('Skip end but no active skip list?!')
      skip_tag = None
      skip_lines = False
    elif t_line.startswith('@{'):
      if loop_tag or tag_suffix != '':
        sys.exit('Recursive looping not supported, please fix me!')
      loop_tag = t_line[2:-1]
    elif t_line.startswith('@}'):
      tag = t_line[2:-1] + tag_suffix
      if not loop_tag:
        sys.exit('Loop end but no active loop?!')
      if loop_tag != tag:
        sys.exit(f'Loop end but loop tag mismatch: {loop_tag} != {tag}!')
      loop_lines.pop() # remove loop end directive
      suffix = '+'
      while loop_tag + suffix in rep:
        do_replace(loop_lines, rep, suffix)
        suffix += '+'
      loop_tag = None
      loop_lines = []
    else:
      if not skip_lines:
        # special-case inline optional substitution (hard-codes specific pattern in funcs.sgml because we're lazy)
        output_line = re.sub(r'@\?(RET)@@RET@@:@(void)@;@', lambda m: rep.get(m.group(1) + tag_suffix, m.group(2)), t_line)
        # replace any substitution tags with their respective substitution text
        output_line = re.sub(r'@(\w+)@', lambda m: rep.get(m.group(1) + tag_suffix, ''), output_line)
        print(output_line, file=fout)

# process template for each replacement set
for rep in replacement_sets:
  do_replace(template_lines, rep)
