#!/usr/bin/env python3
# Copyright © Microsoft Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import argparse

# Take a log file produced by GALLIUM_REFCNT_LOG, filter it to the objects that
# weren't destroyed by the end of the log, and write the results out sorted.
# Strips stacks by default to prevent OOM. Could probably be rewritten to walk
# the file twice to preserve stacks without OOM, but this was the easy way.
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input',
                        action='store',
                        required=True,
                        help='path to file containing refcount log')
    parser.add_argument('--output',
                        action='store',
                        required=True,
                        help='path to trimmed log')
    parser.add_argument('--filter',
                        help='object type filter')
    parser.add_argument('--keep-stacks',
                        help='keep stacks, otherwise only headers')
    args = parser.parse_args()

    objects = {}

    with open(args.input) as in_file:
        stack = []
        cur_object = ''

        for line in in_file:
            if line[0] == '<':

                parts = line.split(' ')
                prev_object = cur_object
                cur_object = parts[1]
                if parts[3].strip() == 'Destroy':
                    if cur_object in objects:
                        del objects[cur_object]
                else:
                    if parts[3].strip() == 'Create':
                        if (not args.filter) or (args.filter in parts[0]):
                            objects[cur_object] = []
                    if prev_object in objects:
                        objects[prev_object] += stack

                stack = [line]
            elif args.keep_stacks:
                stack += line

    with open(args.output, 'wt') as out_file:
        for stack in objects.values():
            for stack_line in stack:
                out_file.write(stack_line)


if __name__ == '__main__':
    main()
