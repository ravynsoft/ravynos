#!/usr/bin/env python3
# Copyright © 2019, 2022 Intel Corporation
# SPDX-License-Identifier: MIT

from __future__ import annotations
import argparse
import copy
import intel_genxml
import pathlib
import typing


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('files', nargs='*',
                        default=pathlib.Path(__file__).parent.glob('*.xml'),
                        type=pathlib.Path)

    g = parser.add_mutually_exclusive_group(required=True)
    g.add_argument('--import', dest='_import', action='store_true',
                   help='Import and optimize genxml')
    g.add_argument('--flatten', action='store_true',
                   help='Remove imports from genxml')
    g.add_argument('--validate', action='store_true',
                   help='Validate genxml has no items duplicating imports')

    parser.add_argument('--quiet', action='store_true')
    args: Args = parser.parse_args()

    filenames = list(args.files)
    intel_genxml.sort_genxml_files(filenames)
    for filename in filenames:
        if not args.quiet:
            print('Processing {}... '.format(filename), end='', flush=True)

        genxml = intel_genxml.GenXml(filename)

        if args.validate:
            original = copy.deepcopy(genxml)
            genxml.optimize_xml_import()
            assert genxml.is_equivalent_xml(original), \
                f'{filename} is invalid, run genxml_import.py to fix it'
        elif args._import:
            genxml.add_xml_imports()
            genxml.optimize_xml_import()
            genxml.write_file()
        elif args.flatten:
            genxml.flatten_imported()
            genxml.write_file()

        if not args.quiet:
            print('done.')


if __name__ == '__main__':
    main()
