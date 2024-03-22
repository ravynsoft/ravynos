#!/usr/bin/python3

# Copyright (c) 2019 Collabora Ltd
# Copyright (c) 2020 Valve Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT

import argparse
import yaml

def cmd_fossils_db_repo(args):
    with open(args.file, 'r') as f:
        y = yaml.safe_load(f)
    print(y['fossils-db']['repo'])

def cmd_fossils_db_commit(args):
    with open(args.file, 'r') as f:
        y = yaml.safe_load(f)
    print(y['fossils-db']['commit'])

def cmd_fossils(args):
    with open(args.file, 'r') as f:
        y = yaml.safe_load(f)

    fossils = list(y['fossils'])
    if len(fossils) == 0:
        return

    print('\n'.join((t['path'] for t in fossils)))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', required=True,
                        help='the name of the yaml file')

    subparsers = parser.add_subparsers(help='sub-command help')

    parser_fossils_db_repo = subparsers.add_parser('fossils_db_repo')
    parser_fossils_db_repo.set_defaults(func=cmd_fossils_db_repo)

    parser_fossils_db_commit = subparsers.add_parser('fossils_db_commit')
    parser_fossils_db_commit.set_defaults(func=cmd_fossils_db_commit)

    parser_fossils = subparsers.add_parser('fossils')
    parser_fossils.set_defaults(func=cmd_fossils)

    args = parser.parse_args()
    args.func(args)

if __name__ == "__main__":
    main()
